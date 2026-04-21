#pragma once
#include <Arduino.h>

    static const char DASH_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
    <html lang="en" data-theme="dark">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
        <title>ev-open-can-tools</title>
        <style>
            * {
                margin: 0;
                padding: 0;
                box-sizing: border-box
            }

            [data-theme="dark"] {
                --bg: #0d0d0d;
                --card: #161616;
                --card2: #1e1e1e;
                --bd: #2a2a2a;
                --bd2: #333;
                --tx: #f0f0f0;
                --tx2: #999;
                --tx3: #555;
                --acc: #5b8fff;
                --accBg: rgba(91, 143, 255, .1);
                --accBd: rgba(91, 143, 255, .25);
                --ok: #3dba72;
                --okBg: rgba(61, 186, 114, .1);
                --err: #ff4f4f;
                --errBg: rgba(255, 79, 79, .08);
                --errBd: rgba(255, 79, 79, .2);
                --warn: #f5a623;
            }

            [data-theme="light"] {
                --bg: #f5f5f5;
                --card: #fff;
                --card2: #f0f0f0;
                --bd: #e0e0e0;
                --bd2: #ccc;
                --tx: #111;
                --tx2: #555;
                --tx3: #999;
                --acc: #2563eb;
                --accBg: rgba(37, 99, 235, .08);
                --accBd: rgba(37, 99, 235, .2);
                --ok: #16a34a;
                --okBg: rgba(22, 163, 74, .08);
                --err: #dc2626;
                --errBg: rgba(220, 38, 38, .06);
                --errBd: rgba(220, 38, 38, .18);
                --warn: #d97706;
            }

            html {
                scroll-behavior: smooth
            }

            body {
                background: var(--bg);
                color: var(--tx);
                font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
                min-height: 100vh;
                max-width: 480px;
                margin: 0 auto;
                font-size: 14px;
                line-height: 1.5;
                transition: background .2s, color .2s
            }

            /* Header */
            .hdr {
                padding: 20px 16px 0;
                display: flex;
                flex-direction: column;
                gap: 4px
            }

            .hdr-top {
                display: flex;
                align-items: center;
                justify-content: space-between
            }

            .hdr-left {
                display: flex;
                align-items: center;
                gap: 10px
            }

            .hdr-title {
                font-size: 20px;
                font-weight: 700;
                color: var(--tx)
            }

            .hw-badge {
                padding: 3px 8px;
                border-radius: 5px;
                font-size: 11px;
                font-weight: 600;
                background: var(--accBg);
                border: 1px solid var(--accBd);
                color: var(--acc)
            }

            .theme-btn {
                padding: 6px 10px;
                border: 1px solid var(--bd2);
                border-radius: 8px;
                background: var(--card);
                color: var(--tx2);
                font-size: 12px;
                cursor: pointer;
                display: flex;
                align-items: center;
                gap: 4px;
                transition: all .2s
            }

            .theme-btn:hover {
                border-color: var(--acc);
                color: var(--acc)
            }

            .hdr-status {
                display: flex;
                align-items: center;
                gap: 6px;
                font-size: 12px;
                color: var(--tx2)
            }

            .sdot {
                width: 7px;
                height: 7px;
                border-radius: 50%;
                flex-shrink: 0;
                transition: all .4s
            }

            .dot-on {
                background: var(--ok);
                box-shadow: 0 0 8px var(--ok)
            }

            .dot-off {
                background: var(--err)
            }

            .dot-warn {
                background: var(--warn)
            }

            /* FPS bar */
            .fps-bar {
                margin: 14px 16px 0;
                height: 3px;
                background: var(--bd);
                border-radius: 2px;
                overflow: hidden
            }

            .fps-fill {
                height: 100%;
                background: var(--acc);
                border-radius: 2px;
                transition: width .5s;
                width: 0%
            }

            /* Status grid */
            .stat-grid {
                display: grid;
                grid-template-columns:1fr 1fr 1fr;
                gap: 8px;
                margin: 14px 16px 0
            }

            .stat {
                background: var(--card);
                border: 1px solid var(--bd);
                border-radius: 10px;
                padding: 10px 12px
            }

            .stat-lbl {
                font-size: 10px;
                color: var(--tx3);
                text-transform: uppercase;
                letter-spacing: .8px;
                margin-bottom: 3px
            }

            .stat-val {
                font-size: 14px;
                font-weight: 600;
                color: var(--tx)
            }

            .v-ok {
                color: var(--ok)
            }

            .v-err {
                color: var(--err)
            }

            .v-acc {
                color: var(--acc)
            }

            .v-dim {
                color: var(--tx3)
            }

            .v-warn {
                color: var(--warn)
            }

            .stat-wide {
                grid-column: span 3
            }

            /* Divider */
            hr {
                border: none;
                border-top: 1px solid var(--bd);
                margin: 16px
            }

            /* Cards */
            .card {
                background: var(--card);
                border: 1px solid var(--bd);
                border-radius: 12px;
                padding: 16px;
                margin: 0 16px 12px
            }

            .card-hdr {
                display: flex;
                align-items: center;
                justify-content: space-between;
                margin-bottom: 14px
            }

            .card-title {
                font-size: 13px;
                font-weight: 600;
                color: var(--tx);
                text-transform: uppercase;
                letter-spacing: .5px
            }

            .card-meta {
                font-size: 11px;
                color: var(--tx3)
            }

            .card.collapsible > .card-hdr {
                cursor: pointer;
                user-select: none;
                margin-bottom: 0;
            }

            .card.collapsible.expanded > .card-hdr {
                margin-bottom: 14px;
            }

            .card.collapsible > .card-hdr .card-title::after {
                content: ' ▾';
                font-size: 11px;
                color: var(--tx3);
                display: inline-block;
                transition: transform .2s;
                vertical-align: middle;
            }

            .card.collapsible:not(.expanded) > .card-hdr .card-title::after {
                transform: rotate(-90deg);
            }

            .card.collapsible:not(.expanded) > *:not(.card-hdr) {
                display: none !important;
            }

            /* HW seg */
            .hw-seg {
                display: flex;
                background: var(--card2);
                border: 1px solid var(--bd);
                border-radius: 9px;
                padding: 3px;
                gap: 2px
            }

            .hw-btn {
                flex: 1;
                padding: 8px;
                border: none;
                border-radius: 7px;
                font-size: 12px;
                font-weight: 600;
                cursor: pointer;
                background: transparent;
                color: var(--tx2);
                transition: all .18s;
                font-family: inherit
            }

            .hw-btn.active {
                background: var(--card);
                color: var(--acc);
                border: 1px solid var(--accBd);
                box-shadow: 0 1px 4px rgba(0, 0, 0, .15)
            }

            .hw-btn:hover:not(.active) {
                background: var(--bd);
                color: var(--tx)
            }

            /* Speed pills */
            .pills {
                display: flex;
                gap: 6px;
                flex-wrap: wrap
            }

            .pill {
                flex: 1;
                min-width: 60px;
                padding: 9px 8px;
                border: 1px solid var(--bd);
                border-radius: 9px;
                font-size: 12px;
                font-weight: 600;
                cursor: pointer;
                background: var(--bg);
                color: var(--tx2);
                transition: all .18s;
                text-align: center;
                font-family: inherit
            }

            .pill.active {
                background: var(--accBg);
                border-color: var(--accBd);
                color: var(--acc)
            }

            .pill:hover:not(.active) {
                border-color: var(--bd2);
                color: var(--tx)
            }

            /* Feature rows */
            .feat-row {
                display: flex;
                align-items: center;
                justify-content: space-between;
                padding: 12px 0;
                border-bottom: 1px solid var(--bd)
            }

            .feat-row:last-of-type {
                border-bottom: none;
                padding-bottom: 0
            }

            .feat-row:first-of-type {
                padding-top: 0
            }

            .feat-info {
                flex: 1;
                min-width: 0
            }

            .feat-name {
                font-size: 13px;
                font-weight: 500;
                color: var(--tx)
            }

            .feat-desc {
                font-size: 11px;
                color: var(--tx3);
                margin-top: 2px
            }

            /*.hw4-only.hidden {*/
            /*    display: none*/
            /*}*/

            /* Toggle */
            .tgl {
                position: relative;
                width: 44px;
                height: 24px;
                flex-shrink: 0;
                margin-left: 12px
            }

            .tgl input {
                opacity: 0;
                width: 0;
                height: 0;
                position: absolute
            }

            .tgl-track {
                position: absolute;
                inset: 0;
                background: var(--bd2);
                border-radius: 24px;
                cursor: pointer;
                transition: all .22s
            }

            .tgl-thumb {
                position: absolute;
                top: 3px;
                left: 3px;
                width: 18px;
                height: 18px;
                background: #fff;
                border-radius: 50%;
                transition: all .22s;
                box-shadow: 0 1px 3px rgba(0, 0, 0, .3)
            }

            .tgl input:checked ~ .tgl-track {
                background: var(--acc)
            }

            .tgl input:checked ~ .tgl-track .tgl-thumb {
                transform: translateX(20px)
            }

            .tgl input:disabled ~ .tgl-track {
                opacity: .35;
                cursor: not-allowed
            }

            /* Sniffer */
            .sniff-ctrl {
                display: flex;
                gap: 6px;
                margin-bottom: 8px
            }

            .sniff-input {
                flex: 1;
                background: var(--bg);
                border: 1px solid var(--bd);
                border-radius: 8px;
                padding: 7px 10px;
                color: var(--tx);
                font-size: 12px;
                font-family: inherit;
                transition: border .2s
            }

            .sniff-input {
                width: 100%;
                min-width: 0;
                box-sizing: border-box;
            }

            .sniff-input:focus {
                outline: none;
                border-color: var(--acc)
            }

            .sniff-input::placeholder {
                color: var(--tx3)
            }

            .h4o-row {
                display: grid;
                grid-template-columns: 68px minmax(0, 1fr) 48px 14px;
                align-items: center;
                gap: 10px;
                padding: 5px 10px;
                border: 1px solid var(--bd);
                border-radius: 10px;
                background: var(--bg);
                font-size: 12px;
                color: var(--tx2);
            }

            .h4o-kmh {
                font-size: 12px;
                font-weight: 600;
                color: var(--tx);
                line-height: 1;
                white-space: nowrap;
                text-align: left;
            }

            .h4o-unit {
                font-size: 12px;
                font-weight: 600;
                color: var(--tx2);
                white-space: nowrap;
                text-align: left;
            }

            .h4o-custom {
                display: flex;
                flex-direction: column;
                gap: 8px;
            }

            .h4o-inp {
                height: 30px;
                background: var(--card);
                border: 1px solid var(--bd);
                border-radius: 8px;
                padding: 0 6px;
                color: var(--tx);
                font-size: 13px;
                font-weight: 600;
                font-family: inherit;
                text-align: center;
                width: 100%;
                box-sizing: border-box;
            }

            .h4o-inp:focus {
                outline: none;
                border-color: var(--acc);
            }

            .h4o-inp::-webkit-outer-spin-button,
            .h4o-inp::-webkit-inner-spin-button {
                -webkit-appearance: none;
                margin: 0;
            }

            .h4o-inp {
                -moz-appearance: textfield;
            }

            .h4o-bar {
                -webkit-appearance: none;
                appearance: none;
                width: 100%;
                height: 6px;
                background: var(--bd);
                border-radius: 999px;
                outline: none;
                cursor: pointer;
            }

            .h4o-bar::-webkit-slider-thumb {
                -webkit-appearance: none;
                appearance: none;
                width: 18px;
                height: 18px;
                border-radius: 50%;
                background: var(--acc);
                border: 2px solid var(--card);
                box-shadow: 0 1px 4px rgba(0, 0, 0, .25);
                cursor: pointer;
                margin-top: -6px;
            }

            .h4o-bar::-moz-range-thumb {
                width: 18px;
                height: 18px;
                border-radius: 50%;
                background: var(--acc);
                border: 2px solid var(--card);
                box-shadow: 0 1px 4px rgba(0, 0, 0, .25);
                cursor: pointer;
            }

            .h4o-bar::-moz-range-track {
                height: 6px;
                background: var(--bd);
                border-radius: 999px;
                border: none;
            }

            .sniff-btn {
                padding: 7px 12px;
                background: transparent;
                border: 1px solid var(--bd);
                border-radius: 8px;
                color: var(--tx2);
                font-size: 11px;
                font-weight: 600;
                cursor: pointer;
                transition: all .18s;
                font-family: inherit
            }

            .sniff-btn.paused {
                border-color: var(--warn);
                color: var(--warn)
            }

            .sniff-btn:hover:not(.paused) {
                border-color: var(--bd2);
                color: var(--tx)
            }

            .sniff-box {
                background: var(--bg);
                border: 1px solid var(--bd);
                border-radius: 9px;
                max-height: 250px;
                overflow-y: auto;
                font-family: 'SF Mono', 'Courier New', monospace
            }

            .sniff-box::-webkit-scrollbar {
                width: 4px
            }

            .sniff-box::-webkit-scrollbar-thumb {
                background: var(--bd2);
                border-radius: 4px
            }

            .sniff-row {
                display: grid;
                grid-template-columns:38px 72px 1fr;
                gap: 8px;
                padding: 6px 10px;
                border-bottom: 1px solid var(--bd);
                font-size: 11px;
                align-items: start
            }

            .sniff-row:last-child {
                border-bottom: none
            }

            .sniff-row.hi {
                border-left: 2px solid var(--acc);
                padding-left: 8px
            }

            .s-ts {
                color: var(--tx3);
                font-size: 10px;
                padding-top: 1px
            }

            .s-id {
                color: var(--acc);
                font-weight: 700
            }

            .s-data {
                color: var(--tx2);
                word-break: break-all
            }

            .s-name {
                color: var(--ok);
                font-size: 10px;
                margin-top: 2px
            }

            /* EFLG */
            .eflg-row {
                display: flex;
                flex-wrap: wrap;
                gap: 5px;
                margin-top: 10px
            }

            .eflg-pill {
                padding: 3px 8px;
                border-radius: 5px;
                font-size: 10px;
                font-weight: 600;
                letter-spacing: .3px
            }

            .eflg-ok {
                background: var(--okBg);
                color: var(--ok)
            }

            .eflg-warn {
                background: rgba(245, 166, 35, .1);
                color: var(--warn)
            }

            .eflg-err {
                background: var(--errBg);
                color: var(--err)
            }

            /* Mux table */
            .mux-tbl {
                width: 100%;
                border-collapse: collapse;
                font-size: 12px;
                margin-top: 10px
            }

            .mux-tbl th {
                color: var(--tx3);
                font-size: 10px;
                text-transform: uppercase;
                letter-spacing: .8px;
                text-align: left;
                padding: 4px 8px;
                border-bottom: 1px solid var(--bd);
                font-weight: 500
            }

            .mux-tbl td {
                padding: 5px 8px;
                color: var(--tx2);
                border-bottom: 1px solid var(--bd)
            }

            .mux-tbl tr:last-child td {
                border-bottom: none
            }

            .mux-tbl td:first-child {
                color: var(--acc);
                font-weight: 600
            }

            /* Buttons */
            .btn-row {
                display: flex;
                gap: 8px;
                margin-top: 14px
            }

            .btn {
                flex: 1;
                padding: 10px;
                border: 1px solid;
                border-radius: 9px;
                background: transparent;
                font-family: inherit;
                font-size: 12px;
                font-weight: 600;
                cursor: pointer;
                transition: all .18s;
                letter-spacing: .3px
            }

            .btn-stop {
                border-color: var(--errBd);
                color: var(--err)
            }

            .btn-stop:hover {
                background: var(--errBg)
            }

            .btn-reboot {
                border-color: var(--bd2);
                color: var(--tx2)
            }

            .btn-reboot:hover {
                border-color: var(--acc);
                color: var(--acc)
            }

            /* Confirm modal */
            .modal-backdrop {
                position: fixed;
                inset: 0;
                display: none;
                align-items: center;
                justify-content: center;
                padding: 16px;
                background: rgba(0, 0, 0, .55);
                z-index: 9999
            }

            .modal-card {
                width: min(100%, 360px);
                background: var(--card);
                border: 1px solid var(--bd2);
                border-radius: 12px;
                padding: 16px;
                box-shadow: 0 16px 40px rgba(0, 0, 0, .35)
            }

            .modal-title {
                font-size: 14px;
                font-weight: 700;
                color: var(--tx)
            }

            .modal-msg {
                margin-top: 8px;
                font-size: 12px;
                color: var(--tx2);
                line-height: 1.6;
                white-space: pre-wrap
            }

            .modal-actions {
                display: flex;
                justify-content: flex-end;
                gap: 8px;
                margin-top: 14px
            }

            .modal-btn-primary {
                background: var(--accBg);
                border-color: var(--accBd);
                color: var(--acc)
            }

            .modal-btn-primary:hover {
                background: var(--acc);
                color: #fff
            }

            /* OTA upload */
            .ota-drop {
                border: 2px dashed var(--bd2);
                border-radius: 10px;
                padding: 24px 16px;
                text-align: center;
                cursor: pointer;
                transition: all .2s;
                background: var(--bg)
            }

            .ota-drop:hover, .ota-drop.drag {
                border-color: var(--acc);
                background: var(--accBg)
            }

            .ota-drop input {
                display: none
            }

            .ota-icon {
                font-size: 24px;
                margin-bottom: 8px
            }

            .ota-text {
                font-size: 13px;
                font-weight: 500;
                color: var(--tx2);
                margin-bottom: 3px
            }

            .ota-sub {
                font-size: 11px;
                color: var(--tx3)
            }

            .ota-progress {
                margin-top: 12px;
                display: none
            }

            .ota-bar {
                height: 4px;
                background: var(--bd);
                border-radius: 2px;
                overflow: hidden;
                margin-bottom: 6px
            }

            .ota-fill {
                height: 100%;
                background: var(--acc);
                border-radius: 2px;
                transition: width .3s;
                width: 0%
            }

            .ota-status {
                font-size: 11px;
                color: var(--acc);
                text-align: center
            }

            .ota-btn {
                width: 100%;
                margin-top: 10px;
                padding: 10px;
                border: 1px solid var(--accBd);
                border-radius: 9px;
                background: var(--accBg);
                color: var(--acc);
                font-family: inherit;
                font-size: 13px;
                font-weight: 600;
                cursor: pointer;
                transition: all .2s;
                display: none
            }

            .ota-btn:hover {
                background: var(--acc);
                color: #fff
            }

            /* Log */
            .log-box {
                background: var(--bg);
                border: 1px solid var(--bd);
                border-radius: 9px;
                padding: 10px 12px;
                font-family: 'SF Mono', 'Courier New', monospace;
                font-size: 11px;
                color: var(--tx2);
                max-height: 180px;
                overflow-y: auto;
                line-height: 1.9;
                white-space: pre-wrap;
                word-break: break-all
            }

            .log-box::-webkit-scrollbar {
                width: 4px
            }

            .log-box::-webkit-scrollbar-thumb {
                background: var(--bd2);
                border-radius: 4px
            }

            .lf {
                color: var(--ok)
            }

            .lh {
                color: var(--acc)
            }

            .le {
                color: var(--err)
            }

            .lc {
                color: var(--warn)
            }

            .lo {
                color: var(--tx2)
            }

            /* Recorder */
            .rec-bar {
                height: 4px;
                background: var(--bd);
                border-radius: 2px;
                overflow: hidden;
                margin-bottom: 6px
            }

            .rec-fill {
                height: 100%;
                background: var(--err);
                border-radius: 2px;
                transition: width .3s;
                width: 0%
            }

            .rec-info {
                display: flex;
                justify-content: space-between;
                font-size: 11px;
                color: var(--tx3);
                margin-bottom: 10px
            }

            .foot {
                text-align: center;
                padding: 8px 16px 20px;
                font-size: 11px;
                color: var(--tx3)
            }

            /* Tabs */
            .tab-bar {
                display: flex;
                gap: 0;
                padding: 0 16px;
                border-bottom: 1px solid var(--bd);
                background: var(--bg);
                position: sticky;
                top: 0;
                z-index: 100;
                margin-top: 10px;
            }

            .tab-btn {
                padding: 10px 14px;
                border: none;
                border-bottom: 2px solid transparent;
                background: transparent;
                color: var(--tx2);
                font-size: 12px;
                font-weight: 600;
                cursor: pointer;
                font-family: inherit;
                transition: color .18s, border-color .18s;
                margin-bottom: -1px;
                letter-spacing: .3px;
            }

            .tab-btn.active {
                color: var(--acc);
                border-bottom-color: var(--acc);
            }

            .tab-btn:hover:not(.active) {
                color: var(--tx);
            }

            .tab-pane {
                display: none;
            }

            .tab-pane.active {
                display: block;
            }
        </style>
    </head>
    <body>

    <div class="hdr">
        <div class="hdr-top">
            <div class="hdr-left">
                <div class="hdr-title">ev-open-can-tools</div>
                <span class="hw-badge" id="hw-badge">HW3</span>
            </div>
            <button class="theme-btn" onclick="toggleTheme()" id="theme-btn">&#9788; Light</button>
        </div>
        <div class="hdr-status">
            <span class="sdot dot-off" id="dot"></span>
            <span id="hdr-desc">Waiting for CAN frames</span>
        </div>
    </div>

    <div class="fps-bar">
        <div class="fps-fill" id="fps-fill"></div>
    </div>

    <!-- Tab 导航栏 -->
    <div class="tab-bar">
        <button class="tab-btn active" onclick="switchTab('status')">状态</button>
        <button class="tab-btn" onclick="switchTab('control')">控制</button>
        <button class="tab-btn" onclick="switchTab('debug')">调试</button>
        <button class="tab-btn" onclick="switchTab('system')">系统</button>
    </div>

    <!-- Tab 1: 状态 -->
    <div class="tab-pane active" id="tab-status">

    <div style="height:12px"></div>

    <div class="stat-grid">
        <div class="stat">
            <div class="stat-lbl">CAN Bus</div>
            <div class="stat-val" id="s-can">Offline</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Injection</div>
            <div class="stat-val v-dim" id="s-inj">—</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">AD</div>
            <div class="stat-val" id="s-AD">Inactive</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Frame rate</div>
            <div class="stat-val v-dim" id="s-fps">0.0 Hz</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">RX</div>
            <div class="stat-val v-acc" id="s-rx">0</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">TX</div>
            <div class="stat-val v-acc" id="s-tx">0</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">TX Errors</div>
            <div class="stat-val v-dim" id="s-txerr">0</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Follow dist</div>
            <div class="stat-val v-dim" id="s-fd">—</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Profile</div>
            <div class="stat-val v-dim" id="s-prof">—</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Speed Offset</div>
            <div class="stat-val v-dim" id="s-soff">0</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Uptime</div>
            <div class="stat-val v-dim" id="s-up">0s</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">BanShield</div>
            <div class="stat-val v-dim" id="s-bscnt">0</div>
        </div>
        <div class="stat">
            <div class="stat-lbl">Speed Limit</div>
            <div class="stat-val v-dim" id="s-splim">—</div>
        </div>

        <div class="stat">
            <div class="stat-lbl">Speed Limit(Vision)</div>
            <div class="stat-val v-dim" id="s-splimv">—</div>
        </div>
    </div>

    </div><!-- /tab-status -->

    <!-- Tab 2: 控制 -->
    <div class="tab-pane" id="tab-control">

    <div style="height:12px"></div>

    <div class="card" style="display:none">
