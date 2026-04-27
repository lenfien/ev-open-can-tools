//
// Created by hong- on 2026/4/26.
//

#include "app.h"
#include <memory>
#include <ArduinoJson.h>
#include "can_frame_types.h"
#include "drivers/can_driver.h"
#include "handlers.h"
#include "drivers/twai_driver.h"
#include "web_logic.h"

CanHandler *g_can_handler = nullptr;
CanDriver* g_can_driver;

void
AppSetup() {
    delay(1500);
    Serial.begin(115200);
    unsigned long t0 = millis();
    while (!Serial && millis() - t0 < 1000){}

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);

    g_can_driver = new TWAIDriver(TWAI_TX_PIN, TWAI_RX_PIN);
    if (!g_can_driver->init())
        Serial.println("CAN init failed");

    g_can_driver->setFilters(g_can_handler->GetFilterIDs(), g_can_handler->GetFilterIDCnt());
    Serial.println("Ready.");
    delay(2000);
}

void
AppLoop() {
    CanFrame frame;
    CanHandler *h = g_can_handler;
    while (g_can_driver->read(frame))
    {
        digitalWrite(PIN_LED, LOW);
        h->m_state.frame_cnt++;
        mcpDashOnFrame(frame);

        // CanFrame original = frame;
        bool should_send = h->Handle(frame, *g_can_driver);
        if (should_send && h->m_cnf.enable_inject) {

            {
                // Never set this bit to 1 — otherwise it will directly trigger a 1-week suspension.
                if (frame.id == 1021 && frame.GetMux() == 0)
                    frame.SetBit(52, 0);
            }

            g_can_driver->send(frame);
            h->m_state.frame_sent++;
        }
    }

    digitalWrite(PIN_LED, HIGH);
}
