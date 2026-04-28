//
// Created by hong- on 2026/4/26.
//

#include "common.h"
#include "web_logic.h"

Preferences prefs;
WebServer   server(80);

// ── Prefs ─────────────────────────────────────────────────────────

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

    String apS = prefs.getString("ap_ssid", "");
    String apP = prefs.getString("ap_pass", "");
    strlcpy(apSSID, apS.length() ? apS.c_str() : DASH_SSID, sizeof(apSSID));
    strlcpy(apPass, apP.length() ? apP.c_str() : DASH_PASS, sizeof(apPass));
    apHidden = prefs.getBool("ap_hidden", false);

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

// ── WiFi helpers ──────────────────────────────────────────────────

static void
dashConnectSTA() {
    if (!strlen(staSSID)) return;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(apSSID, apPass, 1, apHidden ? 1 : 0, 4);
    if (staStaticIP && (uint32_t)staIP != 0)
        WiFi.config(staIP, staGW, staMask, staDNS);
    else
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.begin(staSSID, staPass);
}

static void
dashCheckWifi() {
    static unsigned long lastCheck = 0;
    if (!strlen(staSSID) || millis() - lastCheck < 5000) return;
    lastCheck = millis();
    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected != staConnected) {
        staConnected = connected;
        Serial.printf("[WIFI] STA %s\n",
            connected ? WiFi.localIP().toString().c_str() : "disconnected");
    }
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
    server.send_P(200, "text/html", DASH_HTML);
}

static void
handleStatus() {
    if (canOnline && millis() - lastFrameMs > 10000)
        canOnline = false;

    CanState &s = g_can_handler->m_state;
    CanConf  &c = g_can_handler->m_cnf;

    String j = "{\"state\":{";
    j += "\"follow_distance\":"          + String(s.follow_distance);
    j += ",\"profile_hw3\":"             + String(s.speed_profile_to_hw3);
    j += ",\"profile_hw4\":"             + String(s.speed_profile_to_hw4);
    j += ",\"frame_cnt\":"               + String(s.frame_cnt);
    j += ",\"frame_sent\":"              + String(s.frame_sent);
    j += ",\"ban_shield_cnt\":"          + String(s.ban_shield_cnt);
    j += ",\"ban_shield_check_cnt\":"    + String(s.ban_shield_check_cnt);
    j += ",\"speed_limit_fused\":"       + String(s.speed_limit_fused);
    j += ",\"speed_limit_vision_only\":" + String(s.speed_limit_vision_only);
    j += ",\"speed_offset\":"            + String(s.speed_offset);
    j += ",\"gateway_autopilot\":"       + String(s.gateway_autopilot);
    j += ",\"uptime\":"                  + String(millis() / 1000);
    j += ",\"can_online\":"              + String(canOnline ? "true" : "false");
    j += "},\"cnf\":{";
#define JB(field) j += ",\"" #field "\":" + String(c.field ? "true" : "false")
#define JU(field) j += ",\"" #field "\":" + String(c.field)
    j += "\"enable_inject\":"            + String(c.enable_inject ? "true" : "false");
    JB(enable_fsd);
    JB(enable_print);
    JB(use_hw3_code);
    JB(enable_ban_shield);
    JB(enable_nag_suppress);
    JB(enable_summon_unlock);
    JB(enable_enhanced_autopilot_runtime);
    JB(disable_camera);
    JB(enable_emergency_vehicle_detection_runtime);
    JB(enable_isa_speed_chime_suppress_runtime);
    JB(speed_profile_use_follow_distance);
    JU(speed_profile_from_web);
    JB(enable_set_hw3_profile);
    JB(speed_offset_enable_override);
    JB(speed_offset_use_fix_or_dynamic);
    JU(speed_offset_fix_from_web);
    JB(start_from_park);
#undef JB
#undef JU
    j += ",\"auto_cfg\":[";
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

    struct { const char *k; uint32_t *f; } bools[] = {
        {"enable_inject",                              &c.enable_inject},
        {"enable_fsd",                                 &c.enable_fsd},
        {"enable_print",                               &c.enable_print},
        {"use_hw3_code",                               &c.use_hw3_code},
        {"enable_ban_shield",                          &c.enable_ban_shield},
        {"enable_nag_suppress",                        &c.enable_nag_suppress},
        {"enable_summon_unlock",                       &c.enable_summon_unlock},
        {"enable_enhanced_autopilot_runtime",          &c.enable_enhanced_autopilot_runtime},
        {"disable_camera",                             &c.disable_camera},
        {"enable_emergency_vehicle_detection_runtime", &c.enable_emergency_vehicle_detection_runtime},
        {"enable_isa_speed_chime_suppress_runtime",    &c.enable_isa_speed_chime_suppress_runtime},
        {"speed_profile_use_follow_distance",       &c.speed_profile_use_follow_distance},
        {"enable_set_hw3_profile",                     &c.enable_set_hw3_profile},
        {"speed_offset_enable_override",               &c.speed_offset_enable_override},
        {"speed_offset_use_fix_or_dynamic",            &c.speed_offset_use_fix_or_dynamic},
        {"start_from_park",                            &c.start_from_park},
    };
    for (auto &b : bools) {
        if (server.hasArg(b.k))
            *b.f = server.arg(b.k) == "1" ? 1u : 0u;
    }
    if (server.hasArg("speed_profile_from_web")) {
        uint32_t v = (uint32_t)server.arg("speed_profile_from_web").toInt();
        if (v >= 1 && v <= 5) c.speed_profile_from_web = v;
    }
    if (server.hasArg("speed_offset_fix_from_web")) {
        int v = server.arg("speed_offset_fix_from_web").toInt();
        c.speed_offset_fix_from_web = (uint32_t)constrain(v, 0, 50);
    }
    for (int i = 0; i < 12; i++) {
        String key = "auto_cfg_" + String(i);
        if (server.hasArg(key))
            c.speed_limit_auto_cfg[i].offset_percent = (uint8_t)constrain(server.arg(key).toInt(), 0, 100);
    }

    dashSavePrefs();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void
handleReboot() {
    server.send(200, "text/plain", "Rebooting...");
    delay(200);
    ESP.restart();
}

static void
handleWifiScan() {
    int n = WiFi.scanNetworks(false, false, false, 300);
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
    j += ",\"ip\":\""     + WiFi.softAPIP().toString() + "\"";
    j += ",\"clients\":"  + String(WiFi.softAPgetStationNum());
    j += ",\"hidden\":"   + String(apHidden ? "true" : "false");
    j += "}";
    server.send(200, "application/json", j);
}

static void
handleApConfig() {
    String newSsid = server.arg("ssid");
    String newPass = server.arg("pass");
    if (!newSsid.length()) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"SSID required\"}");
        return;
    }
    if (newPass.length() > 0 && newPass.length() < 8) {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"Password min 8 chars\"}");
        return;
    }
    strlcpy(apSSID, newSsid.c_str(), sizeof(apSSID));
    if (newPass.length())
        strlcpy(apPass, newPass.c_str(), sizeof(apPass));
    if (server.hasArg("hidden"))
        apHidden = server.arg("hidden") == "1";

    prefs.begin(PREFS_NS, false);
    prefs.putString("ap_ssid", newSsid);
    if (newPass.length())
        prefs.putString("ap_pass", newPass);
    if (server.hasArg("hidden"))
        prefs.putBool("ap_hidden", apHidden);
    prefs.end();
    server.send(200, "application/json", "{\"ok\":true}");
}