<!--    <div class="card" >-->
        <div class="card-hdr">
            <div class="card-title">Hardware</div>
            <div class="card-meta">Autopilot generation</div>
        </div>
        <div class="hw-seg" id="hw-seg">
            <button class="hw-btn" data-v="0" onclick="setHW(0)">Legacy</button>
            <button class="hw-btn active" data-v="1" onclick="setHW(1)">HW3</button>
            <button class="hw-btn" data-v="2" onclick="setHW(2)">HW4</button>
        </div>
    </div>

    <div class="card">
        <div class="card-hdr">
            <div class="card-title">Speed Profile</div>
            <div class="card-meta">AD aggressiveness &bull; Auto follows stalk</div>
        </div>
        <div class="pills" id="sp-pills"></div>
    </div>

    <div class="card hw4-only" style="margin-top:12px;margin-bottom:12px">
        <div class="card-hdr">
            <div class="card-title">Speed Offset</div>
            <div class="card-meta">Static offset injected on mux 2</div>
        </div>

        <div class="hw-seg" style="margin-bottom:10px">
            <button class="hw-btn active" id="h4o-tab-preset" onclick="setH4OTab('preset',true)">Preset</button>
            <button class="hw-btn" id="h4o-tab-custom" onclick="setH4OTab('custom',true)">Custom</button>
        </div>

        <div id="h4o-pills" style="display:flex;flex-wrap:wrap;gap:8px"></div>
        <div id="h4o-custom" class="h4o-custom" style="display:none;margin-top:8px"></div>

        <button id="h4o-save-btn"
                onclick="pushH4OCustom()"
                style="display:none;margin-top:12px;width:100%;padding:10px;background:var(--acc);color:#fff;border:none;border-radius:10px;font-size:13px;font-weight:600;cursor:pointer;font-family:inherit">
            Save Custom Map
        </button>
    </div>

    <div class="card">
        <div class="card-hdr">
            <div class="card-title">Features</div>
        </div>

        <div class="feat-row">
            <div class="feat-info">
                <div class="feat-name">AD Activation</div>
                <div class="feat-desc">Requires active AD subscription</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-AD" checked onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row">
            <div class="feat-info">
                <div class="feat-name">Use HW3 Compatible</div>
                <div class="feat-desc">Use HW3 Enable/Disable</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-use-hw3" checked onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row">
            <div class="feat-info">
                <div class="feat-name">Nag Suppression</div>
                <div class="feat-desc">Remove hands-on-wheel warning (ECE R79)</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-nag" checked onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row">
            <div class="feat-info">
                <div class="feat-name">Summon EU Unlock</div>
                <div class="feat-desc">Remove Smart Summon distance restriction</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-summon" checked onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row" id="row-camera">
            <div class="feat-info">
                <div class="feat-name">Cabin Camera</div>
                <div class="feat-desc">Cabin Camera Enable/Disable</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-camera" checked="true" onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row" id="row-banShield">
            <div class="feat-info">
                <div class="feat-name">Ban Shield</div>
                <div class="feat-desc">Ban Shield Enable/Disable</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-banShield" checked="true" onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row hw4-only" id="row-evd">
            <div class="feat-info">
                <div class="feat-name">Emergency Vehicle Detection</div>
                <div class="feat-desc">Enable approaching EV detection</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-evd" onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="feat-row">
            <div class="feat-info">
                <div class="feat-name">Enable Logging</div>
                <div class="feat-desc">Toggle serial and dashboard log output</div>
            </div>
            <label class="tgl"><input type="checkbox" id="tgl-eprn" checked onchange="pushFeat()">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>

        <div class="btn-row">
            <button class="btn btn-stop" id="btn-stop" style="display:none" onclick="emergencyStop()">Stop Injecting
            </button>
            <button class="btn" id="btn-resume"
                    style="display:none;background:var(--accBg);color:var(--acc);border:1px solid var(--accBd)"
                    onclick="resumeInj()">Resume Injection
            </button>
        </div>
    </div>

    </div><!-- /tab-control -->

    <!-- Tab 3: 调试 -->
    <div class="tab-pane" id="tab-debug">

    <div style="height:12px"></div>

    <div class="card collapsible" id="card-dbg">
        <div class="card-hdr" onclick="toggleCard('card-dbg')">
            <div class="card-title">Debug Injection</div>
            <div class="card-meta" id="dbg-meta">0 rules</div>
        </div>
        <div style="margin-bottom:10px;font-size:12px;color:var(--tx3);line-height:1.5">
            Intercept CAN frames and send a modified copy with specific bits overridden.
