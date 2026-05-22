//
// Created by hong- on 2026/4/26.
//

#include "common.h"
#include "web_logic.h"
#include "ble_probe.h"

Preferences prefs;
WebServer   server(80);

// ── Prefs ─────────────────────────────────────────────────────────

// NVS key 用于持久化四组调试覆盖（mask+value），共 64 字节。
// 结构：[ovr_1016 mask8 val8][ovr_1021_m0 mask8 val8][ovr_1021_m1 mask8 val8][ovr_1021_m2 mask8 val8]
static constexpr const char *PREFS_KEY_DBG_OVR = "dbg_ovr";
static constexpr size_t       DBG_OVR_BYTES    = 4 * 16; // 4 组 * (8 mask + 8 value)

static void
dbgPackOverrides(uint8_t out[DBG_OVR_BYTES]) {
    const DebugOverride *arr[4] = {
        &g_can_handler->m_dbg.ovr_1016,
        &g_can_handler->m_dbg.ovr_1021_m0,
        &g_can_handler->m_dbg.ovr_1021_m1,
        &g_can_handler->m_dbg.ovr_1021_m2,
    };
    uint8_t *p = out;
    for (int i = 0; i < 4; i++) {
        memcpy(p, arr[i]->mask,  8); p += 8;
        memcpy(p, arr[i]->value, 8); p += 8;
    }
}

static void
dbgUnpackOverrides(const uint8_t in[DBG_OVR_BYTES]) {
    DebugOverride *arr[4] = {
        &g_can_handler->m_dbg.ovr_1016,
        &g_can_handler->m_dbg.ovr_1021_m0,
        &g_can_handler->m_dbg.ovr_1021_m1,
        &g_can_handler->m_dbg.ovr_1021_m2,
    };
    const uint8_t *p = in;
    for (int i = 0; i < 4; i++) {
        memcpy(arr[i]->mask,  p, 8); p += 8;
        memcpy(arr[i]->value, p, 8); p += 8;
    }
}

static void
dbgSaveArchive() {
    uint8_t buf[DBG_OVR_BYTES];
    dbgPackOverrides(buf);
    prefs.begin(PREFS_NS, false);
    prefs.putBytes(PREFS_KEY_DBG_OVR, buf, DBG_OVR_BYTES);
    prefs.end();
}

static void
dbgLoadArchive() {
    prefs.begin(PREFS_NS, true);
    size_t sz = prefs.getBytesLength(PREFS_KEY_DBG_OVR);
    if (sz == DBG_OVR_BYTES) {
        uint8_t buf[DBG_OVR_BYTES];
        prefs.getBytes(PREFS_KEY_DBG_OVR, buf, DBG_OVR_BYTES);
        dbgUnpackOverrides(buf);
        Serial.println("[DBG] archive restored from NVS");
    }
    prefs.end();
}

static void
dbgClearArchive() {
    prefs.begin(PREFS_NS, false);
    prefs.remove(PREFS_KEY_DBG_OVR);
    prefs.end();
}

static void
dashSavePrefs() {
    prefs.begin(PREFS_NS, false);
    g_can_handler->SaveConf(prefs);
    prefs.end();
}

static void
dashLoadPrefs() {
    prefs.begin(PREFS_NS, false);
    g_can_handler->LoadConf(prefs);
    BleProbeLoad(prefs);

    String apS = prefs.isKey("ap_ssid") ? prefs.getString("ap_ssid") : "";
    String apP = prefs.isKey("ap_pass") ? prefs.getString("ap_pass") : "";
    strlcpy(apSSID, apS.length() ? apS.c_str() : DASH_SSID, sizeof(apSSID));
    strlcpy(apPass, apP.length() ? apP.c_str() : DASH_PASS, sizeof(apPass));
    apHidden    = prefs.getBool("ap_hidden",   false);
    apDisabled  = prefs.getBool("ap_disabled", false);

    strlcpy(staSSID, prefs.getString("wifi_ssid", "").c_str(), sizeof(staSSID));
    strlcpy(staPass, prefs.getString("wifi_pass", "").c_str(), sizeof(staPass));
    staStaticIP = prefs.getBool("wifi_static", false);
    if (staStaticIP) {
        staIP.fromString(prefs.getString("wifi_ip",   "0.0.0.0"));
        staGW.fromString(prefs.getString("wifi_gw",   "0.0.0.0"));
        staMask.fromString(prefs.getString("wifi_mask", "255.255.255.0"));
        staDNS.fromString(prefs.getString("wifi_dns",  "0.0.0.0"));
    }
    prefs.end();
    g_can_handler->PrintCnf();
}

