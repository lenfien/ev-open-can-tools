#pragma once
#include <Arduino.h>

static const char DASH_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tesla CAN 助手</title>
<style>
:root{
  --bg:#111;--text:#eee;--h2:#555;
  --card:#1a1a1a;--border:#252525;--sep:#222;
  --lbl:#888;--tlbl:#ccc;--fname:#ccc;--tlb:#555;--tval:#eee;
  --sw-off:#2e2e2e;--sw-knob:#888;
  --inp:#1e1e1e;--inp-b:#2e2e2e;--inp-f:#444;
  --net:#1e1e1e;--net-b:#2a2a2a;--net-h:#242424;--rssi:#555;
  --pbtn:#1e1e1e;--pbtn-b:#2e2e2e;--pbtn-c:#666;--pbtn-hb:#555;--pbtn-hc:#ccc;
  --dot-def:#333;--hbtn-b:#2e2e2e;--hbtn-c:#555;--hbtn-hc:#aaa;--hbtn-hb:#555;
  --tabbar:#161616;--tab-c:#555;
}
body.light{
  --bg:#f2f2f7;--text:#111;--h2:#8e8e93;
  --card:#fff;--border:#e5e5ea;--sep:#f0f0f0;
  --lbl:#6e6e73;--tlbl:#333;--fname:#333;--tlb:#8e8e93;--tval:#111;
  --sw-off:#e5e5ea;--sw-knob:#bbb;
  --inp:#f2f2f7;--inp-b:#d1d1d6;--inp-f:#aaa;
  --net:#f2f2f7;--net-b:#e5e5ea;--net-h:#e8e8ed;--rssi:#8e8e93;
  --pbtn:#f2f2f7;--pbtn-b:#d1d1d6;--pbtn-c:#6e6e73;--pbtn-hb:#aaa;--pbtn-hc:#333;
  --dot-def:#c7c7cc;--hbtn-b:#d1d1d6;--hbtn-c:#8e8e93;--hbtn-hc:#333;--hbtn-hb:#aaa;
  --tabbar:#f9f9f9;--tab-c:#8e8e93;
}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font:14px/1.5 -apple-system,BlinkMacSystemFont,sans-serif;max-width:480px;margin:0 auto;padding:16px 12px 68px;transition:background .2s,color .2s}
h2{font-size:11px;text-transform:uppercase;letter-spacing:.08em;color:var(--h2);margin:22px 0 6px}
.card{background:var(--card);border:1px solid var(--border);border-radius:10px;overflow:hidden;margin-bottom:6px}
.row{display:flex;justify-content:space-between;align-items:center;padding:9px 14px;border-bottom:1px solid var(--sep)}
.row:last-child{border-bottom:none}
.lbl{color:var(--lbl);font-size:13px}
.val{font-weight:600;font-size:13px}
.ok{color:#3dba72}.err{color:#ff4f4f}.warn{color:#f5a623}
.tog{display:flex;justify-content:space-between;align-items:center;padding:10px 14px;border-bottom:1px solid var(--sep)}
.tog:last-child{border-bottom:none}
.tlbl{font-size:13px;color:var(--tlbl);flex:1;padding-right:12px}
.sw{position:relative;width:40px;height:22px;flex-shrink:0}
.sw input{opacity:0;width:0;height:0;position:absolute}
.sl{position:absolute;inset:0;background:var(--sw-off);border-radius:11px;cursor:pointer;transition:background .15s}
.sl::before{content:'';position:absolute;width:16px;height:16px;left:3px;top:3px;background:var(--sw-knob);border-radius:50%;transition:transform .15s,background .15s}
input:checked+.sl{background:#3a5fa8}
input:checked+.sl::before{transform:translateX(18px);background:#5b8fff}
.inp{background:var(--inp);border:1px solid var(--inp-b);border-radius:7px;color:var(--text);padding:8px 10px;font-size:13px;width:100%;margin-top:6px;outline:none}
.inp:focus{border-color:var(--inp-f)}
.btn{display:block;width:100%;background:#2a3f6e;color:#8ab4f8;border:1px solid #3a5fa8;border-radius:8px;padding:10px;cursor:pointer;font-size:13px;margin-top:8px;font-weight:600;transition:background .15s}
.btn:hover{background:#334d85}
.btn.danger{background:#3b1616;color:#ff7070;border-color:#7a2424}
.btn.danger:hover{background:#4a1e1e}
.srow{display:flex;gap:8px;align-items:flex-end;margin-top:6px}
.srow input{flex:1;margin-top:0}
.srow button{flex-shrink:0;width:auto;margin-top:0;padding:8px 14px}
.net{padding:8px 10px;background:var(--net);border-radius:6px;margin-top:4px;cursor:pointer;display:flex;justify-content:space-between;align-items:center;font-size:13px;border:1px solid var(--net-b)}
.net:hover{background:var(--net-h)}
.rssi{color:var(--rssi);font-size:12px}
.pgrp{display:flex;gap:4px;margin-top:8px}
.pbtn{flex:1;background:var(--pbtn);border:1px solid var(--pbtn-b);border-radius:7px;color:var(--pbtn-c);padding:8px 2px;cursor:pointer;font-size:12px;font-weight:600;transition:all .15s}
.pbtn:hover{border-color:var(--pbtn-hb);color:var(--pbtn-hc)}
.pbtn.act{background:#2a3f6e;border-color:#3a5fa8;color:#8ab4f8}
.hdr{display:flex;align-items:center;justify-content:space-between;margin-bottom:18px}
.htitle{font-size:18px;font-weight:700;letter-spacing:-.02em}
#dot{width:8px;height:8px;border-radius:50%;background:var(--dot-def);flex-shrink:0}
#rfbtn{background:none;border:1px solid var(--hbtn-b);border-radius:6px;color:var(--hbtn-c);cursor:pointer;padding:3px 8px;font-size:16px;line-height:1;transition:color .15s,border-color .15s;margin-left:8px}
#rfbtn:hover{color:var(--hbtn-hc);border-color:var(--hbtn-hb)}
.thsw{position:relative;display:flex;background:var(--sw-off);border-radius:20px;padding:2px;cursor:pointer;margin-left:8px;flex-shrink:0}
.thopt{width:26px;height:24px;display:flex;align-items:center;justify-content:center;font-size:13px;position:relative;z-index:1;user-select:none}
.thknob{position:absolute;top:2px;left:2px;width:26px;height:24px;background:#3a5fa8;border-radius:16px;transition:transform .2s cubic-bezier(.4,0,.2,1)}
body.light .thknob{transform:translateX(26px)}
@keyframes spin{to{transform:rotate(360deg)}}
.spinning{animation:spin .4s linear}
.pad{padding:12px 14px}
.flist{display:grid;grid-template-columns:1fr 1fr;gap:6px;margin-bottom:6px}
.fcard.wide{grid-column:span 2}
.fcard{background:var(--card);border:1px solid var(--border);border-radius:10px;padding:11px 14px;display:flex;align-items:center;gap:12px}
.feat-children{display:grid;grid-template-columns:1fr 1fr;gap:6px;transition:opacity .25s;grid-column:span 2}
.feat-children.locked,#controlled.locked{opacity:.35;pointer-events:none}
#controlled{transition:opacity .25s}
.master-card{background:var(--card);border:1px solid var(--border);border-radius:12px;padding:16px;display:flex;align-items:center;justify-content:space-between;margin-bottom:16px;transition:border-color .2s,background .2s}
.master-card.on{border-color:#3a5fa8;background:linear-gradient(135deg,var(--card) 60%,#0d1f3c)}
.master-lbl{font-size:15px;font-weight:700;color:var(--text)}
.master-sub{font-size:11px;color:var(--h2);margin-top:3px}
.ficon{width:8px;height:8px;border-radius:50%;flex-shrink:0}
.fname{font-size:12px;color:var(--fname);flex:1;line-height:1.3}
.grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:6px;margin-bottom:6px}
.tile{background:var(--card);border:1px solid var(--border);border-radius:10px;padding:10px 12px;display:flex;flex-direction:column;gap:4px;min-width:0}
.tlb{font-size:11px;color:var(--tlb);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.tval{font-size:15px;font-weight:700;color:var(--tval);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.pg{display:none}
.pg.act{display:block}
.tabbar{position:fixed;bottom:0;left:0;right:0;max-width:480px;margin:0 auto;display:flex;background:var(--tabbar);border-top:1px solid var(--border);z-index:100}
.tab{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;padding:8px 0 12px;background:none;border:none;color:var(--tab-c);cursor:pointer;font-size:10px;gap:3px;transition:color .15s;letter-spacing:.02em}
.tab svg{width:22px;height:22px;stroke:currentColor;fill:none;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}
.tab.act{color:#5b8fff}
.segsw{position:relative;display:grid;grid-template-columns:1fr 1fr;background:var(--sw-off);border-radius:20px;padding:2px;cursor:pointer;flex-shrink:0}
.segopt{display:flex;align-items:center;justify-content:center;padding:5px 10px;font-size:11px;font-weight:600;position:relative;z-index:1;user-select:none;color:var(--tlbl);white-space:nowrap;transition:color .2s}
.segsw.right .segopt:last-of-type,.segsw:not(.right) .segopt:first-of-type{color:#fff}
.segknob{position:absolute;top:2px;left:2px;bottom:2px;width:calc(50% - 2px);background:#3a5fa8;border-radius:16px;transition:transform .2s cubic-bezier(.4,0,.2,1)}
.segsw.right .segknob{transform:translateX(calc(100% + 4px))}
.slider{width:100%;margin-top:4px;-webkit-appearance:none;appearance:none;height:4px;border-radius:2px;background:var(--inp-b);outline:none;cursor:pointer}
.slider::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border-radius:50%;background:#5b8fff;cursor:pointer;box-shadow:0 1px 4px rgba(0,0,0,.4)}
.slider::-moz-range-thumb{width:22px;height:22px;border-radius:50%;background:#5b8fff;cursor:pointer;border:none}
.adv-hdr{display:flex;justify-content:space-between;align-items:center;padding:10px 14px;cursor:pointer;user-select:none}
.adv-lbl{font-size:12px;color:var(--lbl)}
.adv-chevron{font-size:11px;color:var(--h2);transition:transform .2s}
.adv-chevron.open{transform:rotate(180deg)}
.adv-body{padding:4px 14px 14px}
.cfg-row{display:flex;align-items:center;gap:8px;margin-top:10px}
.cfg-spd{font-size:12px;color:var(--lbl);width:48px;flex-shrink:0}
.cfg-pct{font-size:12px;font-weight:600;color:#5b8fff;width:36px;text-align:right;flex-shrink:0}
</style>
</head>
<body>
<div class="hdr">
  <span class="htitle">Tesla CAN 助手</span>
  <div style="display:flex;align-items:center">
    <span id="dot"></span>
    <div class="thsw" onclick="toggleTheme()" title="切换主题">
      <span class="thopt">&#9790;</span>
      <span class="thopt">&#9728;</span>
      <span class="thknob"></span>
    </div>
    <button id="rfbtn" onclick="manualRefresh()" title="刷新">&#8635;</button>
  </div>
</div>

<div id="pg_main" class="pg act">
<h2>状态</h2>
<div class="grid">
  <div class="tile"><span class="tlb">CAN 总线</span><span id="s_can" class="tval">--</span></div>
  <div class="tile"><span class="tlb">运行时间</span><span id="s_up" class="tval">--</span></div>
  <div class="tile"><span class="tlb">帧 收 / 发</span><span id="s_frm" class="tval">--</span></div>
  <div class="tile"><span class="tlb">跟车距离</span><span id="s_fd" class="tval">--</span></div>
  <div class="tile"><span class="tlb">速度档位 HW3/HW4</span><span id="s_sp" class="tval">--</span></div>
  <div class="tile"><span class="tlb">限速 融合/视觉</span><span id="s_sl" class="tval">--</span></div>
  <div class="tile"><span class="tlb">速度偏移</span><span id="s_so" class="tval">--</span></div>
  <div class="tile"><span class="tlb">网关自动驾驶</span><span id="s_gw" class="tval">--</span></div>
  <div class="tile"><span class="tlb">Ban 盾 命中/检查</span><span id="s_bs" class="tval">--</span></div>
</div>

<div class="master-card" id="master-card">
  <div>
    <div class="master-lbl">注入激活</div>
    <div class="master-sub">总开关 · 控制以下所有功能</div>
  </div>
  <label class="sw"><input type="checkbox" id="enable_inject" onchange="setConf('enable_inject',this.checked);syncMaster(this.checked)"><span class="sl"></span></label>
</div>

<div id="controlled" class="locked">
<h2>功能</h2>
<div id="feat"></div>

<h2>速度档位</h2>
<div class="card">
  <div class="tog">
    <span class="tlbl">档位来源</span>
    <div class="segsw" id="seg_spd_src" onclick="toggleSpdSrc()">
      <span class="segopt">跟车距离</span>
      <span class="segopt">固定</span>
      <span class="segknob"></span>
    </div>
  </div>
  <div id="profile_web_card" style="display:none;border-top:1px solid var(--sep)" class="pad">
    <div class="pgrp">
      <button class="pbtn" data-pv="1" onclick="setProfile(1)">最慢</button>
      <button class="pbtn" data-pv="2" onclick="setProfile(2)">舒适</button>
      <button class="pbtn" data-pv="3" onclick="setProfile(3)">标准</button>
      <button class="pbtn" data-pv="4" onclick="setProfile(4)">快速</button>
      <button class="pbtn" data-pv="5" onclick="setProfile(5)">最快</button>
    </div>
  </div>
  <div class="tog">
    <span class="tlbl">将档位写入 HW3 帧</span>
    <label class="sw"><input type="checkbox" id="enable_set_hw3_profile" onchange="setConf('enable_set_hw3_profile',this.checked)"><span class="sl"></span></label>
  </div>
</div>

<h2>速度偏移</h2>
<div class="card">
  <div class="tog">
    <span class="tlbl">启用速度偏移覆盖</span>
    <label class="sw"><input type="checkbox" id="speed_offset_enable_override" onchange="_overrideOn=this.checked;setConf('speed_offset_enable_override',this.checked)"><span class="sl"></span></label>
  </div>
  <div class="tog" id="off_mode_row" style="display:none">
    <span class="tlbl">偏移模式</span>
    <div class="segsw" id="seg_off_mode" onclick="toggleOffMode()">
      <span class="segopt">固定值</span>
      <span class="segopt">自动</span>
      <span class="segknob"></span>
    </div>
  </div>
  <div id="offset_fix_card" style="display:none;border-top:1px solid var(--sep)" class="pad">
    <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:10px">
      <span class="lbl">速度偏移</span>
      <span id="offset_display" class="val" style="color:#5b8fff">0%</span>
    </div>
    <input type="range" id="speed_offset_fix_from_web" class="slider" min="0" max="50" value="0" oninput="onOffsetSlide(this.value)">
  </div>
  <div id="auto_cfg_card" style="display:none;border-top:1px solid var(--sep)">
    <div class="adv-hdr" onclick="toggleAutoCfg()">
      <span class="adv-lbl">高级 · 自动偏移表</span>
      <span id="adv-chevron" class="adv-chevron">&#9660;</span>
    </div>
    <div id="adv-body" style="display:none" class="adv-body"></div>
  </div>
</div>

</div><!-- controlled -->
</div><!-- pg_main -->

<div id="pg_wifi" class="pg">
<h2>WiFi — 热点 (AP)</h2>
<div class="card">
  <div class="row"><span class="lbl">SSID</span><span id="ap_ssid" class="val">--</span></div>
  <div class="row"><span class="lbl">IP</span><span id="ap_ip" class="val">--</span></div>
  <div class="row"><span class="lbl">已连接设备</span><span id="ap_cli" class="val">--</span></div>
</div>
<div class="card pad">
  <span class="lbl">修改热点名称 / 密码</span>
  <input type="text"     id="ap_ssid_in" class="inp" placeholder="新 SSID">
  <input type="password" id="ap_pass_in" class="inp" placeholder="新密码（至少 8 位）">
  <button class="btn" onclick="saveAp()">保存热点配置</button>
</div>

<h2>WiFi — 客户端 (STA)</h2>
<div class="card">
  <div class="row"><span class="lbl">状态</span><span id="sta_st" class="val">--</span></div>
  <div class="row"><span class="lbl">SSID</span><span id="sta_ssid" class="val">--</span></div>
  <div class="row"><span class="lbl">IP</span><span id="sta_ip" class="val">--</span></div>
</div>
<div class="card pad">
  <button class="btn" onclick="scanWifi()">扫描网络</button>
  <div id="nets"></div>
  <input type="text"     id="sta_ssid_in" class="inp" placeholder="SSID">
  <input type="password" id="sta_pass_in" class="inp" placeholder="密码">
  <button class="btn" onclick="connectWifi()">连接</button>
</div>

<h2>系统</h2>
<div class="card">
  <div class="tog">
    <span class="tlbl">串口输出</span>
    <label class="sw"><input type="checkbox" id="enable_print" onchange="setConf('enable_print',this.checked)"><span class="sl"></span></label>
  </div>
</div>
<button class="btn danger" onclick="reboot()">重启设备</button>
</div><!-- pg_wifi -->

<nav class="tabbar">
  <button class="tab act" id="tab_main" onclick="switchTab('main')">
    <svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/></svg>
    主页
  </button>
  <button class="tab" id="tab_wifi" onclick="switchTab('wifi')">
    <svg viewBox="0 0 24 24"><path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/><path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1" fill="currentColor"/></svg>
    WiFi / 系统
  </button>
</nav>

<script>
const GW=['无','高速','增强','自动驾驶','基础'];

const FEATS=[
  ['enable_fsd','FSD 启用'],
  ['use_hw3_code','使用 HW3 代码'],
  ['enable_ban_shield','Ban 盾保护'],
  ['enable_nag_suppress','消除提示音'],
  ['enable_summon_unlock','Summon 解锁'],
  ['disable_camera','禁用摄像头'],
  ['enable_emergency_vehicle_detection_runtime','紧急车辆检测'],
  ['enable_isa_speed_chime_suppress_runtime','ISA 提示音抑制'],
  ['enable_enhanced_autopilot_runtime','增强自动驾驶'],
];

function buildToggles(containerId, list) {
  const el = document.getElementById(containerId);
  list.forEach(([key, lbl]) => {
    el.insertAdjacentHTML('beforeend',
      '<div class="tog"><span class="tlbl">'+lbl+'</span>' +
      '<label class="sw"><input type="checkbox" id="'+key+'" onchange="setConf(\''+key+'\',this.checked)">' +
      '<span class="sl"></span></label></div>');
  });
}
const FEAT_COLORS=['#5b8fff','#3dba72','#f5a623','#ff6b6b','#a78bfa','#34d399','#fb923c','#60a5fa','#f472b6','#4ade80','#facc15'];
function buildFeatureCards(containerId, list) {
  const el = document.getElementById(containerId);
  el.className = 'flist';
  list.forEach(([key, lbl], i) => {
    const col = FEAT_COLORS[i % FEAT_COLORS.length];
    el.insertAdjacentHTML('beforeend',
      '<div class="fcard"><span class="ficon" style="background:'+col+'"></span>' +
      '<span class="fname">'+lbl+'</span>' +
      '<label class="sw"><input type="checkbox" id="'+key+'" onchange="setConf(\''+key+'\',this.checked)">' +
      '<span class="sl"></span></label></div>');
  });
}
function syncMaster(on) {
  document.getElementById('controlled').classList.toggle('locked', !on);
  const card = document.getElementById('master-card');
  if (card) card.classList.toggle('on', on);
}
buildFeatureCards('feat', FEATS);
syncMaster(false);

function switchTab(name) {
  document.querySelectorAll('.pg').forEach(p => p.classList.remove('act'));
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('act'));
  document.getElementById('pg_'+name).classList.add('act');
  document.getElementById('tab_'+name).classList.add('act');
}

let _useStalk = true, _overrideOn = false, _useDynamic = false;

function refreshCards() {
  g('profile_web_card').style.display = _useStalk ? 'none' : '';
  g('seg_spd_src').classList.toggle('right', !_useStalk);
  g('off_mode_row').style.display = _overrideOn ? '' : 'none';
  g('offset_fix_card').style.display = (_overrideOn && !_useDynamic) ? '' : 'none';
  g('auto_cfg_card').style.display  = (_overrideOn && _useDynamic)  ? '' : 'none';
  g('seg_off_mode').classList.toggle('right', _useDynamic);
}

function toggleSpdSrc() {
  _useStalk = !_useStalk;
  setConf('speed_profile_use_follow_distance', _useStalk);
  refreshCards();
}
function toggleOffMode() {
  _useDynamic = !_useDynamic;
  setConf('speed_offset_use_fix_or_dynamic', _useDynamic);
  refreshCards();
}

let _autoCfgOpen = false;
function toggleAutoCfg() {
  _autoCfgOpen = !_autoCfgOpen;
  g('adv-body').style.display = _autoCfgOpen ? '' : 'none';
  g('adv-chevron').classList.toggle('open', _autoCfgOpen);
}

const _cfgDebounces = {};
function onCfgSlide(i, v) {
  g('cfg-pct-'+i).textContent = v + '%';
  clearTimeout(_cfgDebounces[i]);
  _cfgDebounces[i] = setTimeout(() => setConf('auto_cfg_'+i, v), 1000);
}

function buildAutoCfgSliders(cfgArr) {
  const body = g('adv-body');
  cfgArr.forEach((entry, i) => {
    if (entry.spd === 0) return;
    let row = g('cfg-row-'+i);
    if (!row) {
      body.insertAdjacentHTML('beforeend',
        '<div class="cfg-row" id="cfg-row-'+i+'">' +
        '<span class="cfg-spd">'+entry.spd+' km/h</span>' +
        '<input type="range" class="slider" style="flex:1" min="0" max="100" value="'+entry.pct+'" oninput="onCfgSlide('+i+',this.value)">' +
        '<span class="cfg-pct" id="cfg-pct-'+i+'">'+entry.pct+'%</span>' +
        '</div>');
    } else if (document.activeElement !== row.querySelector('input')) {
      row.querySelector('input').value = entry.pct;
      g('cfg-pct-'+i).textContent = entry.pct + '%';
    }
  });
}

let _offsetDebounce;
function onOffsetSlide(v) {
  g('offset_display').textContent = v + '%';
  clearTimeout(_offsetDebounce);
  _offsetDebounce = setTimeout(() => setConf('speed_offset_fix_from_web', v), 300);
}
async function setConf(key, val) {
  const p = new URLSearchParams();
  p.set(key, (typeof val === 'boolean') ? (val ? '1' : '0') : String(val));
  try { await fetch('/config', {method:'POST', body:p}); } catch(e) {}
  refreshCards();
}
async function setProfile(v) {
  await setConf('speed_profile_from_web', v);
  document.querySelectorAll('.pbtn').forEach(b => b.classList.toggle('act', +b.dataset.pv === v));
}
async function saveNum(key) {
  const v = document.getElementById(key).value;
  if (v === '') return;
  await setConf(key, v);
}

function g(id) { return document.getElementById(id); }
function txt(id, t, cls) {
  const e = g(id);
  e.textContent = t;
  e.className = 'val' + (cls ? ' '+cls : '');
}
function fmtUp(s) {
  const h=Math.floor(s/3600), m=Math.floor((s%3600)/60), ss=s%60;
  return h+':'+String(m).padStart(2,'0')+':'+String(ss).padStart(2,'0');
}

function updateState(s) {
  ttxt('s_can', s.can_online ? '在线' : '离线', s.can_online ? 'ok' : 'err');
  ttxt('s_up',  fmtUp(s.uptime));
  ttxt('s_frm', s.frame_cnt+' / '+s.frame_sent);
  ttxt('s_fd',  s.follow_distance);
  ttxt('s_sp',  s.profile_hw3+' / '+s.profile_hw4);
  ttxt('s_sl',  s.speed_limit_fused+' / '+s.speed_limit_vision_only+' km/h');
  ttxt('s_so',  s.speed_offset);
  ttxt('s_gw',  GW[s.gateway_autopilot] || s.gateway_autopilot);
  ttxt('s_bs',  s.ban_shield_cnt+' / '+s.ban_shield_check_cnt);
}
function ttxt(id, t, cls) {
  const e = g(id);
  e.textContent = t;
  e.className = 'tval' + (cls ? ' '+cls : '');
}

function updateCnf(c) {
  const inj = g('enable_inject'); if (inj) inj.checked = !!c.enable_inject;
  FEATS.forEach(([key]) => {
    const e = g(key); if (e && e.type === 'checkbox') e.checked = !!c[key];
  });
  const hw3 = g('enable_set_hw3_profile'); if (hw3) hw3.checked = !!c.enable_set_hw3_profile;
  const prnt = g('enable_print'); if (prnt) prnt.checked = !!c.enable_print;
  const ovr = g('speed_offset_enable_override'); if (ovr) ovr.checked = !!c.speed_offset_enable_override;
  _useStalk   = !!c.speed_profile_use_follow_distance;
  _overrideOn = !!c.speed_offset_enable_override;
  _useDynamic = !!c.speed_offset_use_fix_or_dynamic;
  syncMaster(!!c.enable_inject);
  document.querySelectorAll('.pbtn').forEach(b => b.classList.toggle('act', +b.dataset.pv === c.speed_profile_from_web));
  if (c.auto_cfg) buildAutoCfgSliders(c.auto_cfg);
  const so = g('speed_offset_fix_from_web');
  if (so && document.activeElement !== so) {
    so.value = c.speed_offset_fix_from_web;
    g('offset_display').textContent = c.speed_offset_fix_from_web + '%';
  }
  refreshCards();
}

async function poll() {
  try {
    const r = await fetch('/status');
    if (!r.ok) throw 0;
    const d = await r.json();
    updateState(d.state);
    updateCnf(d.cnf);
    g('dot').style.background = '#3dba72';
  } catch(e) {
    g('dot').style.background = '#ff4f4f';
  }
}

async function loadAp() {
  try {
    const d = await (await fetch('/ap_status')).json();
    txt('ap_ssid', d.ssid); txt('ap_ip', d.ip); txt('ap_cli', d.clients);
  } catch(e) {}
}
async function loadSta() {
  try {
    const d = await (await fetch('/wifi_status')).json();
    txt('sta_st',   d.connected ? '已连接' : '未连接', d.connected ? 'ok' : 'warn');
    txt('sta_ssid', d.ssid || '--');
    txt('sta_ip',   d.connected ? (d.ip||'--') : '--');
  } catch(e) {}
}

async function scanWifi() {
  g('nets').innerHTML = '<div style="color:#555;padding:6px 0">扫描中...</div>';
  try {
    const d = await (await fetch('/wifi_scan')).json();
    g('nets').innerHTML = d.networks.map(n => {
      const esc = n.ssid.replace(/&/g,'&amp;').replace(/"/g,'&quot;').replace(/'/g,'&#39;');
      return '<div class="net" onclick="g(\'sta_ssid_in\').value=\''+esc+'\'">'+
             '<span>'+esc+(n.enc?' &#128274;':'')+'</span>'+
             '<span class="rssi">'+n.rssi+' dBm</span></div>';
    }).join('');
  } catch(e) {
    g('nets').innerHTML = '<div style="color:#ff4f4f;padding:6px 0">扫描失败</div>';
  }
}

async function connectWifi() {
  const p = new URLSearchParams({ssid: g('sta_ssid_in').value, pass: g('sta_pass_in').value});
  try { await fetch('/wifi_config', {method:'POST', body:p}); } catch(e) {}
  setTimeout(loadSta, 6000);
}

async function saveAp() {
  const ssid = g('ap_ssid_in').value, pass = g('ap_pass_in').value;
  if (!ssid) return alert('SSID 不能为空');
  if (pass && pass.length < 8) return alert('密码至少需要 8 位');
  const p = new URLSearchParams({ssid, pass});
  try {
    await fetch('/ap_config', {method:'POST', body:p});
    alert('已保存，重启后新热点配置生效。');
  } catch(e) {}
}

async function reboot() {
  if (!confirm('确认重启设备？')) return;
  try { await fetch('/reboot', {method:'POST'}); } catch(e) {}
}

function manualRefresh() {
  location.reload();
}

function toggleTheme() {
  const light = document.body.classList.toggle('light');
  localStorage.setItem('theme', light ? 'light' : 'dark');
}
if (localStorage.getItem('theme') === 'light') document.body.classList.add('light');

poll(); loadAp(); loadSta();
setInterval(poll, 2000);
setInterval(loadAp, 15000);
setInterval(loadSta, 6000);
</script>
</body>
</html>)HTML";
