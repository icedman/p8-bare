#pragma once

#include <Arduino.h>

typedef struct {
    uint8_t button;
    uint8_t charged;
    uint8_t charging;
    uint8_t touch;
    uint8_t accel;
} interrupt;

void initInterrupt();
void readInterrupts();
void clearInterrupts();
interrupt* getInterrupts();