// ── JSON helper ───────────────────────────────────────────────────

static String
jesc(const String &s) {
    String o;
    o.reserve(s.length() + 4);
    for (unsigned int i = 0; i < s.length(); i++) {
        char c = s[i];
        if      (c == '"')  o += "\\\"";
        else if (c == '\\') o += "\\\\";
        else if (c == '\n') o += "\\n";
        else                o += c;
    }
    return o;
}

// ── Routes ────────────────────────────────────────────────────────

static void
handleRoot() {
    server.sendHeader("Content-Encoding", "gzip");
    server.sendHeader("Cache-Control", "public, max-age=86400");
    server.send_P(200, "text/html", (const char *)DASH_HTML_GZ, DASH_HTML_GZ_LEN);
}

static void
handleStatus() {
    if (canOnline && millis() - lastFrameMs > 10000)
        canOnline = false;

    CanState &s = g_can_handler->m_state;
    CanConf  &c = g_can_handler->m_cnf;
    // 保证下面 schema 驱动的输出用的是最新值
    s.can_online = canOnline ? 1u : 0u;

    String j = "{\"state\":{";
    // schema 驱动：遍历 kStateSchema[] 输出所有只读状态字段（key 即 JSON 字段名）
    bool first = true;
    for (size_t i = 0; i < kStateSchemaCount; ++i) {
        const StateFieldDesc &f = kStateSchema[i];
        uint32_t *slot = (uint32_t *)((char *)&s + f.state_offset);
        if (!first) j += ",";
        first = false;
        j += "\""; j += f.key; j += "\":" + String(*slot);
    }
    // 非 schema 的状态补充字段：只剩 uptime（运行时间 tile 用）。
    if (!first) j += ",";
    j += "\"uptime\":" + String(millis() / 1000);
    j += "},\"cnf\":{";
    // schema 驱动：遍历 kCnfSchema[] 输出所有可写字段（包括 hidden 字段，前端特殊 UI 也读这里）
    bool cnfFirst = true;
    for (size_t i = 0; i < kCnfSchemaCount; ++i) {
        const CnfFieldDesc &f = kCnfSchema[i];
        uint32_t *slot = (uint32_t *)((char *)&c + f.conf_offset);
        if (!cnfFirst) j += ",";
        cnfFirst = false;
        j += "\""; j += f.key; j += "\":";
        if (f.type == FT_BOOL) j += (*slot ? "true" : "false");
        else                   j += String(*slot);
    }
    // auto_cfg 是数组结构，不在 schema 里；保留手写。
    if (!cnfFirst) j += ",";
    j += "\"auto_cfg\":[";
    for (int i = 0; i < 12; i++) {
        if (i > 0) j += ",";
        j += "{\"spd\":" + String(c.speed_limit_auto_cfg[i].speed_limit);
        j += ",\"pct\":" + String(c.speed_limit_auto_cfg[i].offset_percent) + "}";
    }
    j += "]}}";
    server.send(200, "application/json", j);
}

static void
handleConfig() {
    CanConf &c = g_can_handler->m_cnf;

    // 1) Schema 驱动的通用字段：遍历 kCnfSchema[]，有 arg 就写（state 字段只读，不出现在这里）。
    for (size_t i = 0; i < kCnfSchemaCount; ++i) {
        const CnfFieldDesc &f = kCnfSchema[i];
        if (!server.hasArg(f.key)) continue;
        String raw = server.arg(f.key);
        uint32_t v;
        if (f.type == FT_BOOL) {
            v = (raw == "1" || raw == "true") ? 1u : 0u;
        } else {
            v = (uint32_t)raw.toInt();
        }
        SchemaApplyValue(f, c, v);
    }

    // 2) 数组条目（不在 schema 中，手写）
    for (int i = 0; i < 12; i++) {
        String key = "auto_cfg_" + String(i);
        if (server.hasArg(key))
            c.speed_limit_auto_cfg[i].offset_percent = (uint8_t)constrain(server.arg(key).toInt(), 0, 100);
    }

    dashSavePrefs();
    server.send(200, "application/json", "{\"ok\":true}");
}

