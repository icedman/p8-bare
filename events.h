#pragma once

#include <Arduino.h>

typedef struct {
    uint8_t e, x, y, z;
} event;

#define MAX_EVENTS 32

#define E_BUTTON_PRESSED        1
#define E_BUTTON_RELEASED       2
#define E_BUTTON_LONG_PRESS     3

#define E_TOUCH_PRESSED         4
#define E_TOUCH_RELEASED        5
#define E_TOUCH_LONG_PRESS      6
#define E_TOUCH_GESTURE         7

#define E_INTERRUPT_CHARGING    11
#define E_INTERRUPT_CHARGED     12
#define E_INTERRUPT_TOUCHED     13

#define E_STARTUP               21
#define E_WAKE                  22

#define E_BLE_CONNECTED         31
#define E_BLE_DISCONNECTED      32

void initEvents();
void addEvent(int e, int x = 0, int y = 0, int z = 0);
void clearEvents();
event* getEvents();