Format: <b>CAN&nbsp;ID</b> (hex) &bull; <b>MUX</b> (-1&nbsp;=&nbsp;any) &bull; <b>Bit</b> (0-63, default&nbsp;0) &bull; <b>Value</b> (0/1). New rows are off by default.
        </div>

        <table style="width:100%;border-collapse:collapse;font-size:12px">
            <thead>
                <tr style="color:var(--tx3);border-bottom:1px solid var(--bd)">
                    <th style="text-align:left;padding:4px 6px;font-weight:normal">Name</th>
                    <th style="text-align:left;padding:4px 6px;font-weight:normal">CAN ID</th>
                    <th style="text-align:left;padding:4px 6px;font-weight:normal">MUX</th>
                    <th style="text-align:left;padding:4px 6px;font-weight:normal">Bit</th>
                    <th style="text-align:left;padding:4px 6px;font-weight:normal">Value</th>
                    <th style="text-align:center;padding:4px 6px;font-weight:normal">On</th>
                    <th style="padding:4px 2px"></th>
                </tr>
            </thead>
            <tbody id="dbg-rows"></tbody>
        </table>

        <div style="display:flex;gap:6px;margin-top:10px;flex-wrap:wrap;align-items:center">
            <button class="sniff-btn" onclick="dbgAddRow()">+ Add Rule</button>
            <button class="sniff-btn" id="dbg-active-btn" onclick="dbgToggleActive()">Inject</button>
            <span id="dbg-status" style="font-size:11px;color:var(--tx3)"></span>
        </div>

        <div style="margin-top:14px;border-top:1px solid var(--bd);padding-top:10px">
            <div style="font-size:11px;color:var(--tx3);margin-bottom:6px">Sent frames (last seen per CAN ID + MUX, click to expand)</div>
            <div id="dbg-log-entries" style="font-size:11px;font-family:monospace"></div>
        </div>
    </div>

    <div class="card collapsible" id="card-sniffer">
        <div class="card-hdr" onclick="toggleCard('card-sniffer')">
            <div class="card-title">CAN Sniffer</div>
            <div class="card-meta" id="sniff-count">0 frames</div>
        </div>
        <div class="sniff-ctrl">
            <input class="sniff-input" id="sniff-filter" placeholder="Filter by ID or name" oninput="renderSniffer()">
            <button class="sniff-btn" id="sniff-id-btn" onclick="toggleSniffIdMode()">Wire IDs</button>
            <button class="sniff-btn" id="sniff-pause-btn" onclick="togglePause()">Pause</button>
        </div>
        <div class="sniff-box" id="sniffer">
            <div style="padding:20px;color:var(--tx3);text-align:center;font-size:12px">Waiting for CAN frames</div>
        </div>
    </div>

    <div class="card collapsible" id="card-recorder">
        <div class="card-hdr" onclick="toggleCard('card-recorder')">
            <div class="card-title">CAN Recorder</div>
            <div class="card-meta" id="rec-meta">Idle</div>
        </div>
        <div class="rec-bar">
            <div class="rec-fill" id="rec-fill"></div>
        </div>
        <div class="rec-info">
            <span id="rec-count">0 / 2000 frames</span>
            <span id="rec-status">Ready</span>
        </div>
        <div class="btn-row">
            <button class="btn" id="rec-btn" onclick="toggleRec()">Start Recording</button>
            <a class="btn" id="rec-dl" href="/rec_download" download="can_recording.csv"
               style="display:none;text-align:center;text-decoration:none;padding:10px;border:1px solid var(--bd2);color:var(--tx2)">Download
                CSV</a>
        </div>
    </div>

    <div class="card collapsible" id="card-controller">
        <div class="card-hdr" onclick="toggleCard('card-controller')">
            <div class="card-title">CAN Controller</div>
            <div style="display:flex;align-items:center;gap:8px">
                <div class="card-meta" id="s-mcp-raw">MCP2515</div>
                <button onclick="resetStats()"
                        style="font-size:10px;padding:2px 8px;border:1px solid var(--bd2);border-radius:5px;background:transparent;color:var(--tx3);cursor:pointer;font-family:inherit">
                    Reset
                </button>
            </div>
        </div>
        <div class="eflg-row" id="eflg-row"><span class="eflg-pill eflg-ok">OK</span></div>
        <table class="mux-tbl">
            <tr>
                <th>Mux</th>
                <th>RX</th>
                <th>TX</th>
                <th>Errors</th>
            </tr>
            <tr>
                <td>0</td>
                <td id="m0rx">0</td>
                <td id="m0tx">0</td>
                <td id="m0err">0</td>
            </tr>
            <tr>
                <td>1</td>
                <td id="m1rx">0</td>
                <td id="m1tx">0</td>
                <td id="m1err">0</td>
            </tr>
            <tr>
                <td>2</td>
                <td id="m2rx">0</td>
                <td id="m2tx">0</td>
                <td id="m2err">0</td>
            </tr>
        </table>
    </div>

    </div><!-- /tab-debug -->

    <!-- Tab 4: 系统 -->
    <div class="tab-pane" id="tab-system">

    <div style="height:12px"></div>

    <div class="card collapsible" id="card-hotspot">
        <div class="card-hdr" onclick="toggleCard('card-hotspot')">
            <div class="card-title">WiFi Hotspot <span onclick="event.stopPropagation();toggleInfo('ap-info')"
                                                       style="color:var(--tx3);cursor:pointer;font-size:12px;margin-left:4px"
                                                       title="About WiFi storage">&#9432;</span></div>
            <div class="card-meta"><span id="ap-stored" style="margin-right:8px"></span><span
                    id="ap-clients">0 clients</span></div>
        </div>
        <div id="ap-info"
             style="display:none;margin-bottom:10px;padding:10px;background:var(--bg2);border:1px solid var(--bd);border-radius:6px;font-size:12px;color:var(--tx3);line-height:1.5">
            Stored in NVS (non-volatile storage). The SSID and password survive firmware updates and reboots. Only a full
            factory erase via USB clears them.
        </div>
        <div class="feat-desc" style="margin-bottom:8px">Change the WiFi hotspot name and password</div>
        <div style="display:flex;gap:6px;margin-bottom:6px">
            <input class="sniff-input" id="ap-ssid" placeholder="Hotspot Name" style="flex:1">
            <input class="sniff-input" id="ap-pass" placeholder="New Password (min 8)" type="password" style="flex:1">
        </div>
        <div class="feat-row" style="padding:8px 0">
            <div class="feat-info">
                <div class="feat-name">Hide SSID</div>
                <div class="feat-desc">Don't broadcast the hotspot name &mdash; clients must enter it manually</div>
            </div>
            <label class="tgl"><input type="checkbox" id="ap-hidden">
                <div class="tgl-track">
                    <div class="tgl-thumb"></div>
                </div>
            </label>
        </div>
        <div style="display:flex;gap:6px;align-items:center">
            <button class="sniff-btn" onclick="saveAP()">Save</button>
            <span style="font-size:11px;color:var(--tx3)" id="ap-status"></span>
        </div>
        <div style="font-size:10px;color:var(--tx3);margin-top:6px">Changes take effect after reboot. Leave password empty
            to keep current.
        </div>
    </div>

    <div class="card collapsible" id="card-wifi">
        <div class="card-hdr" onclick="toggleCard('card-wifi')">
            <div class="card-title">WiFi Internet <span id="wifi-stored"
                                                        style="font-size:11px;font-weight:normal;color:var(--tx3)"></span>
            </div>
            <div class="card-meta" id="wifi-status">Not configured</div>
        </div>
        <div class="feat-desc" style="margin-bottom:8px">Connect to your home WiFi. Required for firmware updates.
            Stored in NVS &mdash; survives firmware updates.
        </div>
        <div style="display:flex;gap:6px;margin-bottom:6px">
            <input class="sniff-input" id="wifi-ssid" placeholder="WiFi SSID" style="flex:1">
            <button class="sniff-btn" onclick="scanWifi()" id="scan-btn">Scan</button>
        </div>
        <div id="wifi-nets"
             style="display:none;margin-bottom:6px;max-height:140px;overflow-y:auto;border:1px solid var(--bd);border-radius:6px;background:var(--bg2)"></div>
        <div style="display:flex;gap:6px;margin-bottom:6px">
            <input class="sniff-input" id="wifi-pass" placeholder="Password" type="password" style="flex:1">
            <button class="sniff-btn" onclick="saveWifi()">Connect</button>
        </div>
        <details style="margin-top:4px">
            <summary style="font-size:11px;color:var(--acc);cursor:pointer;user-select:none">Static IP (optional)</summary>
            <div style="margin-top:6px">
                <label style="font-size:11px;color:var(--tx3);display:flex;align-items:center;gap:6px;margin-bottom:6px">
                    <input type="checkbox" id="wifi-static" onchange="toggleStaticIP()"> Use static IP
                </label>
                <div id="static-fields" style="display:none">
                    <div style="display:grid;grid-template-columns:1fr 1fr;gap:4px">
                        <input class="sniff-input" id="wifi-ip" placeholder="IP (e.g. 192.168.1.100)">
                        <input class="sniff-input" id="wifi-gw" placeholder="Gateway (e.g. 192.168.1.1)">
                        <input class="sniff-input" id="wifi-mask" placeholder="Mask (255.255.255.0)" value="255.255.255.0">
                        <input class="sniff-input" id="wifi-dns" placeholder="DNS (e.g. 8.8.8.8)">
                    </div>
                </div>
            </div>
        </details>
    </div>

    <div class="card collapsible" id="card-firmware">
        <div class="card-hdr" onclick="toggleCard('card-firmware')">
            <div class="card-title">Firmware Update</div>
            <div class="card-meta" id="fw-ver"></div>
        </div>
        <div style="margin-bottom:10px">
            <div class="feat-row">
                <div class="feat-info">
                    <div class="feat-name">Beta Channel</div>
                    <div class="feat-desc">Include pre-release / beta firmware versions</div>
                </div>
                <label class="tgl"><input type="checkbox" id="beta-tgl" onchange="toggleBeta()">
                    <div class="tgl-track">
                        <div class="tgl-thumb"></div>
                    </div>
                </label>
            </div>
            <div class="feat-row">
                <div class="feat-info">
                    <div class="feat-name">Auto-Update on Boot</div>
                    <div class="feat-desc">Check and install updates automatically ~15 s after WiFi connects</div>
                </div>
                <label class="tgl"><input type="checkbox" id="auto-upd-tgl" onchange="toggleAutoUpdate()">
                    <div class="tgl-track">
                        <div class="tgl-thumb"></div>
                    </div>
                </label>
            </div>
        </div>
        <div style="display:flex;gap:6px;align-items:center">
            <button class="sniff-btn" onclick="checkUpdate()" id="upd-check-btn">Check for Updates</button>
            <span style="font-size:11px;color:var(--tx3)" id="upd-status"></span>
        </div>
        <div id="upd-info" style="display:none;margin-top:10px;padding:10px;background:var(--bg2);border-radius:6px">
            <div style="display:flex;justify-content:space-between;align-items:center">
                <div>
                    <div style="font-size:13px;font-weight:600" id="upd-ver"></div>
                    <div style="font-size:11px;color:var(--tx3)" id="upd-detail"></div>
                </div>
                <button class="sniff-btn" onclick="installUpdate()" id="upd-install-btn"
                        style="background:var(--ok);color:#fff;border-color:var(--ok)">Install
                </button>
            </div>
        </div>

        <details style="margin-top:14px;padding-top:12px;border-top:1px solid var(--bd)">
            <summary style="font-size:12px;color:var(--acc);cursor:pointer;user-select:none">Manual firmware upload (.bin)
            </summary>
            <div style="margin-top:10px">
                <div class="ota-drop" id="ota-drop" onclick="$('ota-file').click()"
                     ondragover="event.preventDefault();this.classList.add('drag')"
                     ondragleave="this.classList.remove('drag')" ondrop="handleDrop(event)">
                    <input type="file" id="ota-file" accept=".bin" onchange="fileSelected(this.files[0])">
                    <div class="ota-icon">&#8679;</div>
                    <div class="ota-text">Tap to select firmware .bin</div>
                    <div class="ota-sub">Or drag and drop a file here</div>
                </div>
                <div class="ota-progress" id="ota-progress">
                    <div class="ota-bar">
                        <div class="ota-fill" id="ota-fill"></div>
                    </div>
                    <div class="ota-status" id="ota-status">Uploading...</div>
                </div>
                <button class="ota-btn" id="ota-upload-btn" onclick="uploadFirmware()">Flash Firmware</button>
                <div style="margin-top:10px;font-size:11px;color:var(--tx3);line-height:1.7">
                    Build your .bin in PlatformIO: <span
                        style="color:var(--acc);font-family:monospace">Ctrl+Alt+B</span><br>
                    File is at: <span style="color:var(--acc);font-family:monospace">.pio/build/esp32_ext_mcp2515/firmware.bin</span>
                </div>
            </div>
        </details>
    </div>

    <div class="card collapsible" id="card-pins">
        <div class="card-hdr" onclick="toggleCard('card-pins')">
            <div class="card-title">CAN Pins <span onclick="event.stopPropagation();toggleInfo('can-pins-info')"
                                                   style="color:var(--tx3);cursor:pointer;font-size:12px;margin-left:4px"
                                                   title="About CAN pins">&#9432;</span></div>
            <div class="card-meta" id="can-pins-status">default</div>
        </div>
        <div id="can-pins-info"
             style="display:none;margin-bottom:10px;padding:10px;background:var(--bg2);border:1px solid var(--bd);border-radius:6px;font-size:12px;color:var(--tx3);line-height:1.5">
            GPIO pins for the CAN transceiver (TWAI). Persisted in NVS so they survive OTA updates. Leave empty to use the
            firmware&#39;s compile-time defaults. <b>Wrong pins disable CAN</b> &mdash; recovery needs a USB re-flash. On
            most ESP32 boards GPIO 6&ndash;11 remain reserved for SPI flash.
        </div>
        <div style="display:flex;gap:6px;align-items:center">
            <input class="sniff-input" id="can-tx" type="number" min="0" max="39" placeholder="TX GPIO" style="flex:1">
            <input class="sniff-input" id="can-rx" type="number" min="0" max="39" placeholder="RX GPIO" style="flex:1">
            <button class="sniff-btn" onclick="saveCanPins()">Save</button>
        </div>
        <div style="font-size:11px;color:var(--tx3);margin-top:6px" id="can-pins-hint">Reboot required after change</div>
    </div>

    <div class="card collapsible" id="card-backup">
        <div class="card-hdr" onclick="toggleCard('card-backup')">
            <div class="card-title">Settings Backup <span onclick="event.stopPropagation();toggleInfo('backup-info')"
                                                          style="color:var(--tx3);cursor:pointer;font-size:12px;margin-left:4px"
                                                          title="About backup">&#9432;</span></div>
            <div class="card-meta" id="backup-status"></div>
        </div>
        <div id="backup-info"
             style="display:none;margin-bottom:10px;padding:10px;background:var(--bg2);border:1px solid var(--bd);border-radius:6px;font-size:12px;color:var(--tx3);line-height:1.5">
            Exports AP credentials, WiFi Internet, CAN pins and beta channel as JSON. Useful before a full re-flash or when
            migrating to another device. <b>Passwords are included in clear text</b> &mdash; keep the file safe.
        </div>
        <div style="display:flex;gap:6px">
            <button class="sniff-btn" onclick="exportSettings()">Download</button>
            <button class="sniff-btn" onclick="document.getElementById('backup-file').click()">Upload &amp; Restore</button>
            <input type="file" id="backup-file" accept=".json,application/json" style="display:none"
                   onchange="importSettings(event)">
        </div>
    </div>

    <div class="card collapsible" id="card-log">
        <div class="card-hdr" onclick="toggleCard('card-log')">
            <div class="card-title">Live Log</div>
        </div>
    <div class="log-box" id="log">Waiting...</div>
    </div>

    <div class="card" id="card-reboot">
        <div class="feat-row" style="padding:4px 0">
            <div class="feat-info">
                <div class="feat-name">Reboot Device</div>
                <div class="feat-desc">Restart the ESP32. All active injections will stop until re-enabled.</div>
            </div>
            <button class="btn btn-reboot" onclick="reboot()">Reboot</button>
        </div>
    </div>

    </div><!-- /tab-system -->

    <div class="modal-backdrop" id="confirm-modal" onclick="dashConfirmBackdrop(event)">
        <div class="modal-card" role="dialog" aria-modal="true" aria-labelledby="confirm-title">
            <div class="modal-title" id="confirm-title">Confirm</div>
            <div class="modal-msg" id="confirm-msg"></div>
            <div class="modal-actions">
                <button class="sniff-btn" id="confirm-cancel" onclick="dashConfirmResolve(false)">Cancel</button>
                <button class="sniff-btn modal-btn-primary" id="confirm-ok" onclick="dashConfirmResolve(true)">Continue
                </button>
            </div>
        </div>
    </div>

    <div class="foot" id="dash-foot">ev-open-can-tools &bull; loading...</div>
    <script>
        const HW = ['Legacy', 'HW3', 'HW4'];
        const SP3 = ['Chill', 'Normal', 'Hurry'];
        const SP4 = ['Chill', 'Normal', 'Hurry', 'Max', 'Sloth'];
        const $ = id => document.getElementById(id);

        function spNames() {
            return state.hw === 2 ? SP4 : SP3;
        }

        const H4O = [
            {l: '+0%', v: 0},
            {l: '+10%', v: 10},
            {l: '+20%', v: 20},
            {l: '+30%', v: 30},
            {l: '+40%', v: 40},
            {l: '+50%', v: 50},
            {l: '+60%', v: 60}
        ];

        let H4O_Custom = [
            {sl: 20, v: 60},
            {sl: 30, v: 60},
            {sl: 40, v: 50},
            {sl: 50, v: 40},
            {sl: 60, v: 33},
            {sl: 70, v: 12},
            {sl: 80, v: 11},
            {sl: 90, v: 10},
            {sl: 100, v: 10},
            {sl: 110, v: 9},
            {sl: 120, v: 8},
        ];

        let state = {hw: 1, sp: 1, can: true, h4o: 0, spl: false};
        let sniffPaused = false, sniffFrames = [];
        let sniffShowDbcIds = localStorage.getItem('sniffIdMode') === 'dbc';
        let otaFile = null;
        let otaUser = localStorage.getItem('otaU') || '', otaPass = localStorage.getItem('otaP') || '';
        let logSince = 0;
        let dashConfirmState = null;
        let dashboardPollTimers = [];
        let dashboardPollFailures = 0;
        let h4oCustomLoaded = false;
        let h4oTabLoaded = false;
        let dashboardPollStopped = false;
        let dashboardStaIp = '';
        const pollLocks = {};

        function stopDashboardPolling() {
            if (dashboardPollStopped) return;
            dashboardPollStopped = true;
            dashboardPollTimers.forEach(clearInterval);
            dashboardPollTimers = [];
            $('dot').className = 'sdot dot-off';
            $('hdr-desc').textContent = 'Dashboard disconnected';
            let msg = 'Connection to ' + location.hostname + ' lost. Reload after reconnecting.';
            if (dashboardStaIp && dashboardStaIp !== location.hostname) msg = 'Connection to ' + location.hostname + ' lost. Switch to your normal WiFi and open http://' + dashboardStaIp;
            $('wifi-status').textContent = msg;
            $('wifi-status').style.color = 'var(--err)';
        }

        function noteDashboardPoll(ok) {
            if (ok) {
                dashboardPollFailures = 0;
                return;
            }
            if (dashboardPollStopped) return;
            dashboardPollFailures++;
            if (dashboardPollFailures >= 3) stopDashboardPolling();
        }

        async function fetchPollJson(url, timeoutMs) {
            const ctrl = new AbortController();
            const timer = setTimeout(() => ctrl.abort(), timeoutMs || 2500);
            try {
                const r = await fetch(url, {signal: ctrl.signal});
                if (!r.ok) throw new Error('HTTP ' + r.status);
                const d = await r.json();
                noteDashboardPoll(true);
                return d;
            } catch (e) {
                noteDashboardPoll(false);
                throw e;
            } finally {
                clearTimeout(timer);
            }
        }

        async function runPoll(name, fn) {
            if (dashboardPollStopped || pollLocks[name]) return;
            pollLocks[name] = true;
            try {
                return await fn();
            } finally {
                pollLocks[name] = false;
            }
        }

        function waitMs(ms) {
            return new Promise(resolve => setTimeout(resolve, ms));
        }

        function actionErrorMessage(e, fallback) {
            if (!e) return fallback;
            if (e.name === 'AbortError' || e.name === 'SyntaxError' || e.message === 'Failed to fetch' || e.message === 'Empty response') return fallback;
            return e.message || fallback;
        }

        async function fetchJsonWithTimeout(url, options, timeoutMs) {
            const ctrl = new AbortController();
            const timer = setTimeout(() => ctrl.abort(), timeoutMs || 2500);
            try {
                const opts = Object.assign({}, options || {});
                opts.signal = ctrl.signal;
                const r = await fetch(url, opts);
                const text = await r.text();
                if (!text || !text.trim()) throw new Error(r.ok ? 'Empty response' : ('HTTP ' + r.status));
                const d = JSON.parse(text);
                if (!r.ok) throw new Error(d.error || ('HTTP ' + r.status));
                return d;
            } finally {
                clearTimeout(timer);
            }
        }

        function dashConfirmResolve(ok) {
            if (!dashConfirmState) return;
            const resolve = dashConfirmState.resolve;
            dashConfirmState = null;
            $('confirm-modal').style.display = 'none';
            document.body.style.overflow = '';
            resolve(!!ok);
        }

        function dashConfirmBackdrop(ev) {
            if (ev.target === $('confirm-modal')) dashConfirmResolve(false);
        }

        function dashConfirm(message, title, okText, cancelText) {
            if (dashConfirmState) dashConfirmResolve(false);
            return new Promise(resolve => {
                dashConfirmState = {resolve};
                $('confirm-title').textContent = title || 'Confirm';
                $('confirm-msg').textContent = message || '';
                $('confirm-ok').textContent = okText || 'Continue';
                $('confirm-cancel').textContent = cancelText || 'Cancel';
                $('confirm-modal').style.display = 'flex';
                document.body.style.overflow = 'hidden';
                setTimeout(() => {
                    $('confirm-ok').focus();
                }, 0);
            });
        }

        document.addEventListener('keydown', e => {
            if (e.key === 'Escape' && dashConfirmState) dashConfirmResolve(false);
        });

        function toggleTheme() {
            const html = document.documentElement;
            const isDark = html.getAttribute('data-theme') === 'dark';
            html.setAttribute('data-theme', isDark ? 'light' : 'dark');
            $('theme-btn').innerHTML = isDark ? '&#9790; Dark' : '&#9788; Light';
            localStorage.setItem('theme', isDark ? 'light' : 'dark');
        }

        (function () {
            const t = localStorage.getItem('theme') || 'dark';
            document.documentElement.setAttribute('data-theme', t);
            // will be updated after DOM ready
            window.addEventListener('DOMContentLoaded', () => {
                $('theme-btn').innerHTML = t === 'dark' ? '&#9788; Light' : '&#9790; Dark';
            });
        })();

        function buildPills() {
            const ns = spNames(), c = $('sp-pills');
            c.innerHTML = '';
            const auto = document.createElement('button');
            auto.className = 'pill' + (!state.spl ? ' active' : '');
            auto.textContent = 'Auto';
            auto.onclick = () => setSPL(false);
            c.appendChild(auto);
            ns.forEach((n, i) => {
                const b = document.createElement('button');
                b.className = 'pill' + (state.spl && i === state.sp ? ' active' : '');
                b.dataset.v = i;
                b.textContent = n;
                b.onclick = () => setSP(i);
                c.appendChild(b);
            });
            const hc = $('h4o-pills');
            if (!hc) return;
            hc.innerHTML = '';
            H4O.forEach(o => {
                const b = document.createElement('button');
                b.className = 'pill' + (o.v === state.h4o ? ' active' : '');
                b.textContent = o.l;
                b.onclick = () => {
                    state.h4o = o.v;
                    buildPills();
                    pushFeat();
                };
                hc.appendChild(b);
            });

            const hcCustom = $('h4o-custom');
            if (hcCustom && !hcCustom.children.length) {
                H4O_Custom.forEach((o, i) => {
                    const row = document.createElement('div');
                    row.className = 'h4o-row';

                    const kmh = document.createElement('span');
                    kmh.className = 'h4o-kmh';
                    kmh.textContent = o.sl + ' km/h';
                    kmh.tabIndex = -1;

                    const bar = document.createElement('input');
                    bar.type = 'range';
                    bar.className = 'h4o-bar';
                    bar.min = 0;
                    bar.max = 60;
                    bar.value = o.v;
                    bar.style.width = '100%';
                    bar.style.minWidth = '0';

                    const vInp = document.createElement('input');
                    vInp.type = 'number';
                    vInp.className = 'h4o-inp';
                    vInp.value = o.v;
                    vInp.min = 0;
                    vInp.max = 60;
                    vInp.style.width = '48px';
                    vInp.oninput = () => {
                        let val = parseInt(vInp.value) || 0;
                        if (val > 60) {
                            val = 60;
                            vInp.value = 60;
                        }
                        if (val < 0) {
                            val = 0;
                            vInp.value = 0;
                        }
                        H4O_Custom[i].v = val;
                        bar.value = val;
                    };

                    bar.oninput = () => {
                        const val = parseInt(bar.value) || 0;
                        H4O_Custom[i].v = val;
                        vInp.value = val;
                    };

                    const pct = document.createElement('span');
                    pct.className = 'h4o-unit';
                    pct.textContent = '%';

                    row.appendChild(kmh);
                    row.appendChild(bar);
                    row.appendChild(vInp);
                    row.appendChild(pct);
                    hcCustom.appendChild(row);
                });
            }
        }

        async function pushH4OCustom() {
            const isCustom = $('h4o-tab-custom') && $('h4o-tab-custom').classList.contains('active');
            let body = 'tab=' + (isCustom ? 1 : 0);
            H4O_Custom.forEach((o, i) => {
                body += '&sl' + i + '=' + o.sl + '&v' + i + '=' + o.v;
            });
            try { await fetch('/h4o_custom', {method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body}); } catch (e) {}
        }

        function setH4OTab(tab, save) {
            const isPreset = tab === 'preset';
            $('h4o-pills').style.display = isPreset ? 'flex' : 'none';
            $('h4o-custom').style.display = isPreset ? 'none' : 'flex';
            const saveBtn = $('h4o-save-btn');
            if (saveBtn) saveBtn.style.display = isPreset ? 'none' : '';
            const tp = $('h4o-tab-preset'), tc = $('h4o-tab-custom');
            if (tp) tp.classList.toggle('active', isPreset);
            if (tc) tc.classList.toggle('active', !isPreset);
            if (save) pushH4OCustom();
        }

        function updSeg(el, v, cls) {
            el.querySelectorAll('.' + cls).forEach(b => b.classList.toggle('active', parseInt(b.dataset.v) === v));
        }

        function setHW(v) {
            state.hw = v;
            updSeg($('hw-seg'), v, 'hw-btn');
            buildPills();
            updateSniffIdToggle();
            renderSniffer();
            pushCfg();
            var b = $('hw-badge');
            if (b) b.textContent = HW[v];
        }

        function setSP(v) {
            state.sp = v;
            state.spl = true;
            buildPills();
            pushCfg();
        }

        function setSPL(v) {
            state.spl = v;
            buildPills();
            pushCfg();
        }

        function updateInjectButtons(active) {
            $('btn-stop').style.display = active ? '' : 'none';
            $('btn-resume').style.display = active ? 'none' : '';
        }

        function sniffBusPrefix() {
            return state.hw === 0 ? 0x0800 : 0x1000;
        }

        function sniffBusLabel() {
            return state.hw === 0 ? 'PARTY' : 'CH';
        }

        function sniffWireId(id) {
            return id & 0x7FF;
        }

        function sniffDbcId(id) {
            return sniffWireId(id) | sniffBusPrefix();
        }

        function sniffDisplayId(id) {
            return sniffShowDbcIds ? sniffDbcId(id) : sniffWireId(id);
        }

        function updateSniffIdToggle() {
            const b = $('sniff-id-btn'), bus = sniffBusLabel();
            b.textContent = sniffShowDbcIds ? ('DBC ' + bus) : 'Wire IDs';
            b.title = sniffShowDbcIds ? ('Showing DBC JSON IDs with ' + bus + ' prefix') : ('Showing on-wire 11-bit CAN IDs');
            $('sniff-filter').placeholder = 'Filter by wire/DBC ID or name';
        }

        function toggleSniffIdMode() {
            sniffShowDbcIds = !sniffShowDbcIds;
            localStorage.setItem('sniffIdMode', sniffShowDbcIds ? 'dbc' : 'wire');
            updateSniffIdToggle();
            renderSniffer();
        }

        async function pushCfg() {
            const body = 'hw=' + state.hw + '&sp=' + state.sp + '&can=' + (state.can ? '1' : '0') + '&spl=' + (state.spl ? '1' : '0');
            try {
                await fetch('/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body
                });
            } catch (e) {
            }
        }

        async function pushFeat() {
            const body = 'AD=' + ($('tgl-AD').checked ? '1' : '0')
                + '&nag=' + ($('tgl-nag').checked ? '1' : '0')
                + '&usehw3=' + ($('tgl-use-hw3').checked ? '1' : '0')
                + '&summon=' + ($('tgl-summon').checked ? '1' : '0')
                + '&camera=' + ($('tgl-camera').checked ? '1' : '0')
                + '&banShield=' + ($('tgl-banShield').checked ? '1' : '0')
                + '&evd=' + ($('tgl-evd').checked ? '1' : '0')
                + '&eprn=' + ($('tgl-eprn').checked ? '1' : '0')
                + '&h4o=' + state.h4o;
            try {
                await fetch('/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body
                });
            }
            catch (e) {
            }
            poll();
        }

        async function emergencyStop() {
            if (!await dashConfirm('Stop injecting? This remains disabled after reboot until you press Resume Injection.', 'Stop injection', 'Stop')) return;
            try {
                await fetch('/disable', {method: 'POST'});
            } catch (e) {
            }
            poll();
        }

        async function resumeInj() {
            try {
                await fetch('/config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'hw=' + state.hw + '&sp=' + state.sp + '&can=1'
                });
            } catch (e) {
            }
            poll();
        }

        async function reboot() {
            if (!await dashConfirm('Reboot device?', 'Reboot', 'Reboot')) return;
            try {
                await fetch('/reboot', {method: 'POST'});
            } catch (e) {
            }
        }

        function fmtUp(s) {
            if (s < 60) return s + 's';
            if (s < 3600) return Math.floor(s / 60) + 'm ' + String(s % 60).padStart(2, '0') + 's';
            return Math.floor(s / 3600) + 'h ' + Math.floor((s % 3600) / 60) + 'm';
        }

        function toHex(n, p) {
            return n.toString(16).toUpperCase().padStart(p, '0')
        }

        function renderEflg(e) {
            const el = $('eflg-row');
            if (!e) {
                el.innerHTML = '<span class="eflg-pill eflg-ok">OK</span>';
                return;
            }
            let h = '';
            if (e & 0x20) h += '<span class="eflg-pill eflg-err">Bus-Off</span>';
            if (e & 0x10) h += '<span class="eflg-pill eflg-warn">TX Passive</span>';
            if (e & 0x08) h += '<span class="eflg-pill eflg-warn">RX Passive</span>';
            if (e & 0x04) h += '<span class="eflg-pill eflg-warn">TX Warn</span>';
            if (e & 0x02) h += '<span class="eflg-pill eflg-warn">RX Warn</span>';
            if (e & 0xC0) h += '<span class="eflg-pill eflg-err">RX Overflow</span>';
            el.innerHTML = h || '<span class="eflg-pill eflg-ok">OK</span>';
        }

        function togglePause() {
            sniffPaused = !sniffPaused;
            const b = $('sniff-pause-btn');
            b.textContent = sniffPaused ? 'Resume' : 'Pause';
            b.classList.toggle('paused', sniffPaused);
        }

        function renderSniffer() {
            updateSniffIdToggle();
            const filter = $('sniff-filter').value.trim().toLowerCase();
            const el = $('sniffer');
            let frames = sniffFrames;
            if (filter) {
                const fid = parseInt(filter);
                if (!isNaN(fid)) frames = frames.filter(f => sniffWireId(f.id) === fid || sniffDbcId(f.id) === fid);
                else frames = frames.filter(f => f.name && f.name.toLowerCase().includes(filter));
            }
            $('sniff-count').textContent = frames.length + ' frames';
            if (!frames.length) {
                el.innerHTML = '<div style="padding:20px;color:var(--tx3);text-align:center;font-size:12px">No frames</div>';
                return;
            }
            const ADIds = new Set([1021, 1016, 921]);
            el.innerHTML = frames.slice(-30).reverse().map(f => {
                const hex = Array.from({length: f.dlc}, (_, i) => toHex(f.data[i], 2)).join(' ');
                const wireId = sniffWireId(f.id), dbcId = sniffDbcId(f.id), displayId = sniffDisplayId(f.id);
                const altId = sniffShowDbcIds ? ('Wire 0x' + toHex(wireId, 3)) : ('DBC ' + sniffBusLabel() + ' 0x' + toHex(dbcId, 3));
                return `<div class="sniff-row${ADIds.has(f.id) ? ' hi' : ''}">
      <span class="s-ts">${(f.ts / 1000).toFixed(1)}s</span>
      <span class="s-id" title="${altId}">0x${toHex(displayId, 3)}</span>
      <div><div class="s-data">${hex}</div>${f.name ? `<div class="s-name">${f.name}</div>` : ''}</div>
    </div>`;
            }).join('');
        }

        async function pollSniffer() {
            return runPoll('frames', async () => {
                if (sniffPaused) return;
                try {
                    const d = await fetchPollJson('/frames', 1500);
                    sniffFrames = d.frames || [];
                    renderSniffer();
                } catch (e) {
                }
            });
        }

        // OTA upload
        function fileSelected(file) {
            if (!file) return;
            otaFile = file;
            const drop = $('ota-drop');
            drop.querySelector('.ota-text').textContent = file.name;
            drop.querySelector('.ota-sub').textContent = (file.size / 1024).toFixed(0) + ' KB';
            $('ota-upload-btn').style.display = 'block';
        }

        function handleDrop(e) {
            e.preventDefault();
            $('ota-drop').classList.remove('drag');
            const file = e.dataTransfer.files[0];
            if (file && file.name.endsWith('.bin')) fileSelected(file);
        }

        async function uploadFirmware() {
            if (!otaFile) return;
            if (!otaUser) {
                otaUser = prompt('OTA Username:') || '';
                localStorage.setItem('otaU', otaUser);
            }
            if (!otaPass) {
                otaPass = prompt('OTA Password:') || '';
                localStorage.setItem('otaP', otaPass);
            }
            if (!otaUser || !otaPass) return;
            const prog = $('ota-progress');
            const fill = $('ota-fill');
            const status = $('ota-status');
            prog.style.display = 'block';
            $('ota-upload-btn').disabled = true;
            $('ota-upload-btn').textContent = 'Flashing...';

            const xhr = new XMLHttpRequest();
            xhr.upload.onprogress = e => {
                if (e.lengthComputable) {
                    const pct = Math.round(e.loaded / e.total * 100);
                    fill.style.width = pct + '%';
                    status.textContent = 'Uploading... ' + pct + '%';
                }
            };
            xhr.onload = () => {
                if (xhr.status === 200) {
                    status.textContent = 'Done! Device is rebooting...';
                    fill.style.width = '100%';
                    setTimeout(() => window.location.reload(), 5000);
                } else {
                    status.textContent = 'Upload failed: ' + xhr.status;
                    status.style.color = 'var(--err)';
                }
                $('ota-upload-btn').disabled = false;
                $('ota-upload-btn').textContent = 'Flash Firmware';
            };
            xhr.onerror = () => {
                status.textContent = 'Connection error';
                status.style.color = 'var(--err)';
                $('ota-upload-btn').disabled = false;
            };
            xhr.open('POST', '/update', true, otaUser, otaPass);
            xhr.setRequestHeader('X-File-Name', otaFile.name);
            xhr.setRequestHeader('X-File-Size', otaFile.size);
            const form = new FormData();
            form.append('firmware', otaFile);
            xhr.send(form);
        }

        async function poll() {
            return runPoll('status', async () => {
                try {
                    const d = await fetchPollJson('/status', 1500);
                    const on = d.can;
                    $('s-can').textContent = on ? 'Active' : 'Offline';
                    $('s-can').className = 'stat-val ' + (on ? 'v-ok' : 'v-err');
                    $('s-inj').textContent = d.ci ? 'Active' : 'BLOCKED';
                    $('s-inj').className = 'stat-val ' + (d.ci ? 'v-ok' : 'v-err');
                    $('s-AD').textContent = d.AD ? 'Active' : 'Inactive';
                    $('s-AD').className = 'stat-val ' + (d.AD ? 'v-ok' : 'v-dim');
                    $('s-fps').textContent = d.fps.toFixed(1) + ' Hz';
                    $('s-fps').className = 'stat-val ' + (d.fps > 5 ? 'v-acc' : 'v-dim');
                    $('s-rx').textContent = d.rx;
                    $('s-tx').textContent = d.tx;
                    $('s-txerr').textContent = d.txerr;
                    $('s-txerr').className = 'stat-val ' + (d.txerr > 0 ? 'v-warn' : 'v-dim');
                    $('s-fd').textContent = d.fd || '—';
                    $('s-prof').textContent = spNames()[d.sp] || '—';
                    $('s-soff').textContent = d.soff || '0';
                    $('s-up').textContent = fmtUp(d.up);
                    if (typeof d.bsCnt !== 'undefined' && typeof d.bsCheckCnt !== "undefined") $('s-bscnt').textContent = d.bsCnt + "/" + d.bsCheckCnt;
                    if (typeof d.spLim !== 'undefined') $('s-splim').textContent = d.spLim > 0 ? d.spLim + ' km/h' : '—';
                    if (typeof d.spLimv !== 'undefined') $('s-splimv').textContent = d.spLimv > 0 ? d.spLimv : '—';
                    $('s-mcp-raw').textContent = 'EFLG: 0x' + toHex(d.eflg, 2);
                    $('fps-fill').style.width = Math.min(d.fps / 20 * 100, 100) + '%';
                    $('hw-badge').textContent = HW[d.hw] || '?';
                    $('dot').className = 'sdot ' + (d.txerr > 5 ? 'dot-warn' : on ? 'dot-on' : 'dot-off');
                    $('hdr-desc').textContent = on ? (d.AD ? 'AD active — injecting' : 'CAN active — monitoring') : 'Waiting for CAN frames';
                    renderEflg(d.eflg);
                    if (d.mux) {
                        for (let i = 0; i < 3; i++) {
                            $(('m' + i + 'rx')).textContent = d.mux[i].rx;
                            $(('m' + i + 'tx')).textContent = d.mux[i].tx;
                            const e = $(('m' + i + 'err'));
                            e.textContent = d.mux[i].err;
                            e.style.color = d.mux[i].err > 0 ? 'var(--err)' : '';
                        }
                    }
                    state.hw = d.hw;
                    state.sp = d.sp;
                    state.can = d.ci;
                    updateInjectButtons(d.ci);
                    updateSniffIdToggle();
                    updSeg($('hw-seg'), d.hw, 'hw-btn');
                    buildPills();
                    if (d.feat) {
                        $('tgl-AD').checked = d.feat.AD;
                        $('tgl-nag').checked = d.feat.nag;
                        $('tgl-summon').checked = d.feat.summon;
                        $('tgl-camera').checked = d.feat.camera;
                        $('tgl-banShield').checked = d.feat.banShield;
                        $('tgl-evd').checked = d.feat.evd;
                        if (typeof d.feat.h4o !== 'undefined') {
                            state.h4o = d.feat.h4o;
                            buildPills();
                        }
                        if (typeof d.feat.spl !== 'undefined') {
                            state.spl = d.feat.spl;
                        }
                    }
                    if (typeof d.eprn !== 'undefined') $('tgl-eprn').checked = d.eprn;

                    $('tgl-use-hw3').checked = d.usehw3;

                    if (!h4oCustomLoaded && d.h4oCust) {
                        h4oCustomLoaded = true;
                        H4O_Custom = d.h4oCust.map(o => ({sl: o.sl, v: o.v}));
                        const hcCustom = $('h4o-custom');
                        if (hcCustom) hcCustom.innerHTML = '';
                        buildPills();
                    }
                    if (!h4oTabLoaded && typeof d.h4oTab !== 'undefined') {
                        h4oTabLoaded = true;
                        setH4OTab(d.h4oTab === 1 ? 'custom' : 'preset');
                    }
                } catch (e) {
                }
            });
        }

        function colorLog(l) {
            if (l.includes('AD=ON') || l.includes('AD active')) return '<span class="lf">' + l + '</span>';
            if (l.match(/\[HW[34]\]|\[LEGACY\]|\[HW3\]/)) return '<span class="lh">' + l + '</span>';
            if (l.includes('ERR') || l.includes('FAIL')) return '<span class="le">' + l + '</span>';
            if (l.includes('[CFG]') || l.includes('[FEAT]')) return '<span class="lc">' + l + '</span>';
            if (l.includes('[OK]') || l.includes('[BOOT]')) return '<span class="lf">' + l + '</span>';
            if (l.includes('[OTA]')) return '<span class="lo">' + l + '</span>';
            return l;
        }

        async function pollLog() {
            return runPoll('log', async () => {
                try {
                    const d = await fetchPollJson('/log?since=' + logSince, 2000);
                    if (d.seq) logSince = d.seq;
                    if (!d.lines.length) return;
                    const el = $('log');
                    const newHtml = d.lines.map(colorLog).join('\n');
                    if (el.textContent === 'Waiting...') el.innerHTML = newHtml;
                    else el.innerHTML += '\n' + newHtml;
                    // trim to 100 lines
                    const lines = el.innerHTML.split('\n');
                    if (lines.length > 100) el.innerHTML = lines.slice(-100).join('\n');
                    el.scrollTop = el.scrollHeight;
                } catch (e) {
                }
            });
        }

        async function resetStats() {
            try {
                await fetch('/reset_stats', {method: 'POST'});
            } catch (e) {
            }
            poll();
        }

        let recIsActive = false, recInterval = null;

        async function toggleRec() {
            recIsActive ? await stopRec() : await startRec();
        }

        async function startRec() {
            try {
                await fetch('/rec_start', {method: 'POST'});
                recIsActive = true;
                const b = $('rec-btn');
                b.textContent = 'Stop Recording';
                b.style.borderColor = 'var(--err)';
                b.style.color = 'var(--err)';
                $('rec-dl').style.display = 'none';
                recInterval = setInterval(pollRec, 800);
            } catch (e) {
            }
        }

        async function stopRec() {
            clearInterval(recInterval);
            recIsActive = false;
            try {
                await fetch('/rec_stop', {method: 'POST'});
            } catch (e) {
            }
            const b = $('rec-btn');
            b.textContent = 'Start Recording';
            b.style.borderColor = '';
            b.style.color = '';
            await pollRec();
        }

        async function pollRec() {
            try {
                const d = await (await fetch('/rec_status')).json();
                const pct = Math.min(d.count / d.cap * 100, 100);
                $('rec-fill').style.width = pct + '%';
                $('rec-count').textContent = d.count + ' / ' + d.cap + ' frames';
                if (d.active) {
                    $('rec-status').textContent = 'Recording...';
                    $('rec-status').style.color = 'var(--err)';
                    $('rec-meta').textContent = 'Recording...';
                } else {
                    $('rec-meta').textContent = d.saved ? d.count + ' frames saved' : 'Idle';
                    $('rec-status').textContent = d.saved ? 'Saved' : 'Ready';
                    $('rec-status').style.color = d.saved ? 'var(--ok)' : '';
                    $('rec-dl').style.display = d.saved ? '' : 'none';
                    if (recIsActive) {
                        recIsActive = false;
                        clearInterval(recInterval);
                        const b = $('rec-btn');
                        b.textContent = 'Start Recording';
                        b.style.borderColor = '';
                        b.style.color = '';
                    }
                }
            } catch (e) {
            }
        }

        // ── AP Hotspot management ──
        async function saveAP() {
            const ssid = $('ap-ssid').value, pass = $('ap-pass').value, hidden = $('ap-hidden').checked ? '1' : '0';
            if (!ssid) {
                $('ap-status').textContent = 'Enter hotspot name';
                $('ap-status').style.color = 'var(--err)';
                return;
            }
            if (pass && pass.length < 8) {
                $('ap-status').textContent = 'Password min 8 chars';
                $('ap-status').style.color = 'var(--err)';
                return;
            }
            try {
                const r = await fetch('/ap_config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass) + '&hidden=' + hidden
                });
                const d = await r.json();
                if (d.ok) {
                    $('ap-status').textContent = 'Saved! Reboot to apply.';
                    $('ap-status').style.color = 'var(--ok)';
                    $('ap-pass').value = '';
                } else {
                    $('ap-status').textContent = d.error || 'Error';
                    $('ap-status').style.color = 'var(--err)';
                }
            } catch (e) {
                $('ap-status').textContent = 'Error';
                $('ap-status').style.color = 'var(--err)';
            }
        }

        async function loadApStatus() {
            return runPoll('ap_status', async () => {
                try {
                    const d = await fetchPollJson('/ap_status', 2000);
                    if (d.ssid) $('ap-ssid').value = d.ssid;
                    $('ap-clients').textContent = d.clients + ' client' + (d.clients !== 1 ? 's' : '');
                    if (typeof d.hidden !== 'undefined') $('ap-hidden').checked = !!d.hidden;
                    if (d.stored) {
                        $('ap-stored').textContent = 'saved';
                        $('ap-stored').style.color = 'var(--ok)';
                    } else {
                        $('ap-stored').textContent = 'firmware default';
                        $('ap-stored').style.color = 'var(--tx3)';
                    }
                } catch (e) {
                }
            });
        }

        // ── WiFi management ──
        function toggleStaticIP() {
            $('static-fields').style.display = $('wifi-static').checked ? 'block' : 'none';
        }

        function rssiIcon(r) {
            if (r >= -50) return '\u2587\u2587\u2587\u2587';
            if (r >= -60) return '\u2587\u2587\u2587\u2581';
            if (r >= -70) return '\u2587\u2587\u2581\u2581';
            return '\u2587\u2581\u2581\u2581';
        }

        async function scanWifi() {
            $('scan-btn').textContent = 'Scanning...';
            $('scan-btn').disabled = true;
            try {
                const r = await fetch('/wifi_scan');
                const d = await r.json();
                const el = $('wifi-nets');
                if (!d.networks.length) {
                    el.innerHTML = '<div style="padding:8px;font-size:11px;color:var(--tx3);text-align:center">No networks found</div>';
                    el.style.display = 'block';
                } else {
                    el.innerHTML = d.networks.map(n => '<div onclick="pickWifi(\'' + n.ssid.replace(/'/g, "\\'") + '\')" style="padding:6px 10px;cursor:pointer;display:flex;justify-content:space-between;align-items:center;border-bottom:1px solid var(--bd);font-size:12px" onmouseover="this.style.background=\'var(--bg)\'" onmouseout="this.style.background=\'\'"><span>' + (n.enc ? '\uD83D\uDD12 ' : '') + n.ssid + '</span><span style="color:var(--tx3);font-size:10px">' + rssiIcon(n.rssi) + ' ' + n.rssi + 'dBm CH' + n.ch + '</span></div>').join('');
                    el.style.display = 'block';
                }
            } catch (e) {
                $('wifi-status').textContent = 'Scan failed';
                $('wifi-status').style.color = 'var(--err)';
            }
            $('scan-btn').textContent = 'Scan';
            $('scan-btn').disabled = false;
        }

        function pickWifi(ssid) {
            $('wifi-ssid').value = ssid;
            $('wifi-nets').style.display = 'none';
            $('wifi-pass').focus();
        }

        async function loadWifiStatus() {
            return runPoll('wifi_status', async () => {
                try {
                    const d = await fetchPollJson('/wifi_status', 2000);
                    dashboardStaIp = d.connected && d.ip ? d.ip : '';
                    if (d.ssid) $('wifi-ssid').value = d.ssid;
                    if (d.stored) {
                        $('wifi-stored').textContent = '\u2022 saved';
                        $('wifi-stored').style.color = 'var(--ok)';
                    } else {
                        $('wifi-stored').textContent = '';
                    }
                    if (d.connected) {
                        $('wifi-status').textContent = (d.ip && d.ip !== location.hostname) ? ('Connected: ' + d.ip + ' \u2022 switch to that WiFi and open this IP') : ('Connected: ' + d.ip);
                        $('wifi-status').style.color = 'var(--ok)';
                    } else if (d.ssid) {
                        $('wifi-status').textContent = 'Connecting to ' + d.ssid + '...';
                        $('wifi-status').style.color = 'var(--acc)';
                    }
                    if (d.static) {
                        $('wifi-static').checked = true;
                        toggleStaticIP();
                        if (d.cfg_ip) $('wifi-ip').value = d.cfg_ip;
                        if (d.cfg_gw) $('wifi-gw').value = d.cfg_gw;
                        if (d.cfg_mask) $('wifi-mask').value = d.cfg_mask;
                        if (d.cfg_dns) $('wifi-dns').value = d.cfg_dns;
                    }
                } catch (e) {
                }
            });
        }

        async function saveWifi() {
            const ssid = $('wifi-ssid').value, pass = $('wifi-pass').value;
            if (!ssid) {
                $('wifi-status').textContent = 'Enter SSID';
                return;
            }
            let body = 'ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass);
            if ($('wifi-static').checked) {
                body += '&static=1&ip=' + encodeURIComponent($('wifi-ip').value) + '&gw=' + encodeURIComponent($('wifi-gw').value) + '&mask=' + encodeURIComponent($('wifi-mask').value) + '&dns=' + encodeURIComponent($('wifi-dns').value);
            }
            try {
                await fetch('/wifi_config', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body
                });
                $('wifi-status').textContent = 'Connecting to ' + ssid + '...';
                $('wifi-status').style.color = 'var(--acc)';
            } catch (e) {
                $('wifi-status').textContent = 'Error';
                $('wifi-status').style.color = 'var(--err)';
            }
        }

        function toggleInfo(id) {
            var el = $(id);
            if (el) el.style.display = el.style.display === 'none' ? 'block' : 'none';
        }

        // ── Firmware update ──
        var pendingUpdateUrl = '';

        async function toggleBeta() {
            const beta = $('beta-tgl').checked ? '1' : '0';
            try {
                await fetch('/update_beta', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'beta=' + beta
                });
            } catch (e) {
            }
            $('upd-info').style.display = 'none';
            $('upd-status').textContent = '';
        }

        async function checkUpdate() {
            $('upd-check-btn').disabled = true;
            $('upd-status').textContent = 'Checking...';
            $('upd-status').style.color = 'var(--acc)';
            $('upd-info').style.display = 'none';
            pendingUpdateUrl = '';
            try {
                const r = await fetch('/update_check');
                const d = await r.json();
                if (!d.ok) {
                    $('upd-status').textContent = d.error || 'Error';
                    $('upd-status').style.color = 'var(--err)';
                    $('upd-check-btn').disabled = false;
                    return;
                }
                $('fw-ver').textContent = 'v' + d.current;
                if (d.update) {
                    $('upd-status').textContent = 'Update available!';
                    $('upd-status').style.color = 'var(--ok)';
                    $('upd-ver').textContent = 'v' + d.latest + (d.prerelease ? ' (beta)' : '');
                    $('upd-detail').textContent = d.artifact + ' \u2022 ' + d.tag;
                    pendingUpdateUrl = d.url;
                    $('upd-info').style.display = 'block';
                } else {
                    $('upd-status').textContent = 'Up to date (v' + d.current + ')';
                    $('upd-status').style.color = 'var(--ok)';
                }
            } catch (e) {
                $('upd-status').textContent = 'Connection error';
                $('upd-status').style.color = 'var(--err)';
            }
            $('upd-check-btn').disabled = false;
        }

        async function installUpdate() {
            if (!pendingUpdateUrl) {
                $('upd-status').textContent = 'No update URL';
                return;
            }
            if (!await dashConfirm('Install firmware update? The device will reboot.', 'Install update', 'Install')) return;
            $('upd-install-btn').disabled = true;
            $('upd-status').textContent = 'Downloading & installing...';
            $('upd-status').style.color = 'var(--acc)';
            try {
                await fetch('/update_install', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'url=' + encodeURIComponent(pendingUpdateUrl)
                });
                $('upd-status').textContent = 'Update installed! Rebooting...';
                $('upd-status').style.color = 'var(--ok)';
                setTimeout(() => location.reload(), 15000);
            } catch (e) {
                $('upd-status').textContent = 'Update failed';
                $('upd-status').style.color = 'var(--err)';
                $('upd-install-btn').disabled = false;
            }
        }

        async function loadUpdateInfo() {
            try {
                const r = await fetch('/update_beta', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'noop=1'
                });
                const d = await r.json();
                if (d.version) {
                    $('fw-ver').textContent = 'v' + d.version;
                    updateFoot(d.version);
                }
                $('beta-tgl').checked = !!d.beta;
            } catch (e) {
            }
            try {
                const r = await fetch('/auto_update');
                const d = await r.json();
                $('auto-upd-tgl').checked = !!d.enabled;
            } catch (e) {
            }
        }

        async function toggleAutoUpdate() {
            const en = $('auto-upd-tgl').checked ? '1' : '0';
            try {
                await fetch('/auto_update', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'enabled=' + en
                });
            } catch (e) {
            }
        }

        function updateFoot(ver) {
            var ip = location.hostname || '192.168.4.1';
            $('dash-foot').textContent = 'ev-open-can-tools \u2022 v' + ver + ' \u2022 ' + ip;
        }

        async function loadCanPins() {
            try {
                const r = await fetch('/can_pins');
                const d = await r.json();
                if (d.tx >= 0) $('can-tx').value = d.tx;
                if (d.rx >= 0) $('can-rx').value = d.rx;
                $('can-pins-status').textContent = d.customized ? ('custom TX=' + d.tx + ' RX=' + d.rx) : ('firmware default TX=' + d.tx + ' RX=' + d.rx);
            } catch (e) {
            }
        }

        async function saveCanPins() {
            var tx = parseInt($('can-tx').value, 10), rx = parseInt($('can-rx').value, 10);
            if (isNaN(tx) || isNaN(rx)) {
                $('can-pins-hint').textContent = 'Enter both TX and RX';
                $('can-pins-hint').style.color = 'var(--err)';
                return;
            }
            if (!await dashConfirm('Save CAN pins TX=' + tx + ' RX=' + rx + ' and reboot? Wrong pins disable CAN.', 'Save CAN pins', 'Save')) return;
            try {
                const r = await fetch('/can_pins', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'tx=' + tx + '&rx=' + rx
                });
                const d = await r.json();
                if (d.ok) {
                    $('can-pins-hint').textContent = 'Saved. Rebooting...';
                    $('can-pins-hint').style.color = 'var(--ok)';
                    await fetch('/reboot', {method: 'POST'});
                    setTimeout(() => location.reload(), 8000);
                } else {
                    $('can-pins-hint').textContent = d.error || 'Save failed';
                    $('can-pins-hint').style.color = 'var(--err)';
                }
            } catch (e) {
                $('can-pins-hint').textContent = 'Connection error';
                $('can-pins-hint').style.color = 'var(--err)';
            }
        }

        async function exportSettings() {
            $('backup-status').textContent = 'Preparing...';
            $('backup-status').style.color = 'var(--tx3)';
            try {
                const r = await fetch('/settings_export');
                if (!r.ok) {
                    throw new Error('HTTP ' + r.status);
                }
                const text = await r.text();
                const blob = new Blob([text], {type: 'application/json'});
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = 'evtools-backup.json';
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                URL.revokeObjectURL(url);
                $('backup-status').textContent = 'Downloaded';
                $('backup-status').style.color = 'var(--ok)';
            } catch (e) {
                $('backup-status').textContent = 'Export failed';
                $('backup-status').style.color = 'var(--err)';
            }
        }

        async function importSettings(ev) {
            const f = ev.target.files[0];
            if (!f) return;
            const text = await f.text();
            try {
                JSON.parse(text);
            } catch (e) {
                $('backup-status').textContent = 'Invalid JSON';
                $('backup-status').style.color = 'var(--err)';
                return;
            }
            if (!await dashConfirm('Restore settings from ' + f.name + ' and reboot?', 'Restore settings', 'Restore')) return;
            $('backup-status').textContent = 'Uploading...';
            $('backup-status').style.color = 'var(--acc)';
            try {
                const r = await fetch('/settings_import', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: text
                });
                const d = await r.json();
                if (d.ok) {
                    $('backup-status').textContent = 'Restored. Rebooting...';
                    $('backup-status').style.color = 'var(--ok)';
                    await fetch('/reboot', {method: 'POST'});
                    setTimeout(() => location.reload(), 8000);
                } else {
                    $('backup-status').textContent = d.error || 'Import failed';
                    $('backup-status').style.color = 'var(--err)';
                }
            } catch (e) {
                $('backup-status').textContent = 'Upload failed';
                $('backup-status').style.color = 'var(--err)';
            }
            ev.target.value = '';
        }

        // ── Debug Injection ──────────────────────────────────────────────
        let dbgState = { active: false, rules: [] };
        let dbgSaveTimer = null;
        const dbgLogOpen = {};   // key "id:mux" → bool (expand state)

        // ── helpers ──

        function dbgMakeEl(tag, attrs, onChange) {
            const el = document.createElement(tag);
            if (attrs.cls) { el.className = attrs.cls; delete attrs.cls; }
            Object.keys(attrs).forEach(k => { el[k] = attrs[k]; });
            if (onChange) el.onchange = onChange;
            return el;
        }

        function dbgMakeTd(style, child) {
            const td = document.createElement('td');
            td.style.cssText = style;
            td.appendChild(child);
            return td;
        }

        // ── row building ──

        function dbgCreateRow(r, i) {
            const tr = document.createElement('tr');
            tr.style.borderBottom = '1px solid var(--bd)';

            // Name
            const inpName = dbgMakeEl('input',
                { cls: 'sniff-input', style: 'width:80px', placeholder: 'func' + (i + 1),
                  value: r.name || '' },
                e => dbgFieldChange(i, 'name', e.target.value));
            tr.appendChild(dbgMakeTd('padding:3px 4px', inpName));

            // CAN ID
            const inpId = dbgMakeEl('input',
                { cls: 'sniff-input', style: 'width:72px', placeholder: '0x1FF',
                  value: r.idStr || (r.id ? '0x' + (r.id | 0).toString(16).toUpperCase() : '') },
                e => dbgFieldChange(i, 'id', e.target.value));
            tr.appendChild(dbgMakeTd('padding:3px 4px', inpId));

            // MUX (select: any / 0-15)
            const selMux = dbgMakeEl('select',
                { cls: 'sniff-input', style: 'width:54px' },
                e => dbgFieldChange(i, 'mux', e.target.value));
            [['any', '-1']].concat(Array.from({length:16},(_,k)=>[String(k),String(k)])).forEach(([label,val]) => {
                const o = document.createElement('option');
                o.value = val; o.textContent = label;
                o.selected = (String(r.mux | 0) === val) || (r.mux < 0 && val === '-1');
                selMux.appendChild(o);
            });
            tr.appendChild(dbgMakeTd('padding:3px 4px', selMux));

            // Bit (select: 0-63)
            const selBit = dbgMakeEl('select',
                { cls: 'sniff-input', style: 'width:54px' },
                e => dbgFieldChange(i, 'bit', e.target.value));
            Array.from({length:64},(_,k)=>[String(k),String(k)]).forEach(([label,val]) => {
                const o = document.createElement('option');
                o.value = val; o.textContent = label;
                o.selected = (String(r.bit < 0 ? 0 : r.bit) === val);
                selBit.appendChild(o);
            });
            tr.appendChild(dbgMakeTd('padding:3px 4px', selBit));

            // Value (select)
            const selVal = dbgMakeEl('select',
                { cls: 'sniff-input', style: 'width:50px' },
                e => dbgFieldChange(i, 'val', e.target.value));
            ['1','0'].forEach(v => {
                const o = document.createElement('option');
                o.value = v; o.textContent = v;
                o.selected = (String(r.val | 0) === v);
                selVal.appendChild(o);
            });
            tr.appendChild(dbgMakeTd('padding:3px 4px', selVal));

            // Enabled checkbox
            const chk = dbgMakeEl('input', { type: 'checkbox', checked: !!r.en },
                e => dbgFieldChange(i, 'en', e.target.checked));
            const tdEn = document.createElement('td');
            tdEn.style.cssText = 'text-align:center;padding:3px 4px';
            tdEn.appendChild(chk);
            tr.appendChild(tdEn);

            // Buttons: copy + delete
            const tdBtns = document.createElement('td');
            tdBtns.style.cssText = 'padding:3px 2px;white-space:nowrap';

            const btnCopy = document.createElement('button');
            btnCopy.className = 'sniff-btn';
            btnCopy.style.cssText = 'padding:2px 7px;margin-right:2px';
            btnCopy.title = 'Copy row';
            btnCopy.textContent = '\u2398';   // ⎘
            btnCopy.onclick = () => dbgCopyRow(i);
            tdBtns.appendChild(btnCopy);

            const btnDel = document.createElement('button');
            btnDel.className = 'sniff-btn';
            btnDel.style.cssText = 'padding:2px 7px';
            btnDel.textContent = '\u00d7';    // ×
            btnDel.onclick = () => dbgRemoveRow(i);
            tdBtns.appendChild(btnDel);

            tr.appendChild(tdBtns);
            return tr;
        }

        function dbgRenderRows() {
            const tbody = $('dbg-rows');
            while (tbody.firstChild) tbody.removeChild(tbody.firstChild);
            if (dbgState.rules.length) {
                dbgState.rules.forEach((r, i) => tbody.appendChild(dbgCreateRow(r, i)));
            } else {
                const tr = document.createElement('tr');
                const td = document.createElement('td');
                td.colSpan = 6;
                td.style.cssText = 'text-align:center;padding:10px;color:var(--tx3);font-size:12px';
                td.textContent = 'No rules \u2014 click \u201c+ Add Rule\u201d';
                tr.appendChild(td); tbody.appendChild(tr);
            }
            $('dbg-meta').textContent = dbgState.rules.length
                + ' rule' + (dbgState.rules.length !== 1 ? 's' : '')
                + (dbgState.active ? ' \u00b7 active' : '');
            const btn = $('dbg-active-btn');
            btn.style.color = dbgState.active ? 'var(--acc)' : '';
            btn.style.borderColor = dbgState.active ? 'var(--acc)' : '';
        }

        // ── CRUD ──

        function dbgAddRow() {
            const idx = dbgState.rules.length + 1;
            dbgState.rules.push({ id: 0, idStr: '', mux: -1, bit: -1, val: 1, en: false, name: 'func' + idx });
            dbgRenderRows();
            dbgScheduleSave();
        }

        function dbgCopyRow(i) {
            const src = dbgState.rules[i];
            if (!src) return;
            dbgState.rules.splice(i + 1, 0, Object.assign({}, src, { en: false }));
            dbgRenderRows();
            dbgScheduleSave();
        }

        function dbgRemoveRow(i) {
            dbgState.rules.splice(i, 1);
            dbgRenderRows();
            dbgScheduleSave();
        }

        function dbgFieldChange(i, field, value) {
            const r = dbgState.rules[i];
            if (!r) return;
            if (field === 'id') {
                const s = String(value).trim();
                const n = s.toLowerCase().startsWith('0x') ? parseInt(s, 16) : parseInt(s, 10);
                r.id = isNaN(n) ? 0 : Math.max(1, Math.min(0x7FF, n));
                r.idStr = s;
            } else if (field === 'mux') {
                const s = String(value).trim();
                const n = parseInt(s, 10);
                r.mux = (s === '' || isNaN(n)) ? -1 : Math.max(0, Math.min(15, n));
            } else if (field === 'bit') {
                const s = String(value).trim();
                const n = parseInt(s, 10);
                r.bit = (s === '' || isNaN(n)) ? -1 : Math.max(0, Math.min(63, n));
            } else if (field === 'val') {
                r.val = parseInt(value, 10) & 1;
            } else if (field === 'en') {
                r.en = !!value;
            } else if (field === 'name') {
                r.name = String(value).substring(0, 23);
            }
            dbgScheduleSave();
        }

        // ── persistence ──

        function dbgScheduleSave() {
            if (dbgSaveTimer) clearTimeout(dbgSaveTimer);
            dbgSaveTimer = setTimeout(dbgSave, 600);
        }

        async function dbgSave() {
            dbgSaveTimer = null;
            const payload = JSON.stringify(dbgState.rules.map(r => ({
                id: r.id | 0, mux: (r.mux >= 0) ? (r.mux | 0) : -1,
                bit: (r.bit >= 0) ? (r.bit | 0) : -1,
                val: r.val | 0, en: r.en ? 1 : 0,
                name: r.name || ''
            })));
            const statusEl = $('dbg-status');
            try {
                await fetchJsonWithTimeout('/dbg_rules', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: payload
                }, 4000);
                statusEl.textContent = 'Saved';
                statusEl.style.color = 'var(--ok)';
                setTimeout(() => { if (statusEl.textContent === 'Saved') statusEl.textContent = ''; }, 2000);
            } catch (e) {
                statusEl.textContent = actionErrorMessage(e, 'Save failed');
                statusEl.style.color = 'var(--err)';
            }
        }

        async function dbgToggleActive() {
            const next = dbgState.active ? '0' : '1';
            try {
                const d = await fetchJsonWithTimeout('/dbg_active', {
                    method: 'POST',
                    headers: { 'Content-Type': 'text/plain' },
                    body: next
                }, 4000);
                dbgState.active = !!(d && d.active);
                dbgRenderRows();
            } catch (e) {
                const statusEl = $('dbg-status');
                statusEl.textContent = actionErrorMessage(e, 'Error');
                statusEl.style.color = 'var(--err)';
            }
        }

        async function dbgLoad() {
            try {
                const d = await fetchJsonWithTimeout('/dbg_rules', null, 3000);
                dbgState.active = !!(d && d.active);
                dbgState.rules = (d && d.rules || []).map((r, i) => ({
                    id: r.id | 0,
                    idStr: r.idStr || (r.id ? '0x' + (r.id | 0).toString(16).toUpperCase() : ''),
                    mux: (r.mux !== undefined && r.mux >= 0) ? (r.mux | 0) : -1,
                    bit: (r.bit !== undefined && r.bit >= 0) ? (r.bit | 0) : -1,
                    val: r.val | 0, en: !!r.en,
                    name: r.name || ('func' + (i + 1))
                }));
                dbgRenderRows();
            } catch (e) { }
        }

        // ── log display ──

        function dbgToBin8(byte) {
            return ((byte | 0) & 255).toString(2).padStart(8, '0');
        }

        function dbgToHex2(byte) {
            return ((byte | 0) & 255).toString(16).padStart(2, '0').toUpperCase();
        }

        function dbgRenderLog(entries) {
            const container = $('dbg-log-entries');
            if (!entries || !entries.length) {
                container.textContent = 'No frames sent yet.';
                return;
            }
            entries.forEach(entry => {
                const key = entry.id + ':' + entry.mux;
                const hex = '0x' + (entry.id | 0).toString(16).toUpperCase();
                const hexBytes = entry.data.map(dbgToHex2).join(' ');
                const label = hex + ' mux=' + entry.mux + '  ' + hexBytes;

                let det = document.getElementById('dbg-log-' + key);
                if (!det) {
                    det = document.createElement('details');
                    det.id = 'dbg-log-' + key;
                    det.style.cssText = 'margin-bottom:4px;border:1px solid var(--bd);border-radius:4px;padding:3px 6px';
                    if (dbgLogOpen[key]) det.open = true;
                    det.addEventListener('toggle', () => { dbgLogOpen[key] = det.open; });
                    container.appendChild(det);
                }

                // Update summary (hex line)
                let sum = det.querySelector('summary');
                if (!sum) {
                    sum = document.createElement('summary');
                    sum.style.cssText = 'cursor:pointer;user-select:none;list-style:none;outline:none';
                    det.insertBefore(sum, det.firstChild);
                }
                sum.textContent = label;

                // Update binary expand body
                let body = det.querySelector('.dbg-log-body');
                if (!body) {
                    body = document.createElement('div');
                    body.className = 'dbg-log-body';
                    body.style.cssText = 'padding:4px 2px 2px 2px;line-height:1.7';
                    det.appendChild(body);
                }
                while (body.firstChild) body.removeChild(body.firstChild);
                entry.data.forEach((byte, bi) => {
                    const row = document.createElement('div');
                    const startBit = bi * 8;
                    const lbl = document.createElement('span');
                    lbl.style.cssText = 'color:var(--tx3);display:inline-block;width:28px;text-align:right;margin-right:6px';
                    lbl.textContent = String(startBit) + ':';
                    const bits = document.createElement('span');
                    bits.textContent = dbgToBin8(byte);
                    row.appendChild(lbl);
                    row.appendChild(bits);
                    body.appendChild(row);
                });
            });

            // Remove stale entries
            Array.from(container.children).forEach(el => {
                const key = el.id.replace('dbg-log-', '');
                if (!entries.some(e => (e.id + ':' + e.mux) === key))
                    container.removeChild(el);
            });
        }

        async function dbgPollLog() {
            try {
                const d = await fetchPollJson('/dbg_log', 2000);
                if (Array.isArray(d)) dbgRenderLog(d);
            } catch (e) { }
        }

        dashboardPollTimers.push(setInterval(poll, 2000));
        dashboardPollTimers.push(setInterval(pollLog, 3000));
        dashboardPollTimers.push(setInterval(pollSniffer, 1000));
        dashboardPollTimers.push(setInterval(loadWifiStatus, 10000));
        dashboardPollTimers.push(setInterval(loadApStatus, 10000));
        dashboardPollTimers.push(setInterval(dbgPollLog, 2000));

        // Tab 切换
        function switchTab(name) {
            document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
            document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
            const pane = document.getElementById('tab-' + name);
            if (pane) pane.classList.add('active');
            const btns = document.querySelectorAll('.tab-btn');
            const labels = { status: '状态', control: '控制', debug: '调试', system: '系统' };
            btns.forEach(b => { if (b.textContent.trim() === labels[name]) b.classList.add('active'); });
            localStorage.setItem('activeTab', name);
        }
        // 恢复上次 tab
        (function() {
            const t = localStorage.getItem('activeTab');
            if (t) switchTab(t);
        })();

        // Collapsible cards
        function toggleCard(id) {
            const card = document.getElementById(id);
            if (!card) return;
            card.classList.toggle('expanded');
            const s = {};
            document.querySelectorAll('.card.collapsible[id]').forEach(c => { s[c.id] = c.classList.contains('expanded'); });
            localStorage.setItem('cardState', JSON.stringify(s));
        }
        try {
            const s = JSON.parse(localStorage.getItem('cardState') || '{}');
            document.querySelectorAll('.card.collapsible[id]').forEach(c => {
                if (s[c.id] === true) c.classList.add('expanded');
            });
        } catch(e) {}

        buildPills();
        updateSniffIdToggle();
        poll();
        pollLog();
        pollSniffer();
        pollRec();
        loadWifiStatus();
        loadApStatus();
        loadUpdateInfo();
        loadCanPins();
        dbgLoad();
    </script>
    </body>
    </html>
    )HTML";
