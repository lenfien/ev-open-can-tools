#pragma once

#include <memory>
#include "drivers/can_driver.h"
#include "handlers.h"

extern CanHandler *g_can_handler;
extern CanDriver* g_can_driver;

void AppSetup();
void AppLoop();