// GET /schema  → 返回所有 schema 项 的元数据数组，前端据此自动渲染 UI。
static const char *
schemaFieldTypeStr(FieldType t) {
    switch (t) {
        case FT_BOOL:   return "bool";
        case FT_ENUM:   return "enum";
        case FT_NUMBER: return "number";
    }
    return "bool";
}

static const char *
schemaWidgetTypeStr(WidgetType w) {
    switch (w) {
        case WT_CHECKBOX: return "checkbox";
        case WT_SELECT:   return "select";
        case WT_SLIDER:   return "slider";
        case WT_INPUT:    return "input";
    }
    return "checkbox";
}

static void
handleSchema() {
    String j = "{\"cnf\":[";
    for (size_t i = 0; i < kCnfSchemaCount; ++i) {
        const CnfFieldDesc &f = kCnfSchema[i];
        if (i > 0) j += ",";
        j += "{\"key\":\"";    j += f.key;             j += "\"";
        j += ",\"label\":\"";   j += f.label_zh;        j += "\"";
        j += ",\"group\":\"";   j += (f.group ? f.group : ""); j += "\"";
        j += ",\"type\":\"";    j += schemaFieldTypeStr(f.type);    j += "\"";
        j += ",\"widget\":\"";  j += schemaWidgetTypeStr(f.widget); j += "\"";
        if (f.hidden) { j += ",\"hidden\":true"; }
        if (f.type == FT_NUMBER) {
            j += ",\"min\":"  + String(f.min_val);
            j += ",\"max\":"  + String(f.max_val);
            j += ",\"step\":" + String(f.step);
        }
        if (f.type == FT_ENUM && f.enum_options) {
            j += ",\"options\":[";
            bool first = true;
            for (const EnumOption *op = f.enum_options; op->label_zh != nullptr; ++op) {
                if (!first) j += ",";
                first = false;
                j += "{\"v\":" + String(op->value) + ",\"l\":\"" + String(op->label_zh) + "\"}";
            }
            j += "]";
        }
        j += "}";
    }
    j += "],\"state\":[";
    for (size_t i = 0; i < kStateSchemaCount; ++i) {
        const StateFieldDesc &f = kStateSchema[i];
        if (i > 0) j += ",";
        j += "{\"key\":\"";    j += f.key;             j += "\"";
        j += ",\"label\":\"";   j += f.label_zh;        j += "\"";
        j += ",\"type\":\"";    j += schemaFieldTypeStr(f.type);    j += "\"";
        if (f.unit)        { j += ",\"unit\":\"";       j += f.unit;            j += "\""; }
        if (f.tile)        { j += ",\"tile\":\"";       j += f.tile;            j += "\""; }
        if (f.tile_label)  { j += ",\"tile_label\":\""; j += f.tile_label;      j += "\""; }
        if (f.tile_sep)    { j += ",\"tile_sep\":\"";   j += jesc(f.tile_sep);  j += "\""; }
        if (f.enum_labels) { j += ",\"enum_labels\":\""; j += jesc(f.enum_labels); j += "\""; }
        j += "}";
    }
    j += "]}";
    server.send(200, "application/json", j);
}

static void
handleReboot() {
    server.send(200, "text/plain", "Rebooting...");
    delay(200);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
    ESP.restart();
}

// ------------------------ WIFI ----------------------------- //

static void
dashConnectSTA() {
    if (!strlen(staSSID)) return;
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.setSleep(WIFI_PS_MIN_MODEM);
    if (apDisabled) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(apSSID, apPass, 1, apHidden ? 1 : 0, 4);
    }

    if (staStaticIP && (uint32_t)staIP != 0)
        WiFi.config(staIP, staGW, staMask, staDNS);
    else
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.begin(staSSID, staPass);
}

