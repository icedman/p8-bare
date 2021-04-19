#pragma once

#include <Arduino.h>

void startBootloader(bool without_sd = false);
void systemReboot();
void systemOff();