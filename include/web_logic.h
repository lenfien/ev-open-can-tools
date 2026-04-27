#pragma once

#include "common.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "handlers.h"
#include "web_ui.h"

#define PREFS_NS "ADunlock"

extern Preferences prefs;
extern WebServer   server;

static unsigned long startMs     = 0;
static bool          canOnline   = false;
static unsigned long lastFrameMs = 0;

// WiFi AP
static char    apSSID[33] = "";
static char    apPass[65] = "";
static bool    apHidden   = false;

// WiFi STA
static char      staSSID[33]  = "";
static char      staPass[65]  = "";
static bool      staConnected = false;
static bool      staStaticIP  = false;
static IPAddress staIP(0, 0, 0, 0);
static IPAddress staGW(0, 0, 0, 0);
static IPAddress staMask(255, 255, 255, 0);
static IPAddress staDNS(0, 0, 0, 0);

void mcpDashOnFrame(const CanFrame &f);
void WebSetup(CanHandler *handler, CanDriver *driver);
void mcpDashboardLoop();