static void
dashCheckWifi() {
    static unsigned long lastCheck  = 0;
    static unsigned long lastReconn = 0;
    if (!strlen(staSSID) || millis() - lastCheck < 5000) return;
    lastCheck = millis();

    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected != staConnected) {
        staConnected = connected;
        Serial.printf("[WIFI] STA %s\n", connected ? WiFi.localIP().toString().c_str() : "disconnected");
    }

    if (!connected && millis() - lastReconn > 30000) {
        lastReconn = millis();
        Serial.println("[WIFI] reconnecting...");
        WiFi.disconnect(false);
        WiFi.begin(staSSID, staPass);
    }
}

static void
handleWifiScan() {
    int n = WiFi.scanComplete();
    if (n == WIFI_SCAN_RUNNING) {
        server.send(202, "application/json", "{\"scanning\":true}");
        return;
    }
    if (n == WIFI_SCAN_FAILED || n == 0) {
        WiFi.scanNetworks(true, false, false, 120);
        server.send(202, "application/json", "{\"scanning\":true}");
        return;
    }
    String j = "{\"networks\":[";
    for (int i = 0; i < n && i < 20; i++) {
        if (i) j += ",";
        j += "{\"ssid\":\"" + jesc(WiFi.SSID(i)) + "\"";
        j += ",\"rssi\":"   + String(WiFi.RSSI(i));
        j += ",\"enc\":"    + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
        j += "}";
    }
    WiFi.scanDelete();
    j += "]}";
    server.send(200, "application/json", j);
}

static void
handleWifiConfig() {
    if (!server.hasArg("ssid")) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing ssid\"}");
        return;
    }
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    strlcpy(staSSID, ssid.c_str(), sizeof(staSSID));
    strlcpy(staPass, pass.c_str(), sizeof(staPass));

    prefs.begin(PREFS_NS, false);
    prefs.putString("wifi_ssid", ssid);
    prefs.putString("wifi_pass", pass);
    if (server.hasArg("static") && server.arg("static") == "1") {
        staStaticIP = true;
        staIP.fromString(server.arg("ip"));
        staGW.fromString(server.arg("gw"));
        staMask.fromString(server.arg("mask"));
        staDNS.fromString(server.arg("dns"));
        prefs.putBool("wifi_static",  true);
        prefs.putString("wifi_ip",   server.arg("ip"));
        prefs.putString("wifi_gw",   server.arg("gw"));
        prefs.putString("wifi_mask", server.arg("mask"));
        prefs.putString("wifi_dns",  server.arg("dns"));
    } else {
        staStaticIP = false;
        prefs.putBool("wifi_static", false);
    }
    prefs.end();

    staConnected = false;
    dashConnectSTA();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void
handleWifiStatus() {
    String j = "{\"connected\":";
    j += staConnected ? "true" : "false";
    j += ",\"ssid\":\"" + jesc(staSSID) + "\"";
    if (staConnected)
        j += ",\"ip\":\"" + WiFi.localIP().toString() + "\"";
    j += ",\"static\":" + String(staStaticIP ? "true" : "false");
    j += "}";
    server.send(200, "application/json", j);
}

static void
handleApStatus() {
    String j = "{\"ssid\":\"" + jesc(apSSID) + "\"";
    j += ",\"ip\":\""      + WiFi.softAPIP().toString() + "\"";
    j += ",\"clients\":"   + String(WiFi.softAPgetStationNum());
    j += ",\"hidden\":"    + String(apHidden   ? "true" : "false");
    j += ",\"disabled\":"  + String(apDisabled ? "true" : "false");
    j += "}";
    server.send(200, "application/json", j);
}

