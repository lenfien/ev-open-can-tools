#pragma once
#include <Arduino.h>

static const char DASH_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tesla CAN 助手</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#111;color:#eee;font:14px/1.5 -apple-system,BlinkMacSystemFont,sans-serif;max-width:480px;margin:0 auto;padding:16px 12px 48px}
h2{font-size:11px;text-transform:uppercase;letter-spacing:.08em;color:#555;margin:22px 0 6px}
.card{background:#1a1a1a;border:1px solid #252525;border-radius:10px;overflow:hidden;margin-bottom:6px}
.row{display:flex;justify-content:space-between;align-items:center;padding:9px 14px;border-bottom:1px solid #222}
.row:last-child{border-bottom:none}
.lbl{color:#888;font-size:13px}
.val{font-weight:600;font-size:13px}
.ok{color:#3dba72}.err{color:#ff4f4f}.warn{color:#f5a623}
.tog{display:flex;justify-content:space-between;align-items:center;padding:10px 14px;border-bottom:1px solid #222}
.tog:last-child{border-bottom:none}
.tlbl{font-size:13px;color:#ccc;flex:1;padding-right:12px}
.sw{position:relative;width:40px;height:22px;flex-shrink:0}
.sw input{opacity:0;width:0;height:0;position:absolute}
.sl{position:absolute;inset:0;background:#2e2e2e;border-radius:11px;cursor:pointer;transition:background .15s}
.sl::before{content:'';position:absolute;width:16px;height:16px;left:3px;top:3px;background:#888;border-radius:50%;transition:transform .15s,background .15s}
input:checked+.sl{background:#3a5fa8}
input:checked+.sl::before{transform:translateX(18px);background:#5b8fff}
.inp{background:#1e1e1e;border:1px solid #2e2e2e;border-radius:7px;color:#eee;padding:8px 10px;font-size:13px;width:100%;margin-top:6px;outline:none}
.inp:focus{border-color:#444}
.btn{display:block;width:100%;background:#2a3f6e;color:#8ab4f8;border:1px solid #3a5fa8;border-radius:8px;padding:10px;cursor:pointer;font-size:13px;margin-top:8px;font-weight:600;transition:background .15s}
.btn:hover{background:#334d85}
.btn.danger{background:#3b1616;color:#ff7070;border-color:#7a2424}
.btn.danger:hover{background:#4a1e1e}
.srow{display:flex;gap:8px;align-items:flex-end;margin-top:6px}
.srow input{flex:1;margin-top:0}
.srow button{flex-shrink:0;width:auto;margin-top:0;padding:8px 14px}
.net{padding:8px 10px;background:#1e1e1e;border-radius:6px;margin-top:4px;cursor:pointer;display:flex;justify-content:space-between;align-items:center;font-size:13px;border:1px solid #2a2a2a}
.net:hover{background:#242424}
.rssi{color:#555;font-size:12px}
.pgrp{display:flex;gap:4px;margin-top:8px}
.pbtn{flex:1;background:#1e1e1e;border:1px solid #2e2e2e;border-radius:7px;color:#666;padding:8px 2px;cursor:pointer;font-size:12px;font-weight:600;transition:all .15s}
.pbtn:hover{border-color:#555;color:#ccc}
.pbtn.act{background:#2a3f6e;border-color:#3a5fa8;color:#8ab4f8}
.hdr{display:flex;align-items:center;justify-content:space-between;margin-bottom:18px}
.htitle{font-size:18px;font-weight:700;letter-spacing:-.02em}
#dot{width:8px;height:8px;border-radius:50%;background:#333;flex-shrink:0}
#rfbtn{background:none;border:1px solid #2e2e2e;border-radius:6px;color:#555;cursor:pointer;padding:3px 8px;font-size:16px;line-height:1;transition:color .15s,border-color .15s;margin-left:8px}
#rfbtn:hover{color:#aaa;border-color:#555}
@keyframes spin{to{transform:rotate(360deg)}}
.spinning{animation:spin .4s linear}
.pad{padding:12px 14px}
</style>
</head>
<body>
<div class="hdr">
  <span class="htitle">Tesla CAN 助手</span>
  <div style="display:flex;align-items:center">
    <span id="dot"></span>
    <button id="rfbtn" onclick="manualRefresh()" title="刷新">&#8635;</button>
  </div>
</div>

<h2>状态</h2>
<div class="card">
  <div class="row"><span class="lbl">CAN 总线</span><span id="s_can" class="val">--</span></div>
  <div class="row"><span class="lbl">运行时间</span><span id="s_up" class="val">--</span></div>
  <div class="row"><span class="lbl">帧 收 / 发</span><span id="s_frm" class="val">--</span></div>
  <div class="row"><span class="lbl">跟车距离</span><span id="s_fd" class="val">--</span></div>
  <div class="row"><span class="lbl">速度档位 HW3 / HW4</span><span id="s_sp" class="val">--</span></div>
  <div class="row"><span class="lbl">限速 融合 / 视觉</span><span id="s_sl" class="val">--</span></div>
  <div class="row"><span class="lbl">速度偏移</span><span id="s_so" class="val">--</span></div>
  <div class="row"><span class="lbl">网关自动驾驶状态</span><span id="s_gw" class="val">--</span></div>
  <div class="row"><span class="lbl">Ban 盾 命中 / 检查</span><span id="s_bs" class="val">--</span></div>
</div>

<h2>功能</h2>
<div class="card" id="feat"></div>

<h2>速度档位</h2>
<div class="card" id="spd_tog"></div>
<div class="card pad" id="profile_web_card" style="display:none">
  <span class="lbl">档位（网页来源）</span>
  <div class="pgrp">
    <button class="pbtn" data-pv="1" onclick="setProfile(1)">最慢</button>
    <button class="pbtn" data-pv="2" onclick="setProfile(2)">舒适</button>
    <button class="pbtn" data-pv="3" onclick="setProfile(3)">标准</button>
    <button class="pbtn" data-pv="4" onclick="setProfile(4)">快速</button>
    <button class="pbtn" data-pv="5" onclick="setProfile(5)">最快</button>
  </div>
</div>

<h2>速度偏移</h2>
<div class="card" id="off_tog"></div>
<div class="card pad" id="offset_fix_card" style="display:none">
  <span class="lbl">固定偏移值（0 – 50）</span>
  <div class="srow">
    <input type="number" id="speed_offset_fix_from_web" class="inp" min="0" max="50" placeholder="0">
    <button class="btn" onclick="saveNum('speed_offset_fix_from_web')">保存</button>
  </div>
</div>

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
<button class="btn danger" onclick="reboot()">重启设备</button>

<script>
const GW=['无','高速','增强','自动驾驶','基础'];

const FEATS=[
  ['enable_inject','注入激活'],
  ['enable_fsd','FSD 启用'],
  ['use_hw3_code','使用 HW3 代码'],
  ['enable_ban_shield','Ban 盾保护'],
  ['enable_nag_suppress','消除提示音'],
  ['enable_summon_unlock','Summon 解锁'],
  ['disable_camera','禁用摄像头'],
  ['enable_emergency_vehicle_detection_runtime','紧急车辆检测'],
  ['enable_isa_speed_chime_suppress_runtime','ISA 提示音抑制'],
  ['enable_enhanced_autopilot_runtime','增强自动驾驶'],
  ['enable_print','串口输出'],
];
const SPD_TOGS=[
  ['speed_profile_use_follow_distance','使用跟车距离拨杆控制档位'],
  ['enable_set_hw3_profile','将速度档位写入 HW3 帧'],
];
const OFF_TOGS=[
  ['speed_offset_enable_override','启用速度偏移覆盖'],
  ['speed_offset_use_fix_or_dynamic','使用自动表（关 = 固定值）'],
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
buildToggles('feat', FEATS);
buildToggles('spd_tog', SPD_TOGS);
buildToggles('off_tog', OFF_TOGS);

function refreshCards() {
  const useStalk   = g('speed_profile_use_follow_distance')?.checked;
  const overrideOn = g('speed_offset_enable_override')?.checked;
  const useDynamic = g('speed_offset_use_fix_or_dynamic')?.checked;
  g('profile_web_card').style.display = useStalk ? 'none' : '';
  g('offset_fix_card').style.display  = (overrideOn && !useDynamic) ? '' : 'none';
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
  txt('s_can', s.can_online ? '在线' : '离线', s.can_online ? 'ok' : 'err');
  txt('s_up',  fmtUp(s.uptime));
  txt('s_frm', s.frame_cnt+' / '+s.frame_sent);
  txt('s_fd',  s.follow_distance);
  txt('s_sp',  s.profile_hw3+' / '+s.profile_hw4);
  txt('s_sl',  s.speed_limit_fused+' / '+s.speed_limit_vision_only+' km/h');
  txt('s_so',  s.speed_offset);
  txt('s_gw',  GW[s.gateway_autopilot] || s.gateway_autopilot);
  txt('s_bs',  s.ban_shield_cnt+' / '+s.ban_shield_check_cnt);
}

function updateCnf(c) {
  [...FEATS, ...SPD_TOGS, ...OFF_TOGS].forEach(([key]) => {
    const e = g(key);
    if (e && e.type === 'checkbox') e.checked = !!c[key];
  });
  document.querySelectorAll('.pbtn').forEach(b => b.classList.toggle('act', +b.dataset.pv === c.speed_profile_from_web));
  const so = g('speed_offset_fix_from_web');
  if (so && document.activeElement !== so) so.value = c.speed_offset_fix_from_web;
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
  const b = g('rfbtn');
  b.classList.add('spinning');
  b.addEventListener('animationend', () => b.classList.remove('spinning'), {once:true});
  poll(); loadAp(); loadSta();
}

poll(); loadAp(); loadSta();
setInterval(poll, 2000);
setInterval(loadAp, 15000);
setInterval(loadSta, 6000);
</script>
</body>
</html>)HTML";
