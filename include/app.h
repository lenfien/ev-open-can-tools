#pragma once

#include <memory>
#include <ArduinoJson.h>
#include "can_frame_types.h"
#include "drivers/can_driver.h"
#include "can_helpers.h"
#include "handlers.h"

#ifndef NATIVE_BUILD
#include <Arduino.h>
#endif
#if defined(DASH_RGB_STATUS_LED) && !defined(NATIVE_BUILD)
#include <esp32-hal-rgb-led.h>
#endif

#ifndef PIN_LED
#define PIN_LED 2
#endif

#if defined(ESP32_DASHBOARD)
    #if DASH_DEFAULT_HW == 0
        using SelectedHandler = LegacyHandler;
    #elif DASH_DEFAULT_HW == 2
        using SelectedHandler = HW4Handler;
    #else
        using SelectedHandler = HW3Handler;
    #endif

#elif defined(NAG_KILLER)
    using SelectedHandler = NagHandler;
    #elif defined(HW4)
    using SelectedHandler = HW4Handler;
    #elif defined(HW3)
    using SelectedHandler = HW3Handler;
    #elif defined(LEGACY)
    using SelectedHandler = LegacyHandler;
    #else
    #error "Define HW4, HW3, LEGACY, or NAG_KILLER in build_flags"
#endif

static CarManagerBase *dashHandler = nullptr;
static std::unique_ptr<CanDriver> appDriver;

// Debug injection hook — set by dashboard to apply dbg_rules after handler
static bool (*appDebugProcess)(CanFrame &, CanDriver &) = nullptr;

static volatile bool frameReady = true;
static void canISR() { frameReady = true; }

#if defined(ESP32_DASHBOARD) && !defined(NATIVE_BUILD)
#include "web/mcp2515_dashboard.h"
#endif

template <typename Driver>
static void appSetup(std::unique_ptr<Driver> drv, const char *readyMsg)
{
    delay(1500);
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 1000)
    {
    }

#if defined(ESP32_DASHBOARD) && !defined(NATIVE_BUILD) && defined(DASH_INJECTION_TOGGLE_PIN)
    pinMode(DASH_INJECTION_TOGGLE_PIN, INPUT_PULLUP);
#endif

#if defined(ESP32_DASHBOARD) && !defined(NATIVE_BUILD) && defined(DASH_RGB_STATUS_LED)
    appRefreshStatusLed(true);
#else
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
#endif

    appDriver = std::move(drv);
    if (!appDriver->init())
    {
        Serial.println("CAN init failed");
    }

    appDriver->setFilters(dashHandler->filterIds(), dashHandler->filterIdCount());
    if constexpr (Driver::kSupportsISR)
    {
        appDriver->enableInterrupt(canISR);
    }

    Serial.println(readyMsg);

#if defined(ESP32_DASHBOARD) && !defined(NATIVE_BUILD)
    delay(2000);
#endif
}

template <typename Driver>
static void appLoop()
{
#if defined(ESP32_DASHBOARD) && !defined(NATIVE_BUILD)
    if (Update.isRunning())
    {
        delay(1);
        return;
    }

#if defined(DASH_INJECTION_TOGGLE_PIN)
    appPollInjectionToggleButton();
#endif
#endif

    if constexpr (Driver::kSupportsISR)
    {
        if (!frameReady)
            return;
        frameReady = false;
    }

    CanFrame frame;
    CarManagerBase *h = dashHandler;
    while (appDriver->read(frame))
    {
        digitalWrite(PIN_LED, LOW);
        h->frameCount++;

        // CanFrame original = frame;
        bool should_send = h->handleMessage(frame, *appDriver);
        if (appDebugProcess)
        {
            if (appDebugProcess(frame, *appDriver))
                should_send = true;
        }

        if (should_send && h->InjectActive)
        {
            h->framesSent++;
            appDriver->send(frame);
            if (h->onSend)
                h->onSend(0, true);
        }
    }
#if !(defined(ESP32_DASHBOARD) && !defined(NATIVE_BUILD) && defined(DASH_RGB_STATUS_LED))
    digitalWrite(PIN_LED, HIGH);
#endif
}
