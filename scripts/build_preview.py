#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
build_preview.py  ─  把固件的 Web UI 和 Schema 原地“打包”成一份
可直接用浏览器打开的静态预览页：scratch/preview.html

思路
----
真页面由 ``src/web_ui.cpp`` 中的 ``DASH_HTML`` raw-string 提供；ESP32 在线时浏览器
通过 /schema /status /config 等 HTTP 接口拿数据。离线预览想还原这一整套效果，
需要三件事：

1. 完整的 HTML / JS（从 web_ui.cpp 抽出 R"HTML(...)HTML"）。
2. 一个假后端，把前端发出的 fetch 拦截并返回合理 JSON。
3. 保证假后端的 /schema 和 /status 字段与 C++ 里 kCnfSchema / kStateSchema
   完全一致 —— 为此我们直接用正则从 handlers.cpp 解析这两个表，而不是再手抄一份。

脚本没有第三方依赖，纯标准库，放到 CI 里也能跑。

用法
----
    python3 scripts/build_preview.py
    open scratch/preview.html

如果你给 handlers 增加了新 schema 字段，重跑一次就能在 preview 里看到。
"""

from __future__ import annotations

import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT        = Path(__file__).resolve().parent.parent
WEB_UI_CPP  = ROOT / "src" / "web_ui.cpp"
HANDLERS_CPP= ROOT / "src" / "handlers.cpp"
OUT_HTML    = ROOT / "scratch" / "preview.html"

# ──────────────────────────────────────────────────────────────────────
# 1. 从 web_ui.cpp 抽 DASH_HTML
# ──────────────────────────────────────────────────────────────────────
def extract_dash_html(src: str) -> str:
    m = re.search(r'R"HTML\((.*?)\)HTML";', src, flags=re.DOTALL)
    if not m:
        sys.exit("[build_preview] 未在 web_ui.cpp 里找到 R\"HTML(...)HTML\" raw string")
    return m.group(1)


# ──────────────────────────────────────────────────────────────────────
# 2. 从 handlers.cpp 抽 schema
# ──────────────────────────────────────────────────────────────────────
@dataclass
class CnfField:
    key: str
    label: str
    group: str | None
    type: str      # "bool" | "number" | "enum"
    widget: str    # "checkbox" | "input" | "select" | "slider"
    min: int = 0
    max: int = 1
    step: int = 1
    hidden: bool = False


@dataclass
class StateField:
    key: str
    label: str
    type: str       # "number" | "enum"
    unit: str | None = None
    tile: str | None = None
    tile_label: str | None = None
    tile_sep: str | None = None
    enum_labels: str | None = None


def _unquote_c_string(raw: str) -> str | None:
    """把形如 ``"abc"`` 或 ``nullptr`` 的 C 字面量变成 Python str / None。"""
    raw = raw.strip()
    if raw in ("nullptr", "NULL", "0"):
        return None
    m = re.match(r'^"((?:\\.|[^"\\])*)"$', raw)
    if not m:
        return None
    # handlers.cpp 里只会出现几种常见转义
    return (m.group(1)
            .replace(r'\n', '\n')
            .replace(r'\t', '\t')
            .replace(r'\\', '\\')
            .replace(r'\"', '"'))


def _split_top_level_commas(args: str) -> list[str]:
    """按顶层逗号切分，忽略字符串和括号内的逗号。"""
    parts, buf = [], []
    depth = 0
    in_str = False
    esc = False
    for ch in args:
        if in_str:
            buf.append(ch)
            if esc:         esc = False
            elif ch == '\\': esc = True
            elif ch == '"': in_str = False
            continue
        if ch == '"':
            in_str = True
            buf.append(ch)
            continue
        if ch in "([{": depth += 1
        elif ch in ")]}": depth -= 1
        if ch == ',' and depth == 0:
            parts.append(''.join(buf))
            buf = []
            continue
        buf.append(ch)
    if buf:
        parts.append(''.join(buf))
    return [p.strip() for p in parts]


def _iter_macro_calls(src: str, macro: str):
    """yield 每个 `MACRO( ... )` 的顶层参数字符串。"""
    for _, args in _iter_macro_calls_with_pos(src, macro):
        yield args


def _iter_macro_calls_with_pos(src: str, macro: str):
    """yield (start_pos, args_str) —— start_pos 是宏名在 src 里的起始下标。"""
    for m in re.finditer(rf'\b{macro}\s*\(', src):
        # 手动找匹配的右括号（考虑字符串）
        i = m.end()
        depth = 1
        in_str = False
        esc = False
        start = i
        while i < len(src) and depth > 0:
            ch = src[i]
            if in_str:
                if esc: esc = False
                elif ch == '\\': esc = True
                elif ch == '"': in_str = False
            else:
                if ch == '"': in_str = True
                elif ch == '(': depth += 1
                elif ch == ')': depth -= 1
            i += 1
        if depth == 0:
            yield m.start(), src[start:i-1]


def parse_schema(src: str) -> tuple[list[CnfField], list[StateField]]:
    cnf: list[CnfField] = []
    state: list[StateField] = []

    # 只在 kCnfSchema[] / kStateSchema[] 对应的区块里扫描，避免误吞其他地方
    def slice_block(name: str) -> str:
        m = re.search(rf'{name}\s*\[\]\s*=\s*\{{(.*?)\}};', src, flags=re.DOTALL)
        return m.group(1) if m else ""

    state_block = slice_block("kStateSchema")
    cnf_block   = slice_block("kCnfSchema")

    # ── state ──
    # 按宏在源码里的出现位置（pos）统一排序，保证 STATE_NUM 和 STATE_ENUM
    # 交错时也能与 kStateSchema 源码顺序一致（否则预览里 tile 顺序会和实物不一致）。
    state_items: list[tuple[int, StateField]] = []
    for pos, args in _iter_macro_calls_with_pos(state_block, "STATE_NUM"):
        a = _split_top_level_commas(args)
        # KEY, LABEL, UNIT, TILE, TILE_LBL, SEP
        state_items.append((pos, StateField(
            key=a[0].strip(),
            label=_unquote_c_string(a[1]) or a[0].strip(),
            type="number",
            unit=_unquote_c_string(a[2]),
            tile=_unquote_c_string(a[3]),
            tile_label=_unquote_c_string(a[4]),
            tile_sep=_unquote_c_string(a[5]),
        )))
    for pos, args in _iter_macro_calls_with_pos(state_block, "STATE_ENUM"):
        a = _split_top_level_commas(args)
        # KEY, LABEL, LABELS, TILE, TILE_LBL, SEP
        state_items.append((pos, StateField(
            key=a[0].strip(),
            label=_unquote_c_string(a[1]) or a[0].strip(),
            type="enum",
            tile=_unquote_c_string(a[3]),
            tile_label=_unquote_c_string(a[4]),
            tile_sep=_unquote_c_string(a[5]),
            enum_labels=_unquote_c_string(a[2]),
        )))
    state_items.sort(key=lambda x: x[0])
    state = [sf for _, sf in state_items]

    # ── cnf ──
    for args in _iter_macro_calls(cnf_block, "CNF_BOOL"):
        a = _split_top_level_commas(args)
        # KEY, LABEL, GROUP
        cnf.append(CnfField(
            key=a[0].strip(),
            label=_unquote_c_string(a[1]) or a[0].strip(),
            group=_unquote_c_string(a[2]),
            type="bool",
            widget="checkbox",
        ))
    for args in _iter_macro_calls(cnf_block, "CNF_HIDDEN_BOOL"):
        a = _split_top_level_commas(args)
        key = a[0].strip()
        cnf.append(CnfField(
            key=key, label=key, group=None,
            type="bool", widget="checkbox", hidden=True,
        ))
    for args in _iter_macro_calls(cnf_block, "CNF_HIDDEN_NUM"):
        a = _split_top_level_commas(args)
        # KEY, MINV, MAXV, STEPV
        cnf.append(CnfField(
            key=a[0].strip(), label=a[0].strip(), group=None,
            type="number", widget="input", hidden=True,
            min=int(a[1]), max=int(a[2]), step=int(a[3]),
        ))

    return cnf, state


# ──────────────────────────────────────────────────────────────────────
# 3. 生成 mock prelude（替代原来手写的 preview.html 头部）
# ──────────────────────────────────────────────────────────────────────
def build_prelude(cnf: list[CnfField], state: list[StateField]) -> str:
    # 序列化成 JS 字面量，内联到页面里
    cnf_js = json.dumps(
        [
            {k: v for k, v in {
                "key": f.key, "label": f.label, "group": f.group,
                "type": f.type, "widget": f.widget,
                "hidden": f.hidden or None,
                "min": f.min if f.type == "number" else None,
                "max": f.max if f.type == "number" else None,
                "step": f.step if f.type == "number" else None,
            }.items() if v is not None}
            for f in cnf
        ],
        ensure_ascii=False, indent=2,
    )
    state_js = json.dumps(
        [
            {k: v for k, v in {
                "key": f.key, "label": f.label, "type": f.type,
                "unit": f.unit, "tile": f.tile,
                "tile_label": f.tile_label, "tile_sep": f.tile_sep,
                "enum_labels": f.enum_labels,
            }.items() if v is not None}
            for f in state
        ],
        ensure_ascii=False, indent=2,
    )

    # 给每个 state 字段生成一个合理的初始值：枚举挑第 1 项，数值给个小 demo 数字
    demo_state = {}
    for i, f in enumerate(state):
        if f.type == "enum" and f.enum_labels:
            demo_state[f.key] = min(1, len(f.enum_labels.split("|")) - 1)
        else:
            demo_state[f.key] = (i + 1) * 3  # 让每个 tile 数字都不一样
    demo_state_js = json.dumps(demo_state, ensure_ascii=False, indent=2)

    return PRELUDE_TEMPLATE.format(
        cnf_js=cnf_js, state_js=state_js, demo_state_js=demo_state_js,
    )


# 注意：下面这段里的大括号是 JS 的，要在 .format() 里用 {{ / }} 转义。
PRELUDE_TEMPLATE = r"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tesla CAN 助手 (Mock 预览 · 自动生成)</title>
<!-- Favicon：Tesla logo，base64 内联 SVG（和固件 DASH_HTML 保持一致） -->
<link rel="icon" type="image/svg+xml" href="data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9Ii0zOC4wMzc2IC02My4xMjU1IDMyOS42NTkyIDM3OC43NTMiIGZpbGw9ImN1cnJlbnRDb2xvciI+PHBhdGggZD0iTTEyNi44MDYgMjUyLjUwMmwzNS40NzYtMTk5LjUxOWMzMy44MTUgMCA0NC40ODEgMy43MDggNDYuMDIxIDE4Ljg0MyAwIDAgMjIuNjg0LTguNDU4IDM0LjEyNS0yNS42MzYtNDQuNjQ2LTIwLjY4OC04OS41MDUtMjEuNjIxLTg5LjUwNS0yMS42MjFsLTI2LjE3NiAzMS44ODIuMDU5LS4wMDQtMjYuMTc2LTMxLjg4M3MtNDQuODYuOTM0LTg5LjUgMjEuNjIyYzExLjQzMSAxNy4xNzggMzQuMTI0IDI1LjYzNiAzNC4xMjQgMjUuNjM2IDEuNTQ5LTE1LjEzNiAxMi4yMDItMTguODQ0IDQ1Ljc5LTE4Ljg2OGwzNS43NjIgMTk5LjU0OCIvPjxwYXRoIGQ9Ik0xMjYuNzkyIDE1LjM2YzM2LjA5LS4yNzYgNzcuMzk5IDUuNTgzIDExOS42ODcgMjQuMDE0IDUuNjUyLTEwLjE3MyA3LjEwNS0xNC42NjkgNy4xMDUtMTQuNjY5QzIwNy4zNTcgNi40MTYgMTY0LjA2Ni4xNTcgMTI2Ljc4NyAwIDg5LjUxLjE1NyA0Ni4yMjEgNi40MTcgMCAyNC43MDVjMCAwIDIuMDYyIDUuNTM4IDcuMSAxNC42NjkgNDIuMjgtMTguNDMxIDgzLjU5Ni0yNC4yOSAxMTkuNjg3LTI0LjAxNGguMDA1Ii8+PC9zdmc+">
<!-- ============================================================
  本页面由 scripts/build_preview.py 自动生成，不要手工编辑。
  Schema 从 handlers.cpp 解析而来；UI 从 web_ui.cpp 抽出。
  重新生成：python3 scripts/build_preview.py
  ============================================================ -->
<script>
(function () {{
  'use strict';

  // ---- 从 handlers.cpp 解析出的真实 schema ----
  const SCHEMA_CNF   = {cnf_js};
  const SCHEMA_STATE = {state_js};

  // ---- 假 state：对 SCHEMA_STATE 每个 key 给个初始值 ----
  const mockState = Object.assign({{
    frame_cnt: 0, frame_sent: 0, uptime: 0, _startTime: Date.now(),
  }}, {demo_state_js});

  // ---- 假 cnf：对 SCHEMA_CNF 每个 key 给默认值 ----
  const mockCnf = {{}};
  SCHEMA_CNF.forEach(f => {{
    if (f.type === 'bool')        mockCnf[f.key] = false;
    else if (f.type === 'number') mockCnf[f.key] = (f.min|0);
  }});
  // auto_cfg 是特别结构，前端直接读 cnf.auto_cfg
  mockCnf.auto_cfg = [
    {{spd:30,pct:5}}, {{spd:40,pct:8}}, {{spd:50,pct:10}}, {{spd:60,pct:12}},
    {{spd:70,pct:15}},{{spd:80,pct:18}},{{spd:90,pct:20}}, {{spd:100,pct:22}},
    {{spd:110,pct:25}},{{spd:120,pct:28}},{{spd:0,pct:0}},{{spd:0,pct:0}},
  ];
  mockCnf.speed_profile_from_web = 3;
  mockCnf.speed_offset_fix_from_web = 10;
  mockCnf.enable_fsd = true;
  mockCnf.enable_ban_shield = true;
  mockCnf.enable_nag_suppress = true;

  // ---- 调试四组帧（和固件 /debug_status 结构对齐） ----
  function newFrame(data) {{
    return {{ seen: true, data: data.slice(0,8),
             mask:[0,0,0,0,0,0,0,0], val:[0,0,0,0,0,0,0,0], last_ms: 0 }};
  }}
  const mockDbg = {{
    f1016:    newFrame([0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x80]),
    f1021_m0: newFrame([0x00,0x00,0x00,0x00,0x00,0xC0,0x04,0x80]),
    f1021_m1: newFrame([0x01,0x00,0x08,0x00,0x00,0x00,0x00,0x80]),
    f1021_m2: newFrame([0xE2,0x05,0x00,0x00,0x00,0x00,0x00,0xB0]),
  }};
  function jitterRaw() {{
    const t = Math.floor(Date.now()/3000) & 1;
    mockDbg.f1016.data[0] = (mockDbg.f1016.data[0] & ~0x02) | (t?0x02:0);
  }}

  // ---- 小工具 ----
  function jsonResp(o) {{
    return new Response(JSON.stringify(o),
      {{status:200, headers:{{'Content-Type':'application/json'}}}});
  }}
  function okResp() {{ return jsonResp({{ok:true}}); }}
  function parseForm(body) {{
    const out = {{}};
    if (!body) return out;
    const text = (typeof body === 'string') ? body
                : (body instanceof URLSearchParams ? body.toString() : String(body));
    text.split('&').forEach(kv => {{
      if (!kv) return;
      const i = kv.indexOf('=');
      const k = decodeURIComponent((i<0?kv:kv.slice(0,i)).replace(/\+/g,' '));
      const v = decodeURIComponent((i<0?'' :kv.slice(i+1)).replace(/\+/g,' '));
      out[k] = v;
    }});
    return out;
  }}
  function pickDbgFrame(name) {{
    return mockDbg[name==='1016'?'f1016'
                 :name==='1021_m0'?'f1021_m0'
                 :name==='1021_m1'?'f1021_m1'
                 :name==='1021_m2'?'f1021_m2':null] || null;
  }}

  const MOCK_NVS_KEY = 'mock_esp32_nvs_dbg_ovr_v1';
  const origFetch = window.fetch.bind(window);

  window.fetch = function mockFetch(input, init) {{
    const url    = (typeof input === 'string') ? input : (input && input.url) || '';
    const method = ((init && init.method) || 'GET').toUpperCase();
    const path   = url.split('?')[0];
    if (/^https?:/.test(url) && !url.startsWith(location.origin)) return origFetch(input, init);

    // /schema —— 结构：{{cnf:[...], state:[...]}}，字段名与固件 handleSchema 保持一致
    if (path === '/schema' && method === 'GET') {{
      return jsonResp({{ cnf: SCHEMA_CNF, state: SCHEMA_STATE }});
    }}

    // /status —— 每次推进帧计数和 uptime，并返回全部 state + cnf
    if (path === '/status' && method === 'GET') {{
      mockState.uptime = Math.floor((Date.now() - mockState._startTime) / 1000);
      mockState.frame_cnt  += 120;
      mockState.frame_sent += mockCnf.enable_inject ? 30 : 0;
      // frame_rx_rate / tx_rate 给个缓慢变化的数
      mockState.frame_rx_rate = 40 + Math.floor(Math.sin(Date.now()/2000)*5+5);
      mockState.frame_tx_rate = mockCnf.enable_inject ? 12 : 0;
      return jsonResp({{ state: {{...mockState}}, cnf: {{...mockCnf}} }});
    }}

    // /config —— 把表单参数 merge 回 mockCnf（schema 驱动）
    if (path === '/config' && method === 'POST') {{
      const args = parseForm(init && init.body);
      Object.keys(args).forEach(k => {{
        if (k.startsWith('auto_cfg_')) {{
          const i = +k.slice('auto_cfg_'.length);
          if (mockCnf.auto_cfg[i])
            mockCnf.auto_cfg[i].pct = Math.max(0, Math.min(100, +args[k]|0));
          return;
        }}
        const f = SCHEMA_CNF.find(x => x.key === k);
        if (!f) return;
        if (f.type === 'bool')         mockCnf[k] = (args[k] === '1' || args[k] === 'true');
        else if (f.type === 'number')  mockCnf[k] = +args[k] | 0;
      }});
      return okResp();
    }}

    if (path === '/ap_status'   && method === 'GET') return jsonResp({{ssid:'Tesla-CAN-Mock',ip:'192.168.4.1',clients:1,hidden:false}});
    if (path === '/wifi_status' && method === 'GET') return jsonResp({{connected:false,ssid:'',static:false}});
    if (path === '/wifi_scan'   && method === 'GET') return jsonResp({{networks:[
      {{ssid:'HomeWifi',rssi:-42,enc:true}},
      {{ssid:'Tesla_Garage',rssi:-58,enc:true}},
      {{ssid:'CMCC-Guest',rssi:-71,enc:false}},
    ]}});
    if (path === '/wifi_config' && method === 'POST') return okResp();
    if (path === '/ap_config'   && method === 'POST') return okResp();
    if (path === '/reboot'      && method === 'POST') return okResp();

    if (path === '/debug_status' && method === 'GET') {{
      jitterRaw();
      const now = Date.now();
      const snap = {{}};
      ['f1016','f1021_m0','f1021_m1','f1021_m2'].forEach(k => {{
        const fr = mockDbg[k];
        fr.last_ms = now;
        const data = fr.data.slice();
        for (let i=0;i<8;i++) data[i] = (data[i] & ~fr.mask[i]) | (fr.val[i] & fr.mask[i]);
        snap[k] = {{ seen:fr.seen, data, mask:fr.mask.slice(), val:fr.val.slice() }};
      }});
      return jsonResp(snap);
    }}
    if (path === '/debug_set' && method === 'POST') {{
      const a = parseForm(init && init.body);
      const fr = pickDbgFrame(a.frame);
      if (!fr) return jsonResp({{ok:false,error:'bad frame'}});
      const start=+a.start|0, length=+a.length|0, value=(+a.value)>>>0, on=a.override==='1';
      for (let i=0;i<length;i++) {{
        const b=start+i, bi=b>>3, bo=b&7, m=1<<bo;
        if (on) {{ fr.mask[bi]|=m; if ((value>>>i)&1) fr.val[bi]|=m; else fr.val[bi]&=~m&0xFF; }}
        else    {{ fr.mask[bi]&=~m&0xFF; fr.val[bi]&=~m&0xFF; }}
      }}
      return okResp();
    }}
    if (path === '/debug_clear' && method === 'POST') {{
      const a = parseForm(init && init.body);
      const clear = fr => {{ fr.mask.fill(0); fr.val.fill(0); }};
      if (a.frame === 'all') {{
        ['f1016','f1021_m0','f1021_m1','f1021_m2'].forEach(k => clear(mockDbg[k]));
        try {{ localStorage.removeItem(MOCK_NVS_KEY); }} catch(e) {{}}
      }} else {{
        const fr = pickDbgFrame(a.frame); if (fr) clear(fr);
      }}
      return okResp();
    }}
    if (path === '/debug_archive_save' && method === 'POST') {{
      try {{
        const snap = {{}};
        ['f1016','f1021_m0','f1021_m1','f1021_m2'].forEach(k => {{
          snap[k] = {{ mask: mockDbg[k].mask.slice(), val: mockDbg[k].val.slice() }};
        }});
        localStorage.setItem(MOCK_NVS_KEY, JSON.stringify(snap));
        return okResp();
      }} catch(e) {{ return jsonResp({{ok:false,error:String(e&&e.message||e)}}); }}
    }}

    return new Response('mock: not handled '+path, {{status:404}});
  }};

  // 启动恢复
  try {{
    const saved = localStorage.getItem(MOCK_NVS_KEY);
    if (saved) {{
      const snap = JSON.parse(saved);
      ['f1016','f1021_m0','f1021_m1','f1021_m2'].forEach(k => {{
        if (snap[k] && snap[k].mask && snap[k].val) {{
          mockDbg[k].mask = snap[k].mask.slice();
          mockDbg[k].val  = snap[k].val.slice();
        }}
      }});
      console.log('%c[MOCK] 已从 "NVS" 恢复调试覆盖存档','color:#3dba72');
    }}
  }} catch(e) {{}}

  console.log('%c[MOCK] preview.html · 所有后端请求被前端 mock 拦截',
              'color:#5b8fff;font-weight:bold');
}})();
</script>
"""


