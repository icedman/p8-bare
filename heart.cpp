/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "heart.h"
#include "pinout.h"
#include "i2c.h"
#include "io.h"
#include "sleep.h"
#include "HRS3300lib.h"

bool heartrate_enable = false;
bool is_heartrate_enable = false;
byte last_heartrate_ms;
byte last_heartrate;

//timed heartrate stuff
bool timed_heart_rates = true;
bool has_good_heartrate = false;
int hr_answers;
bool disabled_hr_allready = false;

void initHeart() {
    // pinMode(HRS3300_TEST, INPUT);

    pinMode(HRS3300_TEST, OUTPUT);
    digitalWrite(HRS3300_TEST, LOW);

    HRS3300_begin(userI2CRead, userI2CWrite); //set the i2c read and write function so it can be a user defined i2c hardware see i2c.h
    heartrate_enable = true;
    endHeart();
}

void startHeart() {
    if (!heartrate_enable) {
        HRS3300_enable();
        heartrate_enable = true;
    }
}

void endHeart() {
    if (heartrate_enable) {
        heartrate_enable = false;
        ledEnable(false);
    }
}

byte getHeart() {
    byte hr = last_heartrate_ms;
    switch (hr) {
    case 0:
        break;
    case 255:
        break;
    case 254://No Touch
        break;
    case 253://Please wait
        break;
    default:
        last_heartrate = hr;
        break;
    }
    return hr;
}

void updateHeart() {
    if (heartrate_enable) {
        last_heartrate_ms = HRS3300_getHR();
    }
}
