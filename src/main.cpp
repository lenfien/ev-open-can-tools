/*
    PlatformIO entry point.
    Shared build settings live in platformio_profile.h.
    Logic is in the shared headers under include/.
*/

#include <Arduino.h>
#include "app.h"
#include "web_logic.h"

void setup()
{
    AppSetup();

    delay(2000);

    WebSetup(g_can_handler, g_can_driver);
}

void loop()
{
    AppLoop();
}