// ── Task & public API ─────────────────────────────────────────────

static void
webTask(void *) {
    for (;;) {
        server.handleClient();
        dashCheckWifi();
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

    if (strlen(staSSID)) {
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(apSSID, apPass, 1, apHidden ? 1 : 0, 4);
        if (staStaticIP && (uint32_t)staIP != 0)
            WiFi.config(staIP, staGW, staMask, staDNS);
        WiFi.begin(staSSID, staPass);
    } else {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPass, 1, apHidden ? 1 : 0, 4);
    }
    Serial.printf("[WIFI] AP: %s  IP: %s\n", apSSID, WiFi.softAPIP().toString().c_str());

    server.on("/",            HTTP_GET,  handleRoot);
    server.on("/status",      HTTP_GET,  handleStatus);
    server.on("/config",      HTTP_POST, handleConfig);
    server.on("/reboot",      HTTP_POST, handleReboot);
    server.on("/wifi_scan",   HTTP_GET,  handleWifiScan);
    server.on("/wifi_config", HTTP_POST, handleWifiConfig);
    server.on("/wifi_status", HTTP_GET,  handleWifiStatus);
    server.on("/ap_status",   HTTP_GET,  handleApStatus);
    server.on("/ap_config",   HTTP_POST, handleApConfig);
    server.begin();

    xTaskCreatePinnedToCore(webTask, "web", 6144, nullptr, 1, nullptr, 0);
    Serial.println("[WEB] http://" + WiFi.softAPIP().toString());
}

void
mcpDashboardLoop() {
    if (canOnline && millis() - lastFrameMs > 10000)
        canOnline = false;
}
