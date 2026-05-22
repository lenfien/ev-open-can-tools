//
// Created by bamboozhang on 2026/4/28.
//

#include "web_logic.h"

const char DASH_HTML[] PROGMEM =

R"HTML(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tesla CAN 助手</title>
<!-- Favicon：Tesla logo，base64 内联，无需额外 HTTP 路由；fill=currentColor 支持主题自适应 -->
<link rel="icon" type="image/svg+xml" href="data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9Ii0zOC4wMzc2IC02My4xMjU1IDMyOS42NTkyIDM3OC43NTMiIGZpbGw9ImN1cnJlbnRDb2xvciI+PHBhdGggZD0iTTEyNi44MDYgMjUyLjUwMmwzNS40NzYtMTk5LjUxOWMzMy44MTUgMCA0NC40ODEgMy43MDggNDYuMDIxIDE4Ljg0MyAwIDAgMjIuNjg0LTguNDU4IDM0LjEyNS0yNS42MzYtNDQuNjQ2LTIwLjY4OC04OS41MDUtMjEuNjIxLTg5LjUwNS0yMS42MjFsLTI2LjE3NiAzMS44ODIuMDU5LS4wMDQtMjYuMTc2LTMxLjg4M3MtNDQuODYuOTM0LTg5LjUgMjEuNjIyYzExLjQzMSAxNy4xNzggMzQuMTI0IDI1LjYzNiAzNC4xMjQgMjUuNjM2IDEuNTQ5LTE1LjEzNiAxMi4yMDItMTguODQ0IDQ1Ljc5LTE4Ljg2OGwzNS43NjIgMTk5LjU0OCIvPjxwYXRoIGQ9Ik0xMjYuNzkyIDE1LjM2YzM2LjA5LS4yNzYgNzcuMzk5IDUuNTgzIDExOS42ODcgMjQuMDE0IDUuNjUyLTEwLjE3MyA3LjEwNS0xNC42NjkgNy4xMDUtMTQuNjY5QzIwNy4zNTcgNi40MTYgMTY0LjA2Ni4xNTcgMTI2Ljc4NyAwIDg5LjUxLjE1NyA0Ni4yMjEgNi40MTcgMCAyNC43MDVjMCAwIDIuMDYyIDUuNTM4IDcuMSAxNC42NjkgNDIuMjgtMTguNDMxIDgzLjU5Ni0yNC4yOSAxMTkuNjg3LTI0LjAxNGguMDA1Ii8+PC9zdmc+">
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
body{background:var(--bg);color:var(--text);font:14px/1.5 -apple-system,BlinkMacSystemFont,sans-serif;max-width:480px;margin:0 auto;padding:16px 12px 80px;transition:background .2s,color .2s}
h2{font-size:11px;text-transform:uppercase;letter-spacing:.08em;color:var(--h2);margin:22px 0 6px;transition:color .45s ease}
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
.btn.secondary{background:var(--pbtn);color:var(--pbtn-hc);border-color:var(--pbtn-b)}
.btn.secondary:hover{border-color:var(--pbtn-hb);background:var(--net-h)}
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
.flist{display:grid;grid-template-columns:repeat(3,1fr);grid-auto-flow:dense;gap:8px;margin-bottom:6px}
.fcard.wide{grid-column:span 2}
.fcard.w3{grid-column:span 3}
.fcard{background:var(--card);border:1px solid var(--border);border-radius:10px;padding:10px 12px;min-height:62px;box-sizing:border-box;display:grid;grid-template-columns:1fr auto;grid-template-rows:auto 1fr;gap:6px 8px;min-width:0;transition:border-color .15s,background .15s,transform .08s}
.fcard.on{border-color:#3a5fa8;background:linear-gradient(135deg,var(--card) 60%,#0d1f3c)}
body.light .fcard.on{background:linear-gradient(135deg,var(--card) 60%,#dbe8ff)}
.fcard:active{transform:scale(.97);background:rgba(91,143,255,.08)}
.fcard .ficon{display:none}
.fcard .fname{grid-column:1 / -1;grid-row:1;font-size:12px;color:var(--fname);line-height:1.25;min-width:0;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;text-align:left;align-self:start}
.fcard .led{grid-column:2;grid-row:2;justify-self:end;align-self:end}
.fcard{cursor:pointer;-webkit-tap-highlight-color:transparent;user-select:none}
.led{width:14px;height:14px;border-radius:50%;background:#3a3f4b;border:1px solid #555;box-shadow:inset 0 1px 2px rgba(0,0,0,.6);transition:background .2s,box-shadow .3s,border-color .2s;flex-shrink:0}
body.light .led{background:#c8ccd3;border-color:#b0b4bc;box-shadow:inset 0 1px 2px rgba(0,0,0,.15)}
.fcard.on .led{background:#5b8fff;border-color:#7aa8ff;box-shadow:0 0 8px #5b8fff,0 0 14px rgba(91,143,255,.6),inset 0 1px 2px rgba(255,255,255,.4);animation:ledPulse 2.2s ease-in-out infinite}
@keyframes ledPulse{0%,100%{box-shadow:0 0 6px #5b8fff,0 0 10px rgba(91,143,255,.5),inset 0 1px 2px rgba(255,255,255,.4)}50%{box-shadow:0 0 12px #5b8fff,0 0 20px rgba(91,143,255,.8),inset 0 1px 2px rgba(255,255,255,.4)}}
.feat-children{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;transition:opacity .25s;grid-column:span 3}
.feat-children.locked,#controlled.locked{}
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
亮是啊,.tabbar{position:fixed;bottom:12px;left:12px;right:12px;max-width:456px;margin:0 auto;display:flex;background:var(--tabbar);border:1px solid var(--border);border-radius:22px;padding:4px;box-shadow:0 6px 20px rgba(0,0,0,.28),0 2px 6px rgba(0,0,0,.18);z-index:100;transition:background .45s ease,border-color .45s ease,box-shadow .45s ease,-webkit-backdrop-filter .45s ease,backdrop-filter .45s ease}
body.light .tabbar{box-shadow:0 6px 18px rgba(0,0,0,.10),0 2px 6px rgba(0,0,0,.06)}
.tab{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;padding:8px 0 8px;background:none;border:none;border-radius:18px;color:var(--tab-c);cursor:pointer;font-size:10px;gap:3px;transition:color .45s ease,background .25s ease;letter-spacing:.02em}
.tab svg{width:22px;height:22px;stroke:currentColor;fill:none;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round}
.tab.act{color:#5b8fff;background:rgba(91,143,255,.14)}
body.light .tab.act{background:rgba(91,143,255,.12)}
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
.cfg-spd{font-size:12px;color:var(--lbl);width:64px;flex-shrink:0;white-space:nowrap}
.cfg-pct{font-size:12px;font-weight:600;color:#5b8fff;width:36px;text-align:right;flex-shrink:0}
.dbg-grp{margin-bottom:8px}
.dbg-grp-hdr{display:flex;justify-content:space-between;align-items:center;padding:12px 14px;cursor:pointer;user-select:none;background:var(--card);border:1px solid var(--border);border-radius:10px}
.dbg-grp-hdr.open{border-bottom-left-radius:0;border-bottom-right-radius:0}
.dbg-grp-title{font-size:14px;font-weight:700;color:var(--text)}
.dbg-grp-sub{font-size:11px;color:var(--h2);margin-left:8px}
.dbg-grp-body{display:none;background:var(--card);border:1px solid var(--border);border-top:none;border-radius:0 0 10px 10px;padding:2px 0}
.dbg-grp-body.open{display:block}
.dbg-row{display:grid;grid-template-columns:18px 1fr auto;gap:8px;padding:8px 12px;border-bottom:1px solid var(--sep);align-items:center}
.dbg-row:last-child{border-bottom:none}
.dbg-row.ovr{background:rgba(91,143,255,.06)}
.dbg-chk{width:16px;height:16px;margin:0;cursor:pointer;accent-color:#5b8fff}
.dbg-meta{min-width:0}
.dbg-name{font-size:12px;font-weight:600;color:var(--text);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.dbg-desc{font-size:11px;color:var(--lbl);line-height:1.35;margin-top:1px}
.dbg-pos{font-size:10px;color:var(--h2);margin-left:6px}
.dbg-now{font-size:11px;color:#5b8fff;margin-top:2px;font-variant-numeric:tabular-nums}
.dbg-ctrl{display:flex;align-items:center;justify-content:flex-end;min-width:96px}
.dbg-ctrl .inp{width:80px;margin-top:0;padding:4px 6px;font-size:12px;text-align:right}
.dbg-ctrl select.inp{padding:4px 4px;text-align:left;width:118px}
.dbg-ctrl .sw{width:36px;height:20px}
.dbg-ctrl .sl::before{width:14px;height:14px}
.dbg-ctrl input:checked+.sl::before{transform:translateX(16px)}
.dbg-tip{font-size:11px;color:var(--lbl);padding:8px 2px;line-height:1.5}
.dbg-actions{display:flex;gap:8px;margin-bottom:10px}
.dbg-actions .btn{margin-top:0;flex:1}
/* ===== 响应式：≤640px 维持手机原样；≥641px 按屏幕档位放宽并提升列数 ===== */
@media (min-width:641px){
  body{max-width:720px;padding:20px 18px 84px}
  .tabbar{max-width:696px}
  .grid{grid-template-columns:repeat(4,1fr)}
  .flist{grid-template-columns:repeat(4,1fr)}
  .fcard.wide{grid-column:span 2}
  .feat-children{grid-template-columns:repeat(4,1fr);grid-column:span 4}
}
@media (min-width:1025px){
  body{max-width:960px;padding:24px 20px 88px}
  .tabbar{max-width:936px}
  .grid{grid-template-columns:repeat(5,1fr)}
  .flist{grid-template-columns:repeat(5,1fr)}
  .fcard.wide{grid-column:span 2}
  .feat-children{grid-template-columns:repeat(5,1fr);grid-column:span 5}
  .htitle{font-size:20px}
}
@media (min-width:1440px){
  body{max-width:1120px}
  .tabbar{max-width:1096px}
  .grid{grid-template-columns:repeat(6,1fr)}
  .flist{grid-template-columns:repeat(6,1fr)}
  .fcard.wide{grid-column:span 2}
  .feat-children{grid-template-columns:repeat(6,1fr);grid-column:span 6}
}
/* ===== 注入中全局动效 ===== */
/* 主题感知颜色：全部通过变量驱动，切暗/亮时即时跟随 */
:root{
  --inj-a:91,143,255;   /* 主调蓝（暗色背景更显眼） */
  --inj-b:122,168,255;  /* 副调亮蓝 */
  --inj-c:168,200,255;  /* 高光 */
  --inj-alpha-glow:.22;
  --inj-alpha-edge:.55;
  --inj-alpha-aurora:.28;
}
body.light{
  --inj-a:58,95,168;
  --inj-b:91,143,255;
  --inj-c:140,180,240;
  --inj-alpha-glow:.14;
  --inj-alpha-edge:.38;
  --inj-alpha-aurora:.18;
}
/* 背景柔和呼吸光晕：两处径向渐变缓慢脉动，营造"脉搏"感 */
.inj-glow{position:fixed;inset:0;pointer-events:none;z-index:-1;opacity:0;transition:opacity .6s ease}
body.injecting .inj-glow{opacity:1;animation:injPulse 5.5s ease-in-out infinite}
.inj-glow::before,.inj-glow::after{content:'';position:absolute;width:70vmax;height:70vmax;border-radius:50%;filter:blur(60px)}
.inj-glow::before{top:-20vmax;left:-20vmax;background:radial-gradient(circle,rgba(var(--inj-a),var(--inj-alpha-glow)) 0%,rgba(var(--inj-a),0) 65%)}
.inj-glow::after{bottom:-25vmax;right:-25vmax;background:radial-gradient(circle,rgba(var(--inj-b),calc(var(--inj-alpha-glow) * .75)) 0%,rgba(var(--inj-b),0) 65%)}
@keyframes injPulse{0%,100%{opacity:.7}50%{opacity:1}}
/* 极光层：四个彩色光斑在背景里缓慢旋转+位移，像流动的极光，不像进度条 */
.inj-aurora{position:fixed;inset:-20%;pointer-events:none;z-index:-1;opacity:0;transition:opacity .6s ease;filter:blur(80px)}
body.injecting .inj-aurora{opacity:1;animation:injAuroraSpin 28s linear infinite}
.inj-aurora::before,.inj-aurora::after{content:'';position:absolute;inset:0;background:
  radial-gradient(ellipse 40% 30% at 20% 25%,rgba(var(--inj-a),var(--inj-alpha-aurora)) 0%,transparent 60%),
  radial-gradient(ellipse 35% 28% at 80% 30%,rgba(var(--inj-c),calc(var(--inj-alpha-aurora) * .7)) 0%,transparent 60%),
  radial-gradient(ellipse 45% 32% at 75% 80%,rgba(var(--inj-b),var(--inj-alpha-aurora)) 0%,transparent 60%),
  radial-gradient(ellipse 38% 30% at 25% 75%,rgba(var(--inj-a),calc(var(--inj-alpha-aurora) * .8)) 0%,transparent 60%)}
.inj-aurora::after{animation:injAuroraDrift 14s ease-in-out infinite alternate;mix-blend-mode:screen}
body.light .inj-aurora::after{mix-blend-mode:multiply}
@keyframes injAuroraSpin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}
@keyframes injAuroraDrift{0%{transform:translate(-4%,-3%) scale(1)}100%{transform:translate(5%,4%) scale(1.08)}}
/* 左右边缘：容器只管定位/发光，动画放到::before上； */
/* 这样主元素不带动画，切主题时 var() 能被正常重新解析，不会因合成层缓存导致色值冻结。 */
.inj-edge{position:fixed;top:0;bottom:0;width:2px;pointer-events:none;z-index:9998;opacity:0;transition:opacity .4s ease,box-shadow .2s ease}
.inj-edge.l{left:0;box-shadow:0 0 10px rgba(var(--inj-a),var(--inj-alpha-edge))}
.inj-edge.r{right:0;box-shadow:0 0 10px rgba(var(--inj-a),var(--inj-alpha-edge))}
.inj-edge::before{content:'';position:absolute;inset:0;background:linear-gradient(180deg,
  transparent 0%,
  rgba(var(--inj-a),0) 10%,
  rgba(var(--inj-a),var(--inj-alpha-edge)) 25%,
  rgba(var(--inj-a),0) 40%,
  rgba(var(--inj-b),var(--inj-alpha-edge)) 55%,
  rgba(var(--inj-b),0) 70%,
  rgba(var(--inj-a),var(--inj-alpha-edge)) 85%,
  transparent 100%);background-size:100% 300%}
body.injecting .inj-edge{opacity:.85}
body.injecting .inj-edge.l::before{animation:injEdgeDown 6s linear infinite}
body.injecting .inj-edge.r::before{animation:injEdgeUp 6s linear infinite}
@keyframes injEdgeDown{0%{background-position:0 -100%}100%{background-position:0 200%}}
@keyframes injEdgeUp{0%{background-position:0 200%}100%{background-position:0 -100%}}
/* 毛玻璃（glassmorphism）：注入开启时，卡片变成半透明磨砂玻璃，让背景的极光透过来 */
/* 卡片自身保持平滑过渡，避免切换瞬间的硬跳变 */
.card,.tile,.fcard,.master-card{transition:background .45s ease,border-color .45s ease,backdrop-filter .45s ease,-webkit-backdrop-filter .45s ease,box-shadow .25s ease,transform .08s}
/* 深色：玻璃偏冷白，微弱反光边缘 */
body.injecting{--glass-bg:rgba(30,36,52,.72);--glass-bg-on:rgba(58,95,168,.60);--glass-border:rgba(255,255,255,.12);--glass-border-on:rgba(122,168,255,.60);--glass-shadow:0 4px 24px rgba(0,0,0,.32)}
body.injecting.light{--glass-bg:rgba(255,255,255,.72);--glass-bg-on:rgba(219,232,255,.82);--glass-border:rgba(91,143,255,.22);--glass-border-on:rgba(58,95,168,.48);--glass-shadow:0 4px 18px rgba(58,95,168,.14)}
body.injecting .card,
body.injecting .tile,
body.injecting .fcard,
body.injecting .master-card{
  background:var(--glass-bg);
  border-color:var(--glass-border);
  -webkit-backdrop-filter:blur(14px) saturate(160%);
  backdrop-filter:blur(14px) saturate(160%);
  box-shadow:var(--glass-shadow);
}
/* 激活态（已开启的功能卡 / 总开关），用带色玻璃 + 更亮的边缘反光 */
body.injecting .fcard.on,
body.injecting .master-card.on{
  background:var(--glass-bg-on);
  border-color:var(--glass-border-on);
  -webkit-backdrop-filter:blur(14px) saturate(180%);
  backdrop-filter:blur(14px) saturate(180%);
}
/* :active 按下效果在玻璃态下也保留反馈，但用更柔和的玻璃反光 */
body.injecting .fcard:active{background:rgba(91,143,255,.22)}
/* 卡内元素同步玻璃化，避免"卡透了、里面按钮/输入/Wi-Fi 行还实色"的突兀感 */
/* 注意：.pbtn.act（当前选中档位）与 .btn（强调按钮）保持实色，保留强视觉区分 */
.pbtn,.inp,.net{transition:background .45s ease,border-color .45s ease,backdrop-filter .45s ease,-webkit-backdrop-filter .45s ease,color .15s ease}
body.injecting .pbtn:not(.act),
body.injecting .inp,
body.injecting .net{
  background:var(--glass-bg);
  border-color:var(--glass-border);
  -webkit-backdrop-filter:blur(10px) saturate(160%);
  backdrop-filter:blur(10px) saturate(160%);
}
/* 注入态下档位按钮（最慢/舒适/…）未选中文字对齐其他标签亮度，避免在玻璃背景上显得过暗 */
body.injecting .pbtn:not(.act){color:var(--tlbl)}
body.injecting .pbtn:not(.act):hover,
body.injecting .net:hover{
  background:var(--glass-bg-on);
  border-color:var(--glass-border-on);
}
/* 圆形 iOS 开关：关闭态底座跟随玻璃化；打开态（蓝色）保持实色作为强状态指示 */
.sl{transition:background .45s ease,-webkit-backdrop-filter .45s ease,backdrop-filter .45s ease}
body.injecting input:not(:checked)+.sl{
  background:var(--glass-bg);
  -webkit-backdrop-filter:blur(8px) saturate(160%);
  backdrop-filter:blur(8px) saturate(160%);
}
/* 分段开关（关闭注入/开启注入、跟着距离/固定）：外壳玻璃化，蓝色滑块保持实色 */
.segsw,.thsw{transition:background .45s ease,-webkit-backdrop-filter .45s ease,backdrop-filter .45s ease}
body.injecting .segsw,
body.injecting .thsw{
  background:var(--glass-bg);
  -webkit-backdrop-filter:blur(8px) saturate(160%);
  backdrop-filter:blur(8px) saturate(160%);
}
/* 注入态下每组卡片标题 h2 提亮：暗色 → 纯白；亮色 → 深色（在玻璃背景上更清晰） */
body.injecting h2{color:#fff}
body.injecting.light h2{color:#1c1c1e}
/* 注入态下底部 Tab 栏玻璃化，未激活文字提亮，激活仍保持蓝色 */
body.injecting .tabbar{
  background:var(--glass-bg);
  border-color:var(--glass-border);
  box-shadow:0 8px 24px rgba(0,0,0,.32),0 2px 8px rgba(0,0,0,.22);
  -webkit-backdrop-filter:blur(14px) saturate(160%);
  backdrop-filter:blur(14px) saturate(160%);
}
body.injecting .tab{color:rgba(255,255,255,.78)}
body.injecting.light .tab{color:rgba(28,28,30,.72)}
body.injecting .tab.act{color:#5b8fff}
/* 尊重用户"减少动画"偏好（无障碍） */
@media (prefers-reduced-motion:reduce){
  body.injecting .inj-glow,
  body.injecting .inj-aurora,
  body.injecting .inj-aurora::after,
  body.injecting .inj-edge.l::before,
  body.injecting .inj-edge.r::before{animation:none}
}
</style>
</head>
<body>
<!-- 注入中全局动效层（仅当 body.injecting 时可见） -->
<div class="inj-glow" aria-hidden="true"></div>
<div class="inj-aurora" aria-hidden="true"></div>
<div class="inj-edge l" aria-hidden="true"></div>
<div class="inj-edge r" aria-hidden="true"></div>
<div class="hdr">
  <span class="htitle">Tesla CAN 助手</span>
  <div style="display:flex;align-items:center;gap:10px">
    <span id="dot"></span>
    <div class="segsw" id="seg_inject" onclick="toggleInject()">
      <span class="segopt">关闭注入</span>
      <span class="segopt">开启注入</span>
      <span class="segknob"></span>
    </div>
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
<div id="state_root" class="grid"></div>

<h2>BLE 探针</h2>
<div class="card">
  <div class="row"><span class="lbl">广播状态</span><span id="ble_adv_st" class="val">--</span></div>
  <div class="row"><span class="lbl">广播 UUID</span><span id="ble_uuid_show" class="val" style="max-width:66%;overflow:hidden;text-overflow:ellipsis;white-space:nowrap">--</span></div>
  <div class="row"><span class="lbl">窗口 / 时长</span><span id="ble_window_show" class="val">--</span></div>
</div>
<div class="card">
  <div class="tog">
    <span class="tlbl">启用 BLE 广播</span>
    <label class="sw"><input type="checkbox" id="ble_enabled"><span class="sl"></span></label>
  </div>
  <div class="pad" style="border-top:1px solid var(--sep)">
    <span class="lbl">广播 UUID</span>
    <div class="srow">
      <input type="text" id="ble_uuid" class="inp" placeholder="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx">
      <button class="btn secondary" onclick="genBleUuid()">生成</button>
    </div>
    <div class="srow">
      <div style="flex:1">
        <span class="lbl">广播窗口（秒）</span>
        <input type="number" id="ble_window_sec" class="inp" min="1" max="3600" step="1">
      </div>
      <div style="flex:1">
        <span class="lbl">广播时长（秒）</span>
        <input type="number" id="ble_duration_sec" class="inp" min="1" max="3600" step="1">
      </div>
    </div>
    <button class="btn" onclick="saveBle()">保存 BLE 配置</button>
  </div>
</div>

<div id="controlled" class="locked">
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

<div id="schema_root"></div>

</div><!-- controlled -->
</div><!-- pg_main -->

<div id="pg_wifi" class="pg">
<h2>WiFi — 热点 (AP)</h2>
<div class="card">
  <div class="row"><span class="lbl">SSID</span><span id="ap_ssid" class="val">--</span></div>
  <div class="row"><span class="lbl">IP</span><span id="ap_ip" class="val">--</span></div>
  <div class="row"><span class="lbl">已连接设备</span><span id="ap_cli" class="val">--</span></div>
  <div class="row"><span class="lbl">热点状态</span><span id="ap_dis_lbl" class="val">--</span></div>
</div>
<div class="card pad">
  <button class="btn" id="ap_toggle_btn" onclick="toggleAp()">--</button>
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
</div><!-- pg_wifi -->

<div id="pg_debug" class="pg">
<div class="dbg-tip">调试页：默认只读当前 CAN 实时解码值。仅当勾选右侧“覆盖”后，网页给定的值才会写入帧并发送。覆盖默认只在内存，重启即清；点“保存存档”后会落到设备 NVS，重启后自动恢复。<br>红线：1021 mux 选择器 (bit 0–2) 与 ban protection (bit 52) 不可被覆盖，已隐藏。</div>
<div class="dbg-actions">
  <button class="btn" onclick="dbgSaveArchive()">保存存档</button>
  <button class="btn danger" onclick="dbgClearAll()">清除全部覆盖</button>
</div>
<div id="dbg_root"></div>
</div><!-- pg_debug -->

<nav class="tabbar">
  <button class="tab act" id="tab_main" onclick="switchTab('main')">
    <svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/></svg>
    主页
  </button>
  <!--<button class="tab" id="tab_debug" onclick="switchTab('debug')" >
    <svg viewBox="0 0 24 24"><path d="M12 2v3"/><path d="M4 7l2 2"/><path d="M20 7l-2 2"/><path d="M7 20l-2-2"/><path d="M17 20l2-2"/><rect x="7" y="8" width="10" height="12" rx="4"/><path d="M9 12h6"/><path d="M9 16h6"/></svg>
    调试
  </button>-->
  <button class="tab" id="tab_wifi" onclick="switchTab('wifi')">
    <svg viewBox="0 0 24 24"><path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/><path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1" fill="currentColor"/></svg>
    WiFi / 系统
  </button>
</nav>

<script>
const GW=['无','高速','增强','自动驾驶','基础'];

// 主页大部分开关由 /schema 提供的元数据驱动渲染。
// SCHEMA = { cnf:[...可写字段...], state:[...只读字段...] }
let SCHEMA = { cnf: [], state: [] };
const SCHEMA_GRP_COLORS = {'FSD':'#5b8fff','安全':'#f5a623','系统':'#3dba72'};

// 状态小卡片结构：tile_key -> { label, fields:[schema 条目，按在 SCHEMA.state 中的顺序], sep }
// tile_key 由 schema 的 f.tile 提供；f.tile==null 时 tile_key 退化为 f.key（字段独占一卡片）。
let STATE_TILES = [];       // 按添加顺序保存的 tile_key 数组
let STATE_TILE_MAP = {};    // tile_key -> { label, fields:[], sep }

function stateEnumText(f, v) {
  if (!f.enum_labels) return v;
  const arr = f.enum_labels.split('|');
  return (v >= 0 && v < arr.length) ? arr[v] : String(v);
}

function renderStateTiles() {
  STATE_TILES = [];
  STATE_TILE_MAP = {};
  (SCHEMA.state || []).forEach(f => {
    const tkey = f.tile ? f.tile : f.key;
    if (!(tkey in STATE_TILE_MAP)) {
      STATE_TILE_MAP[tkey] = { label: f.tile_label || f.label, fields: [], sep: f.tile_sep || ' / ' };
      STATE_TILES.push(tkey);
    }
    const t = STATE_TILE_MAP[tkey];
    if (!t.label && f.tile_label) t.label = f.tile_label;
    if (f.tile_sep) t.sep = f.tile_sep;
    t.fields.push(f);
  });

  const root = document.getElementById('state_root');
  root.innerHTML = '';
  // 固定附加一个 “运行时间” tile（uptime 不在 schema中，由 web 层直接下发）
  const addTile = (id, lbl) => {
    root.insertAdjacentHTML('beforeend',
      '<div class="tile"><span class="tlb">'+lbl+'</span><span id="'+id+'" class="tval">--</span></div>');
  };
  // 顺序：CAN 总线（如果在 schema 中）→运行时间→其余 schema tile
  //  为简单，直接按 schema 顺序渲染，在 can_online tile 之后插入 uptime；如果没 can_online tile 则放在最前。
  let uptimeInserted = false;
  STATE_TILES.forEach(tk => {
    const t = STATE_TILE_MAP[tk];
    addTile('tile_'+tk, t.label || tk);
    if (!uptimeInserted && tk === 'can_online') {
      addTile('tile__uptime', '运行时间');
      uptimeInserted = true;
    }
  });
  if (!uptimeInserted) {
    root.insertAdjacentHTML('afterbegin',
      '<div class="tile"><span class="tlb">运行时间</span><span id="tile__uptime" class="tval">--</span></div>');
  }
}

function renderSchemaGroups() {
  const root = document.getElementById('schema_root');
  root.innerHTML = '';
  // 按 group 分桶，保持 schema 本身顺序。
  // hidden 字段有前端自定义控件，跳过。
  const order = [];
  const groups = {};
  (SCHEMA.cnf || []).forEach(f => {
    if (f.hidden) return;
    const g0 = f.group || '其他';
    if (!(g0 in groups)) { groups[g0] = []; order.push(g0); }
    groups[g0].push(f);
  });
  order.forEach(g0 => {
    const h2 = document.createElement('h2'); h2.textContent = g0; root.appendChild(h2);
    // 同一个 group 内分两桶：bool -> 卡片墙 .flist/.fcard；其它 -> .card/.tog 行列表
    const bools = groups[g0].filter(f => f.type === 'bool');
    const others = groups[g0].filter(f => f.type !== 'bool');
    if (bools.length > 0) {
      const list = document.createElement('div'); list.className = 'flist';
      bools.forEach(f => list.insertAdjacentHTML('beforeend', schemaRowHtml(f)));
      root.appendChild(list);
    }
    if (others.length > 0) {
      const card = document.createElement('div'); card.className = 'card';
      others.forEach(f => card.insertAdjacentHTML('beforeend', schemaRowHtml(f)));
      root.appendChild(card);
    }
  });
  autosizeFcards();
}

// 名称单行放不下的 .fcard 自动扩展占位：1→2→3 格，只扩到刚好装下为止
function autosizeFcards() {
  document.querySelectorAll('.fcard').forEach(c => {
    c.classList.remove('wide');
    c.classList.remove('w3');
    const n = c.querySelector('.fname');
    if (!n) return;
    if (n.scrollWidth > n.clientWidth + 1) {
      c.classList.add('wide');
      if (n.scrollWidth > n.clientWidth + 1) {
        c.classList.remove('wide');
        c.classList.add('w3');
      }
    }
  });
}
let _fcResizeT = 0;
window.addEventListener('resize', () => {
  clearTimeout(_fcResizeT);
  _fcResizeT = setTimeout(autosizeFcards, 120);
});

function schemaRowHtml(f) {
  // bool 渲染为卡片（.fcard），自动排入外层 .flist 网格
  if (f.type === 'bool') {
    return '<label class="fcard" id="fc_'+f.key+'" for="'+f.key+'">' +
           '<input type="checkbox" id="'+f.key+'" style="display:none" onchange="onBoolCardChange(\''+f.key+'\',this.checked)">' +
           '<span class="ficon"></span><span class="fname">'+f.label+'</span><span class="led"></span></label>';
  }
  if (f.type === 'enum' && Array.isArray(f.options)) {
    let opts = f.options.map(o => '<option value="'+o.v+'">'+o.l+'</option>').join('');
    return '<div class="tog"><span class="tlbl">'+f.label+'</span>' +
           '<select id="'+f.key+'" class="inp" style="width:auto;margin-top:0" onchange="setConf(\''+f.key+'\',+this.value)">'+opts+'</select></div>';
  }
  // number
  const min = (f.min!==undefined?f.min:0), max = (f.max!==undefined?f.max:255);
  return '<div class="tog"><span class="tlbl">'+f.label+'</span>' +
         '<input type="number" id="'+f.key+'" class="inp" style="width:90px;margin-top:0" min="'+min+'" max="'+max+'" onchange="setConf(\''+f.key+'\',+this.value)"></div>';
}

async function loadSchema() {
  try {
    const r = await fetch('/schema');
    const d = await r.json();
    // 兼容：新格式 {cnf,state}；旧版数组格式暂不再支持。
    SCHEMA = (d && typeof d === 'object' && !Array.isArray(d)) ? { cnf: d.cnf || [], state: d.state || [] }
                                                               : { cnf: [], state: [] };
  } catch(e) { SCHEMA = { cnf: [], state: [] }; }
  renderStateTiles();
  renderSchemaGroups();
}

function syncMaster(on) {
  document.getElementById('controlled').classList.toggle('locked', !on);
  const card = document.getElementById('master-card');
  if (card) card.classList.toggle('on', on);
  document.body.classList.toggle('injecting', !!on);
}
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

function toggleInject() {
  const seg = g('seg_inject');
  const on = !seg.classList.contains('right');
  seg.classList.toggle('right', on);
  setConf('enable_inject', on);
  syncMaster(on);
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
        '<input type="range" class="slider" style="flex:1" min="0" max="50" value="'+entry.pct+'" oninput="onCfgSlide('+i+',this.value)">' +
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
// schema 驱动 bool 卡片切换：先置卡片 on 态，再下发设置。
function onBoolCardChange(key, on) {
  const fc = document.getElementById('fc_'+key);
  if (fc) fc.classList.toggle('on', !!on);
  setConf(key, !!on);
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
  // 运行时间（非 schema）
  ttxt('tile__uptime', fmtUp(s.uptime));

  // Schema 驱动：按 tile 合并字段
  STATE_TILES.forEach(tk => {
    const t = STATE_TILE_MAP[tk];
    const id = 'tile_'+tk;
    const el = g(id);
    if (!el) return;
    // 先把所有字段渲染成“值文本”（不带单位），同时记录“这个字段是否真有值（用于末尾贴单位）”。
    // 单位策略：同一 tile 内，如果所有“有 unit 且有值”的字段共用同一个 unit，则仅在整体末尾贴一次；
    //          否则（unit 不一致）退回老行为：每个元素各自带自己的 unit，避免丢失语义。
    let commonUnit = null;     // 记录观察到的第一个 unit
    let unitUniform = true;    // 是否所有字段的 unit 都一致
    let anyValued = false;     // 是否至少有一个字段取到了值
    t.fields.forEach(f => {
      const v = s[f.key];
      if (v === undefined || v === null) return;
      anyValued = true;
      if (!f.unit) { unitUniform = false; return; }
      if (commonUnit === null) commonUnit = f.unit;
      else if (commonUnit !== f.unit) unitUniform = false;
    });
    const perFieldUnit = !unitUniform;    // true=每个元素各自带 unit；false=仅末尾合并
    const parts = t.fields.map(f => {
      const v = s[f.key];
      if (v === undefined || v === null) return '--';
      let txt = (f.type === 'enum') ? stateEnumText(f, +v) : String(v);
      if (perFieldUnit && f.unit) txt += ' ' + f.unit;
      return txt;
    });
    let joined = parts.join(t.sep || ' / ');
    if (!perFieldUnit && anyValued && commonUnit) joined += ' ' + commonUnit;
    el.textContent = joined;

    // CAN 总线特殊着色：在线绿、离线红
    if (tk === 'can_online') {
      const on = !!s.can_online;
      el.className = 'tval ' + (on ? 'ok' : 'err');
    } else {
      el.className = 'tval';
    }
  });
}
function ttxt(id, t, cls) {
  const e = g(id);
  e.textContent = t;
  e.className = 'tval' + (cls ? ' '+cls : '');
}

function updateCnf(c) {
  const segInj = g('seg_inject'); if (segInj) segInj.classList.toggle('right', !!c.enable_inject);
  // Schema 驱动字段统一按类型回填（只处理 cnf 字段，state 字段不会出现在表单中）
  // hidden 字段由手写 UI 自己回填，这里跳过。
  (SCHEMA.cnf || []).forEach(f => {
    if (f.hidden) return;
    const e = g(f.key); if (!e) return;
    const v = c[f.key];
    if (f.type === 'bool') {
      if (e.type === 'checkbox') e.checked = !!v;
      const fc = document.getElementById('fc_'+f.key);
      if (fc) fc.classList.toggle('on', !!v);
    } else {
      if (document.activeElement !== e) e.value = (v !== undefined ? v : '');
    }
  });
  const hw3 = g('enable_set_hw3_profile'); if (hw3) hw3.checked = !!c.enable_set_hw3_profile;
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
    if ((SCHEMA.cnf || []).length === 0 && (SCHEMA.state || []).length === 0) await loadSchema();
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
    txt('ap_dis_lbl', d.disabled ? '已关闭 (仅STA)' : '运行中', d.disabled ? 'warn' : 'ok');
    g('ap_toggle_btn').textContent = d.disabled ? '开启热点 (AP)' : '关闭热点，仅STA模式';
    g('ap_toggle_btn')._apDisabled = d.disabled;
  } catch(e) {}
}
async function toggleAp() {
  const dis = !g('ap_toggle_btn')._apDisabled;
  const p = new URLSearchParams({disabled: dis ? '1' : '0'});
  try {
    const r = await fetch('/ap_config', {method:'POST', body:p});
    const d = await r.json();
    if (!d.ok) { alert('失败：' + d.error); return; }
    alert('已保存，重启后生效。');
  } catch(e) { alert('请求失败'); }
}
async function loadSta() {
  try {
    const d = await (await fetch('/wifi_status')).json();
    txt('sta_st',   d.connected ? '已连接' : '未连接', d.connected ? 'ok' : 'warn');
    txt('sta_ssid', d.ssid || '--');
    txt('sta_ip',   d.connected ? (d.ip||'--') : '--');
  } catch(e) {}
}

async function loadBle() {
  try {
    const d = await (await fetch('/ble_status')).json();
    const advText = !d.enabled ? '已关闭' : (d.advertising ? '正在广播' : '等待窗口');
    txt('ble_adv_st', advText, !d.enabled ? 'warn' : (d.advertising ? 'ok' : 'warn'));
    txt('ble_uuid_show', d.uuid || '--');
    const rem = d.advertising ? ('剩余 ' + d.adv_remaining_sec + 's') : ('下次窗口 ' + d.cycle_remaining_sec + 's');
    txt('ble_window_show', d.window_sec + 's / ' + d.duration_sec + 's · ' + rem);
    const en = g('ble_enabled'); if (en) en.checked = !!d.enabled;
    const uuid = g('ble_uuid'); if (uuid && document.activeElement !== uuid) uuid.value = d.uuid || '';
    const win = g('ble_window_sec'); if (win && document.activeElement !== win) win.value = d.window_sec;
    const dur = g('ble_duration_sec'); if (dur && document.activeElement !== dur) dur.value = d.duration_sec;
  } catch(e) {
    txt('ble_adv_st', '未知', 'err');
  }
}

async function genBleUuid() {
  try {
    const d = await (await fetch('/ble_generate', {method:'POST'})).json();
    g('ble_uuid').value = d.uuid;
  } catch(e) {
    alert('生成 UUID 失败');
  }
}

async function saveBle() {
  const uuid = g('ble_uuid').value.trim();
  const windowSec = Math.max(1, Math.min(3600, +(g('ble_window_sec').value || 0)));
  const durationSec = Math.max(1, Math.min(3600, +(g('ble_duration_sec').value || 0)));
  const p = new URLSearchParams({
    uuid,
    window_sec: String(windowSec),
    duration_sec: String(durationSec),
    enabled: g('ble_enabled').checked ? '1' : '0'
  });
  try {
    const r = await fetch('/ble_config', {method:'POST', body:p});
    const d = await r.json();
    if (!d.ok) { alert('保存失败：' + (d.error || '配置无效')); return; }
    await loadBle();
  } catch(e) {
    alert('保存 BLE 配置失败');
  }
}

async function scanWifi() {
  g('nets').innerHTML = '<div style="color:#555;padding:6px 0">扫描中...</div>';
  try {
    for (let i = 0; i < 10; i++) {
      const resp = await fetch('/wifi_scan');
      const d = await resp.json();
      if (d.scanning) { await new Promise(r => setTimeout(r, 1500)); continue; }
      g('nets').innerHTML = d.networks.map(n => {
        const esc = n.ssid.replace(/&/g,'&amp;').replace(/"/g,'&quot;').replace(/'/g,'&#39;');
        return '<div class="net" onclick="g(\'sta_ssid_in\').value=\''+esc+'\'">'+
               '<span>'+esc+(n.enc?' &#128274;':'')+'</span>'+
               '<span class="rssi">'+n.rssi+' dBm</span></div>';
      }).join('');
      return;
    }
    g('nets').innerHTML = '<div style="color:#ff4f4f;padding:6px 0">扫描超时</div>';
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

// ── 调试页 ─────────────────────────────────────────────────────

// 信号字典：按 1016.csv / 1021.csv 整理；bit 52 (ban protection) 与 1021 mux 选择器 (bit 0..2) 已剔除。
// 字段：name, desc, start(bit), len(bit), enums(可选：{值:"文本",...})
const DBG_SIGS = {
  'f1016': [
    {name:'UI_autopilotControlRequest',desc:'Autopilot 控制模式请求（Legacy/NextGen）',start:0,len:1,enums:{0:'传统横向控制',1:'新一代控制'}},
    {name:'UI_ulcStalkConfirm',desc:'ULC 拨杆确认（统一车道控制激活确认）',start:1,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_summonHeartbeat',desc:'Summon 心跳计数',start:2,len:2},
    {name:'UI_curvSpeedAdaptDisable',desc:'弯道自适应速度调整禁用',start:4,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_dasDeveloper',desc:'DAS 开发者模式（调试用途）',start:5,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableVinAssociation',desc:'VIN 关联功能启用',start:6,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_lssLkaEnabled',desc:'车道保持辅助（LKA）启用',start:7,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_lssLdwEnabled',desc:'车道偏离预警（LDW）启用',start:8,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_coastToCoast',desc:'跨州/长途 FSD 导航模式启用',start:9,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autoSummonEnable',desc:'自动召唤（Auto Summon）功能启用',start:10,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_exceptionListEnable',desc:'FSD 路线例外列表启用',start:11,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_roadCheckDisable',desc:'FSD 路况检测禁用',start:12,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_driveOnMapsEnable',desc:'基于地图的自动驾驶启用',start:13,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_handsOnRequirementDisable',desc:'禁用手握方向盘要求（关闭 Nag 触发条件）',start:14,len:1,enums:{0:'需要手握',1:'禁用要求'}},
    {name:'UI_ulcOffHighway',desc:'ULC 在非高速公路上启用',start:15,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_fuseLanesDisable',desc:'车道融合功能禁用',start:16,len:1,enums:{0:'启用融合',1:'禁用融合'}},
    {name:'UI_fuseHPPDisable',desc:'高精度地图预测（HPP）融合禁用',start:17,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_fuseVehiclesDisable',desc:'车辆融合感知禁用',start:18,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_enableClipParkedTelemetry',desc:'停车片段遥测上传启用',start:19,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_visionSpeedType',desc:'视觉限速识别时间窗口类型',start:20,len:2,enums:{0:'禁用',1:'1秒',2:'2秒',3:'优化'}},
    {name:'UI_curvatureDatabaseOnly',desc:'仅使用曲率数据库',start:22,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_lssElkEnabled',desc:'紧急车道保持（ELK）启用',start:23,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_summonExitType',desc:'Summon 退出停车位方式',start:24,len:2,enums:{0:'直行',1:'右转',2:'左转',3:'SNA'}},
    {name:'UI_summonEntryType',desc:'Summon 进入停车位方式',start:26,len:2,enums:{0:'直行',1:'右转',2:'左转',3:'SNA'}},
    {name:'UI_selfParkRequest',desc:'自动泊车/召唤动作请求',start:28,len:4,enums:{0:'无',1:'前进泊车',2:'后退泊车',3:'中止',4:'准备',5:'暂停',6:'继续',7:'自动召唤前进',8:'自动召唤后退',9:'取消',10:'已准备',11:'Smart Summon',12:'无操作',15:'SNA'}},
    {name:'UI_summonReverseDist',desc:'Summon 最大后退距离（63=SNA）',start:32,len:6},
    {name:'UI_undertakeAssistEnable',desc:'超车辅助（Undertake Assist）启用',start:38,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_adaptiveSetSpeedEnable',desc:'自适应设定速度启用',start:39,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_drivingSide',desc:'驾驶侧配置',start:40,len:2,enums:{0:'左舵',1:'右舵',2:'未知'}},
    {name:'UI_enableClipTelemetry',desc:'行车片段遥测上传启用',start:42,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableTripTelemetry',desc:'行程遥测上传启用',start:43,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableRoadSegmentTelemetry',desc:'路段遥测上传启用',start:44,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_accFollowDistanceSetting',desc:'★ ACC 跟车距离拨杆设置（速度档位来源）',start:45,len:3,enums:{0:'1格',1:'2格',2:'3格',3:'4格',4:'5格',5:'6格',6:'7格',7:'SNA'}},
    {name:'UI_hasDriveOnNav',desc:'具备基于导航的自动驾驶功能',start:48,len:1,enums:{0:'否',1:'是'}},
    {name:'UI_followNavRouteEnable',desc:'跟随导航路线自动变道启用',start:49,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_ulcSpeedConfig',desc:'ULC 速度控制激进程度',start:50,len:2,enums:{0:'禁用',1:'温和',2:'适中',3:'激进'}},
    {name:'UI_ulcBlindSpotConfig',desc:'ULC 盲区处理激进程度',start:52,len:2,enums:{0:'标准',1:'激进',2:'极限'}},
    {name:'UI_suppressExitPassingLane',desc:'抑制在超车道上的自动退出',start:54,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableClipStartStopTelemetry',desc:'行车片段起止事件遥测启用',start:55,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_alcOffHighwayEnable',desc:'非高速公路自动变道（ALC）启用',start:56,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_validationLoop',desc:'验证循环模式（测试用途）',start:57,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_smartSummonType',desc:'Smart Summon 召唤模式类型',start:58,len:2,enums:{0:'Pin定位',1:'寻找我',2:'智能泊车'}},
    {name:'UI_enableVisionOnlyStops',desc:'仅用视觉识别停车标志/红绿灯',start:60,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_source3D',desc:'3D 路径规划高程数据来源',start:61,len:2,enums:{0:'地图高程',1:'路径预测',2:'XYZ预测'}},
    {name:'UI_isaSpeedingChimeMuted',desc:'★ ISA 超速提示音静音',start:63,len:1,enums:{0:'未静音',1:'已静音'}},
  ],
  'f1021_m0': [
    {name:'UI_hovEnabled',desc:'HOV 高乘载车道功能启用',start:3,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_donDisableAutoWiperDuration',desc:'DON 自动雨刮禁用持续时间',start:4,len:3,enums:{0:'默认',1:'5s',2:'15s',3:'30s',4:'60s',5:'120s',6:'关'}},
    {name:'UI_donDisableOnAutoWiperSpeed',desc:'DON 在雨刮自动运行指定速度以上时禁用',start:7,len:4},
    {name:'UI_blindspotMinSpeed',desc:'盲区监测触发最低车速阈值',start:11,len:4,enums:{0:'默认',1:'5kph',2:'10kph',3:'15kph',4:'20kph',5:'25kph',6:'30kph',7:'35kph',8:'40kph',9:'45kph',10:'关'}},
    {name:'UI_blindspotDistance',desc:'盲区监测侧向距离阈值',start:15,len:3,enums:{0:'默认',1:'0.5m',2:'1m',3:'2m',4:'4m',5:'关'}},
    {name:'UI_blindspotTTC',desc:'盲区监测碰撞时间（TTC）阈值',start:18,len:3,enums:{0:'默认',1:'0.5s',2:'1s',3:'2s',4:'4s',5:'3s',6:'5s',7:'关'}},
    {name:'UI_donStopEndOfRampBuffer',desc:'DON 在匝道末端停车缓冲距离',start:21,len:3,enums:{0:'默认',1:'15m',2:'30m',3:'45m',4:'关'}},
    {name:'UI_donDisableCutin',desc:'禁用 DON 对切入车辆的检测',start:24,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_smartSetSpeedOffset',desc:'智能限速偏移量（-30 至 +33）',start:25,len:6},
    {name:'UI_smartSetSpeedOffsetType',desc:'智能限速偏移类型',start:31,len:1,enums:{0:'固定值',1:'百分比'}},
    {name:'UI_autopilotMonarchBackup',desc:'Autopilot Monarch 备份模式启用',start:32,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_fsdVisualizationEnabled',desc:'FSD 可视化界面启用',start:37,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_fsdStopsControlEnabled',desc:'FSD 停车点控制启用',start:38,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_fsdContinueOnGreenWithCIPV',desc:'FSD 有前车时绿灯自动继续行驶',start:39,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_smartSetSpeed',desc:'智能限速跟随',start:40,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_automaticSetSpeedOffset',desc:'自动速度偏移启用',start:41,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_apply2021_1958_ISA',desc:'应用欧盟 2021/1958 ISA 法规',start:42,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_apply2021_646_ELKS',desc:'应用欧盟 2021/646 ELKS 法规',start:43,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_apply2021_1341_DDAW',desc:'应用欧盟 2021/1341 DDAW 法规',start:44,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_homelinkNearby',desc:'检测到 Homelink 设备附近',start:45,len:1,enums:{0:'未检测到',1:'附近有设备'}},
    {name:'UI_enableFullSelfDriving',desc:'★ FSD 完全自动驾驶功能启用',start:46,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_hasFullSelfDriving',desc:'是否已购买 FSD 订阅',start:47,len:1,enums:{0:'未购买',1:'已购买'}},
    {name:'UI_autosteerActivation',desc:'Autosteer 激活方式',start:48,len:1,enums:{0:'单击',1:'双击'}},
    {name:'UI_autopilotDrivingProfile',desc:'Autopilot 跟车驾驶风格',start:49,len:2,enums:{0:'舒适',1:'标准',2:'激进'}},
    {name:'UI_fsdBetaRequest',desc:'请求激活 FSD Beta 测试版本',start:51,len:1,enums:{0:'未激活',1:'已激活'}},
    {name:'UI_disableOptionalLaneChanges',desc:'禁用 FSD 可选自动变道',start:53,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_applyR152_AEBS',desc:'应用 R152 法规（AEBS）',start:59,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autopilotControlMux0Valid',desc:'Mux 0 帧数据有效性标志',start:63,len:1,enums:{0:'无效',1:'有效'}},
  ],
  'f1021_m1': [
    {name:'UI_selectableCameraRequest',desc:'请求切换中控屏摄像头视角',start:8,len:4,enums:{0:'无',1:'自拍',2:'前主',3:'前鱼眼',4:'前窄',5:'左A柱',6:'左翼子板',7:'右A柱',8:'右翼子板',9:'倒车',10:'全景',11:'网格',12:'格栅',15:'不可用'}},
    {name:'UI_overrideSleepWithSuspend',desc:'挂起模式覆盖正常休眠',start:16,len:1,enums:{0:'禁用',1:'启用'}},
    {name:'UI_driverMonitorConfirmation',desc:'DMS 确认功能启用',start:17,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_parkAssistUseVision',desc:'泊车辅助优先使用视觉传感器',start:18,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_applyEceR79',desc:'★ 应用 ECE R79 法规（Nag 触发位）',start:19,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableMapStops',desc:'FSD 启用地图停车点',start:20,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_disableMain',desc:'禁用前置主摄像头',start:23,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableNarrow',desc:'禁用前置窄角摄像头',start:24,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableFisheye',desc:'禁用前置鱼眼摄像头',start:25,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableLeftPillar',desc:'禁用左 A 柱摄像头',start:26,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableRightPillar',desc:'禁用右 A 柱摄像头',start:27,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableLeftRepeater',desc:'禁用左翼子板摄像头',start:28,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableRightRepeater',desc:'禁用右翼子板摄像头',start:29,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableBackup',desc:'禁用倒车摄像头',start:30,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_disableRadar',desc:'禁用毫米波雷达',start:31,len:1,enums:{0:'启用',1:'禁用'}},
    {name:'UI_noStalkConfirmAlertHaptic',desc:'无拨杆确认时方向盘振动警报',start:32,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_regulatoryLKA',desc:'法规 LKA 强制启用',start:33,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_regulatoryLaneAssistLevel',desc:'法规车道辅助介入强度',start:34,len:1,enums:{0:'仅警告',1:'主动辅助'}},
    {name:'UI_ulcSnooze',desc:'ULC 临时抑制',start:36,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_noStalkConfirmAlertChime',desc:'无拨杆确认时声音提示警报',start:37,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_factorySummonEnable',desc:'工厂模式 Summon 启用',start:39,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_apmv3Branch',desc:'APM v3 软件分支版本',start:40,len:3,enums:{0:'正式',1:'预发布',2:'开发',3:'预发布2',4:'EAP',5:'演示'}},
    {name:'UI_enableCabinCamera',desc:'启用车内驾驶员监控摄像头',start:43,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableAutopilotStopWarning',desc:'Autopilot 主动停车前的提前警告',start:44,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_showLaneGraph',desc:'FSD 可视化中显示车道图',start:45,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_showTrackLabels',desc:'FSD 可视化中显示目标跟踪标签',start:46,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_hardCoreSummon',desc:'★ Hardcore Summon（解除距离限制）',start:47,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableCabinCameraTelemetry',desc:'车内摄像头遥测上传启用',start:48,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableVisionSpeedControl',desc:'纯视觉限速识别控制启用',start:49,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autopilotTelemetryInChina',desc:'中国区 AP 数据遥测合规开关',start:50,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableTeslaAutopark',desc:'Tesla 自动泊车（Autopark）启用',start:51,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autoTurnSignalMode',desc:'Autopilot 自动转向灯模式',start:52,len:1,enums:{0:'关',1:'自动取消'}},
    {name:'UI_enableCautionLightControl',desc:'警示灯（双闪）自动控制启用',start:54,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_applyEceR79SmartSummonOnly',desc:'仅对 Smart Summon 应用 R79 手握限制',start:55,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autopilotMonarchEnabled',desc:'Autopilot Monarch 单机推理启用',start:56,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autopilotEphemerisEnabled',desc:'Autopilot 星历数据启用',start:57,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableCabinAudioRecording',desc:'启用车内音频录制功能',start:59,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autopilotControlMux1Valid',desc:'Mux 1 帧数据有效性标志',start:63,len:1,enums:{0:'无效',1:'有效'}},
  ],
  'f1021_m2': [
    {name:'UI_enableApproachingEmergencyVehicleDetection',desc:'★ 紧急车辆检测（EVD）',start:5,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableStartFsdFromParkBrakeConfirmation',desc:'允许从驻车制动确认后启动 FSD',start:6,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_enableStartFsdFromPark',desc:'允许从 P 档直接启动 FSD',start:7,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_fsdMaxSpeedOffsetPercentage',desc:'FSD 行驶超速最大百分比 (0–63 %)',start:8,len:6},
    {name:'UI_coldStartMonarchInFactory',desc:'工厂冷启动初始化 Monarch',start:16,len:1,enums:{0:'关',1:'开'}},
    {name:'UI_autopilotControlMux2Valid',desc:'Mux 2 帧数据有效性标志',start:63,len:1,enums:{0:'无效',1:'有效'}},
  ],
};

const DBG_GROUPS = [
  {key:'f1016',    title:'CAN 1016',        sub:'UI_autopilotControl (跟车距离 / LKA / Summon 等)', frame:'1016'},
  {key:'f1021_m0', title:'CAN 1021 · mux 0', sub:'FSD / 盲区 / 智能限速 / Monarch',                frame:'1021_m0'},
  {key:'f1021_m1', title:'CAN 1021 · mux 1', sub:'Nag / 摄像头 / 泊车 / Summon',                   frame:'1021_m1'},
  {key:'f1021_m2', title:'CAN 1021 · mux 2', sub:'EVD / FSD from Park / 超速偏移',                 frame:'1021_m2'},
];

// LSB-first 从 8 字节 data[] 里读 [start, start+len) bit，返回 uint
function dbgReadField(data, start, len) {
  let v = 0;
  for (let i = 0; i < len; i++) {
    const b = start + i;
    const bit = (data[b >> 3] >> (b & 7)) & 1;
    v |= (bit << i);
  }
  return v >>> 0;
}

function dbgMaskedStart(mask, start, len) {
  // 返回 [start, start+len) 内 mask 的各 bit；全 0 = 无覆盖，全 1 = 整段覆盖
  let full = true, any = false;
  for (let i = 0; i < len; i++) {
    const b = start + i;
    const bit = (mask[b >> 3] >> (b & 7)) & 1;
    if (bit) any = true; else full = false;
  }
  return {any, full};
}

function dbgFmtValue(v, sig) {
  if (sig.enums && sig.enums[v] !== undefined) return v + ' · ' + sig.enums[v];
  return String(v);
}

function dbgBuildCtrl(group, sig, curVal, overrideOn) {
  // 根据字段类型返回编辑控件 HTML（不含 override checkbox，那个在外层）
  const id = 'dbg_'+group+'_'+sig.start;
  if (sig.len === 1) {
    // 开关
    const chk = curVal ? 'checked' : '';
    return '<label class="sw"><input type="checkbox" id="'+id+'" '+chk+
           ' onchange="dbgOnEdit(\''+group+'\','+sig.start+','+sig.len+',this.checked?1:0)">' +
           '<span class="sl"></span></label>';
  }
  if (sig.enums) {
    let opts = '';
    // 生成 0..2^len-1 的选项；只给 enums 表里有的值加标签
    const maxv = (1 << sig.len) - 1;
    for (let v = 0; v <= maxv; v++) {
      const sel = (v === curVal) ? 'selected' : '';
      const lbl = sig.enums[v] !== undefined ? (v + ' · ' + sig.enums[v]) : String(v);
      opts += '<option value="'+v+'" '+sel+'>'+lbl+'</option>';
    }
    return '<select id="'+id+'" class="inp" onchange="dbgOnEdit(\''+group+'\','+sig.start+','+sig.len+',+this.value)">'+opts+'</select>';
  }
  // 数值
  const max = (1 << sig.len) - 1;
  return '<input type="number" id="'+id+'" class="inp" min="0" max="'+max+'" value="'+curVal+'"'+
         ' onchange="dbgOnEdit(\''+group+'\','+sig.start+','+sig.len+',+this.value)">';
}

function dbgRenderGroup(grpCfg, snap) {
  const sigs = DBG_SIGS[grpCfg.key];
  const seen = snap.seen;
  const data = snap.data;
  const mask = snap.mask;
  const valArr = snap.val;
  let html = '';
  sigs.forEach(sig => {
    const curVal = seen ? dbgReadField(data, sig.start, sig.len) : 0;
    const ovrVal = dbgReadField(valArr, sig.start, sig.len);
    const m = dbgMaskedStart(mask, sig.start, sig.len);
    const isOvr = m.full;  // 只有整段都被 mask 才算"此信号被覆盖"
    const shownVal = isOvr ? ovrVal : curVal;
    const curTxt  = seen ? dbgFmtValue(curVal, sig) : '--';
    const ctrl    = dbgBuildCtrl(grpCfg.key, sig, shownVal, isOvr);
    const ovrChk  = isOvr ? 'checked' : '';
    const rowCls  = isOvr ? 'dbg-row ovr' : 'dbg-row';
    html +=
      '<div class="'+rowCls+'">' +
        '<input type="checkbox" class="dbg-chk" '+ovrChk+
              ' onchange="dbgOnOverride(\''+grpCfg.key+'\','+sig.start+','+sig.len+',this.checked)">' +
        '<div class="dbg-meta">' +
          '<div class="dbg-name">'+sig.name+'<span class="dbg-pos">b'+sig.start+'·'+sig.len+'</span></div>' +
          '<div class="dbg-desc">'+sig.desc+'</div>' +
          '<div class="dbg-now">当前值：'+curTxt+'</div>' +
        '</div>' +
        '<div class="dbg-ctrl">'+ctrl+'</div>' +
      '</div>';
  });
  return html;
}

let _dbgOpen = {f1016:false, f1021_m0:true, f1021_m1:false, f1021_m2:false};
let _dbgSnap = null;
let _dbgEditingId = null; // 正在聚焦编辑的 input id，刷新时跳过，避免跳字

function dbgBuildPage() {
  const root = g('dbg_root');
  if (root.childElementCount) return; // 已建
  DBG_GROUPS.forEach(g0 => {
    const openCls = _dbgOpen[g0.key] ? ' open' : '';
    root.insertAdjacentHTML('beforeend',
      '<div class="dbg-grp" id="grp_'+g0.key+'">' +
        '<div class="dbg-grp-hdr'+openCls+'" onclick="dbgToggle(\''+g0.key+'\')">' +
          '<div><span class="dbg-grp-title">'+g0.title+'</span><span class="dbg-grp-sub">'+g0.sub+'</span></div>' +
          '<span class="adv-chevron'+(_dbgOpen[g0.key]?' open':'')+'" id="chv_'+g0.key+'">&#9660;</span>' +
        '</div>' +
        '<div class="dbg-grp-body'+openCls+'" id="body_'+g0.key+'"></div>' +
      '</div>');
  });
}

function dbgToggle(key) {
  _dbgOpen[key] = !_dbgOpen[key];
  g('body_'+key).classList.toggle('open', _dbgOpen[key]);
  const hdr = g('body_'+key).previousElementSibling; if (hdr) hdr.classList.toggle('open', _dbgOpen[key]);
  g('chv_'+key).classList.toggle('open', _dbgOpen[key]);
  if (_dbgOpen[key] && _dbgSnap) dbgRefreshBody(key);
}

function dbgRefreshBody(key) {
  if (!_dbgSnap) return;
  const snap = _dbgSnap[key];
  if (!snap) return;
  const body = g('body_'+key);
  // 记录当前焦点
  const ae = document.activeElement;
  const focusedId = (ae && ae.id && ae.id.startsWith('dbg_')) ? ae.id : null;
  // select 下拉框打开中或 number 输入中，跳过刷新避免打断用户操作
  if (focusedId && (ae.tagName === 'SELECT' || ae.type === 'number')) return;
  body.innerHTML = dbgRenderGroup(DBG_GROUPS.find(x=>x.key===key), snap);
  if (focusedId) { const el = g(focusedId); if (el) el.focus(); }
}

async function dbgPoll() {
  // 只在当前是调试页时拉，减少带宽
  if (!g('pg_debug').classList.contains('act')) return;
  try {
    const d = await (await fetch('/debug_status')).json();
    _dbgSnap = d;
    dbgBuildPage();
    DBG_GROUPS.forEach(g0 => { if (_dbgOpen[g0.key]) dbgRefreshBody(g0.key); });
  } catch(e) {}
}

async function dbgOnOverride(key, start, len, on) {
  const frame = DBG_GROUPS.find(x=>x.key===key).frame;
  // 覆盖开启时把"当前值"作为初始写入值；关闭时清除
  let value = 0;
  if (on && _dbgSnap && _dbgSnap[key] && _dbgSnap[key].seen) {
    value = dbgReadField(_dbgSnap[key].data, start, len);
  }
  const p = new URLSearchParams({frame, start:String(start), length:String(len),
                                  value:String(value), override: on ? '1' : '0'});
  try { await fetch('/debug_set', {method:'POST', body:p}); } catch(e) {}
  dbgPoll();
}

const _dbgEditDebounce = {};
async function dbgOnEdit(key, start, len, value) {
  const frame = DBG_GROUPS.find(x=>x.key===key).frame;
  const dkey = key+'_'+start;
  clearTimeout(_dbgEditDebounce[dkey]);
  _dbgEditDebounce[dkey] = setTimeout(async () => {
    // 编辑即隐式开启覆盖
    const p = new URLSearchParams({frame, start:String(start), length:String(len),
                                    value:String(value>>>0), override:'1'});
    try { await fetch('/debug_set', {method:'POST', body:p}); } catch(e) {}
    dbgPoll();
  }, 250);
}

async function dbgClearAll() {
  if (!confirm('确认清除全部调试覆盖？同时会删除 NVS 中的存档。')) return;
  const p = new URLSearchParams({frame:'all'});
  try { await fetch('/debug_clear', {method:'POST', body:p}); } catch(e) {}
  dbgPoll();
}

async function dbgSaveArchive() {
  try {
    const r = await fetch('/debug_archive_save', {method:'POST'});
    if (!r.ok) throw 0;
    alert('存档已保存，重启设备后将自动恢复当前覆盖。');
  } catch(e) {
    alert('保存失败，请检查连接。');
  }
}

loadSchema(); poll(); loadAp(); loadSta(); loadBle();
setInterval(poll, 2000);
setInterval(loadAp, 15000);
setInterval(loadSta, 6000);
setInterval(loadBle, 2000);
setInterval(dbgPoll, 1500);
</script>
</body>
</html>)HTML";
