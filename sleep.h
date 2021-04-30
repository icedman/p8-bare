#pragma once

#include <Arduino.h>

#define SLEEP_TIME 5000 // 5 seconds

void initSleep();
void goToSleep();
void wakeUp();
void sleepWait();
void caffeinate();
bool isSleeping();
void updateSleep();
bool shouldSleep();
bool onTwoSeconds();

void disableHardware();