static void
handleApConfig() {
    String newSsid = server.arg("ssid");
    String newPass = server.arg("pass");
    bool onlyToggle = !newSsid.length() && server.hasArg("disabled") && !server.hasArg("hidden");
    if (!newSsid.length() && !onlyToggle) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"SSID required\"}");
        return;
    }
    if (newPass.length() > 0 && newPass.length() < 8) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"Password min 8 chars\"}");
        return;
    }
    if (newSsid.length())
        strlcpy(apSSID, newSsid.c_str(), sizeof(apSSID));
    if (newPass.length())
        strlcpy(apPass, newPass.c_str(), sizeof(apPass));
    if (server.hasArg("hidden"))
        apHidden = server.arg("hidden") == "1";
    if (server.hasArg("disabled")) {
        bool wantDisable = server.arg("disabled") == "1";
        if (wantDisable && WiFi.status() != WL_CONNECTED) {
            server.send(400, "application/json", "{\"ok\":false,\"error\":\"STA not connected — cannot disable AP\"}");
            return;
        }
        apDisabled = wantDisable;
    }

    prefs.begin(PREFS_NS, false);
    if (newSsid.length())
        prefs.putString("ap_ssid", newSsid);
    if (newPass.length())
        prefs.putString("ap_pass", newPass);
    if (server.hasArg("hidden"))
        prefs.putBool("ap_hidden", apHidden);
    if (server.hasArg("disabled"))
        prefs.putBool("ap_disabled", apDisabled);
    prefs.end();
    server.send(200, "application/json", "{\"ok\":true}");
}

// ------------------------ BLE probe ----------------------------- //

static void
handleBleStatus() {
    BleProbeStatus st = BleProbeGetStatus();
    String j = "{\"enabled\":";
    j += st.enabled ? "true" : "false";
    j += ",\"advertising\":";
    j += st.advertising ? "true" : "false";
    j += ",\"use_saved_timing\":";
    j += st.use_saved_timing ? "true" : "false";
    j += ",\"uuid\":\"";
    j += jesc(st.uuid);
    j += "\",\"window_ms\":";
    j += String(st.window_ms);
    j += ",\"duration_ms\":";
    j += String(st.duration_ms);
    j += ",\"interval_ms\":";
    j += String(st.interval_ms);
    j += ",\"cycle_remaining_ms\":";
    j += String(st.cycle_remaining_ms);
    j += ",\"adv_remaining_ms\":";
    j += String(st.adv_remaining_ms);
    j += "}";
    server.send(200, "application/json", j);
}

static void
handleBleConfig() {
    if (!server.hasArg("uuid") || !server.hasArg("window_ms") || !server.hasArg("duration_ms") || !server.hasArg("interval_ms")) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing args\"}");
        return;
    }
    String uuid = server.arg("uuid");
    uuid.trim();
    uint32_t window_ms = (uint32_t)server.arg("window_ms").toInt();
    uint32_t duration_ms = (uint32_t)server.arg("duration_ms").toInt();
    uint32_t interval_ms = (uint32_t)server.arg("interval_ms").toInt();
    bool enabled = !server.hasArg("enabled") || server.arg("enabled") == "1";
    bool use_saved_timing = server.hasArg("use_saved_timing") && server.arg("use_saved_timing") == "1";
    if (!BleProbeSetConfig(uuid, window_ms, duration_ms, interval_ms, enabled, use_saved_timing)) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad uuid\"}");
        return;
    }

    prefs.begin(PREFS_NS, false);
    BleProbeSave(prefs);
    prefs.end();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void
handleBleGenerate() {
    String uuid = BleProbeGenerateUuid();
    String j = "{\"uuid\":\"";
    j += uuid;
    j += "\"}";
    server.send(200, "application/json", j);
}

// ── Debug overlay (不落盘，用于调试页覆盖指定 bit) ─────────────

static DebugOverride *
dbgPickOverride(const String &frame_key) {
    if (frame_key == "1016")    return &g_can_handler->m_dbg.ovr_1016;
    if (frame_key == "1021_m0") return &g_can_handler->m_dbg.ovr_1021_m0;
    if (frame_key == "1021_m1") return &g_can_handler->m_dbg.ovr_1021_m1;
    if (frame_key == "1021_m2") return &g_can_handler->m_dbg.ovr_1021_m2;
    return nullptr;
}

static const DebugLastFrame *
dbgPickLast(const String &frame_key) {
    if (frame_key == "1016")    return &g_can_handler->m_dbg.last_1016;
    if (frame_key == "1021_m0") return &g_can_handler->m_dbg.last_1021_m0;
    if (frame_key == "1021_m1") return &g_can_handler->m_dbg.last_1021_m1;
    if (frame_key == "1021_m2") return &g_can_handler->m_dbg.last_1021_m2;
    return nullptr;
}

