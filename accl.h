/*
   Copyright (c) 2020 Aaron Christophel

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "Arduino.h"

#define USE_SC7A20
// #define COUNT_STEPS

struct accl_data_struct {
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t temp;
    uint32_t steps;
    uint8_t activity;
};

void initAccel();
void readAccel();
accl_data_struct getAccelData();