# ──────────────────────────────────────────────────────────────────────
# 4. 主入口
# ──────────────────────────────────────────────────────────────────────
def main() -> int:
    if not WEB_UI_CPP.exists():   sys.exit(f"[build_preview] 找不到 {WEB_UI_CPP}")
    if not HANDLERS_CPP.exists(): sys.exit(f"[build_preview] 找不到 {HANDLERS_CPP}")

    dash_html = extract_dash_html(WEB_UI_CPP.read_text(encoding="utf-8"))
    cnf, state = parse_schema(HANDLERS_CPP.read_text(encoding="utf-8"))
    if not cnf or not state:
        sys.exit("[build_preview] schema 解析失败；请检查 handlers.cpp 的宏格式")

    prelude = build_prelude(cnf, state)
    # DASH_HTML 开头是 `<!DOCTYPE html>\n<html>\n<head>...<title>...</title>\n<style>`
    # 我们要用自己的 <head>（prelude 里已经给了），跳过固件版本的到第一个 <style> 之前。
    style_idx = dash_html.find("<style>")
    if style_idx < 0:
        sys.exit("[build_preview] 在 DASH_HTML 里没找到 <style>，UI 结构变了？")
    body_html = dash_html[style_idx:]  # 从 <style> 开始拼接

    OUT_HTML.parent.mkdir(parents=True, exist_ok=True)
    OUT_HTML.write_text(prelude + "\n" + body_html, encoding="utf-8")
    print(f"[build_preview] 已生成 {OUT_HTML.relative_to(ROOT)} "
          f"({len(cnf)} 个 cnf 字段 / {len(state)} 个 state 字段)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