static void
dbgAppendGroup(String &j, const char *key, const DebugLastFrame &last, const DebugOverride &ovr) {
    j += "\""; j += key; j += "\":{";
    j += "\"seen\":"; j += (last.seen ? "true" : "false");
    j += ",\"data\":[";
    for (int i = 0; i < 8; i++) {
        if (i) j += ",";
        j += String(last.data[i]);
    }
    j += "],\"mask\":[";
    for (int i = 0; i < 8; i++) {
        if (i) j += ",";
        j += String(ovr.mask[i]);
    }
    j += "],\"val\":[";
    for (int i = 0; i < 8; i++) {
        if (i) j += ",";
        j += String(ovr.value[i]);
    }
    j += "]}";
}

static void
handleDebugStatus() {
    String j = "{";
    dbgAppendGroup(j, "f1016",  g_can_handler->m_dbg.last_1016,    g_can_handler->m_dbg.ovr_1016);
    j += ",";
    dbgAppendGroup(j, "f1021_m0", g_can_handler->m_dbg.last_1021_m0, g_can_handler->m_dbg.ovr_1021_m0);
    j += ",";
    dbgAppendGroup(j, "f1021_m1", g_can_handler->m_dbg.last_1021_m1, g_can_handler->m_dbg.ovr_1021_m1);
    j += ",";
    dbgAppendGroup(j, "f1021_m2", g_can_handler->m_dbg.last_1021_m2, g_can_handler->m_dbg.ovr_1021_m2);
    j += "}";
    server.send(200, "application/json", j);
}

// POST /debug_set
//   frame=1016|1021_m0|1021_m1|1021_m2
//   start=<bit offset 0..63>
//   length=<1..32>
//   value=<uint>
//   override=0|1   (1=开启覆盖并写入 value; 0=清除这段 bits 的覆盖)
static void
handleDebugSet() {
    if (!server.hasArg("frame") || !server.hasArg("start") || !server.hasArg("length")) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing args\"}");
        return;
    }
    DebugOverride *ovr = dbgPickOverride(server.arg("frame"));
    if (!ovr) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad frame\"}");
        return;
    }
    int start  = server.arg("start").toInt();
    int length = server.arg("length").toInt();
    if (start < 0 || length <= 0 || length > 32 || start + length > 64) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad range\"}");
        return;
    }
    bool   on    = server.hasArg("override") && server.arg("override") == "1";
    uint32_t val = server.hasArg("value") ? (uint32_t)strtoul(server.arg("value").c_str(), nullptr, 10) : 0u;

    for (int i = 0; i < length; i++) {
        int  bit      = start + i;
        int  byteIdx  = bit / 8;
        int  bitIdx   = bit % 8;
        uint8_t mbit  = (uint8_t)(1u << bitIdx);
        if (on) {
            ovr->mask[byteIdx]  |= mbit;
            if ((val >> i) & 1u) ovr->value[byteIdx] |=  mbit;
            else                 ovr->value[byteIdx] &= (uint8_t)~mbit;
        } else {
            ovr->mask[byteIdx]  &= (uint8_t)~mbit;
            ovr->value[byteIdx] &= (uint8_t)~mbit;
        }
    }
    server.send(200, "application/json", "{\"ok\":true}");
}

static void
handleDebugClear() {
    if (!server.hasArg("frame")) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing frame\"}");
        return;
    }
    if (server.arg("frame") == "all") {
        g_can_handler->m_dbg.ovr_1016    = {};
        g_can_handler->m_dbg.ovr_1021_m0 = {};
        g_can_handler->m_dbg.ovr_1021_m1 = {};
        g_can_handler->m_dbg.ovr_1021_m2 = {};
        // 清除内存同时抹掉 NVS 存档，避免重启后又被加载回来。
        dbgClearArchive();
        server.send(200, "application/json", "{\"ok\":true}");
        return;
    }
    DebugOverride *ovr = dbgPickOverride(server.arg("frame"));
    if (!ovr) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad frame\"}");
        return;
    }
    *ovr = {};
    server.send(200, "application/json", "{\"ok\":true}");
}

// POST /debug_archive_save — 把当前四组 DebugOverride 整体写入 NVS，重启后自动恢复。
static void
handleDebugArchiveSave() {
    dbgSaveArchive();
    server.send(200, "application/json", "{\"ok\":true}");
}

// ── Task & public API ─────────────────────────────────────────────

static void
webTask(void *) {
    for (;;) {
        server.handleClient();
        dashCheckWifi();
        BleProbeLoop();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

void
mcpDashOnFrame(const CanFrame &) {
    lastFrameMs = millis();
    canOnline   = true;
}

void
WebSetup(CanHandler * /*handler*/, CanDriver * /*driver*/) {
    startMs = millis();
    dashLoadPrefs();
    dbgLoadArchive();

    WiFi.persistent(false);
    WiFi.setSleep(WIFI_PS_MIN_MODEM);
    if (apDisabled) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(apSSID, apPass, 1, apHidden ? 1 : 0, 4);
        Serial.printf("[WIFI] AP: %s  IP: %s\n", apSSID, WiFi.softAPIP().toString().c_str());
    }
    if (strlen(staSSID)) {
        if (staStaticIP && (uint32_t)staIP != 0)
            WiFi.config(staIP, staGW, staMask, staDNS);
        WiFi.setAutoReconnect(false);
        WiFi.setSleep(WIFI_PS_MIN_MODEM);
        WiFi.begin(staSSID, staPass);
    }
    BleProbeSetup();

    server.on("/",             HTTP_GET,  handleRoot);
    server.on("/status",       HTTP_GET,  handleStatus);
    server.on("/schema",       HTTP_GET,  handleSchema);
    server.on("/config",       HTTP_POST, handleConfig);
    server.on("/reboot",       HTTP_POST, handleReboot);
    server.on("/wifi_scan",    HTTP_GET,  handleWifiScan);
    server.on("/wifi_config",  HTTP_POST, handleWifiConfig);
    server.on("/wifi_status",  HTTP_GET,  handleWifiStatus);
    server.on("/ap_status",    HTTP_GET,  handleApStatus);
    server.on("/ap_config",    HTTP_POST, handleApConfig);
    server.on("/ble_status",   HTTP_GET,  handleBleStatus);
    server.on("/ble_config",   HTTP_POST, handleBleConfig);
    server.on("/ble_generate", HTTP_POST, handleBleGenerate);
    server.on("/debug_status",       HTTP_GET,  handleDebugStatus);
    server.on("/debug_set",          HTTP_POST, handleDebugSet);
    server.on("/debug_clear",        HTTP_POST, handleDebugClear);
    server.on("/debug_archive_save", HTTP_POST, handleDebugArchiveSave);
    server.begin();

    xTaskCreatePinnedToCore(webTask, "web", 6144, nullptr, 1, nullptr, 0);
    Serial.println("[WEB] http://" + WiFi.softAPIP().toString());
}

void
mcpDashboardLoop() {
    if (canOnline && millis() - lastFrameMs > 10000)
        canOnline = false;

    // 每 1 秒采样一次 frame_cnt / frame_sent 的差分，得到每秒收/发帧数，
    // 写回 CanState.frame_rx_rate / frame_tx_rate，前端从 schema 驱动的 /status 直接拿到。
    static uint32_t lastSampleMs   = 0;
    static uint32_t lastFrameCnt   = 0;
    static uint32_t lastFrameSent  = 0;
    uint32_t now = millis();
    if (lastSampleMs == 0) {
        lastSampleMs  = now;
        lastFrameCnt  = g_can_handler->m_state.frame_cnt;
        lastFrameSent = g_can_handler->m_state.frame_sent;
    } else if (now - lastSampleMs >= 1000) {
        uint32_t dt_ms = now - lastSampleMs;
        uint32_t cnt   = g_can_handler->m_state.frame_cnt;
        uint32_t sent  = g_can_handler->m_state.frame_sent;
        g_can_handler->m_state.frame_rx_rate = (uint32_t)((uint64_t)(cnt  - lastFrameCnt)  * 1000u / dt_ms);
        g_can_handler->m_state.frame_tx_rate = (uint32_t)((uint64_t)(sent - lastFrameSent) * 1000u / dt_ms);
        lastSampleMs  = now;
        lastFrameCnt  = cnt;
        lastFrameSent = sent;
    }

    g_can_handler->m_state.can_online = canOnline ? 1u : 0u;
}
