/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "io.h"
#include "pinout.h"
#include "events.h"

// volatile long vibration_end_time = 0;
// volatile long led_end_time = 0;
volatile bool inputoutput_inited = false;
// volatile int motor_power = 100;

static int buttonDebounce = 0;
static bool buttonDown = false;
static long buttonDownTime = 0;

void initIO() {

    // pinMode(STATUS_LED, OUTPUT);
    // pinMode(VIBRATOR_OUT, OUTPUT);
    pinMode(PUSH_BUTTON_IN, INPUT);
    if (PUSH_BUTTON_OUT != -1) {
        pinMode(PUSH_BUTTON_OUT, OUTPUT);
        digitalWrite(PUSH_BUTTON_OUT, HIGH);
    }

    // set_led(0);
    // set_motor(0);

    inputoutput_inited = true;
}

void readIO()
{
    bool prevButton = buttonDown;
    buttonDown = getButton();

    if (prevButton != buttonDown && buttonDown) {
        if (buttonDebounce == 0) {
            addEvent(E_BUTTON_PRESSED);
            buttonDownTime = millis();
        }
        buttonDebounce = 4;
    } else if (buttonDown) {
        if (buttonDownTime > 0 && millis() - buttonDownTime > 5000) {
            addEvent(E_BUTTON_LONG_PRESS);
            buttonDownTime = 0;
        }
    }

    if (buttonDebounce > 0 && !buttonDown) {
        if (buttonDebounce-- < 2) {
            addEvent(E_BUTTON_RELEASED);
            buttonDebounce = 0;
            buttonDownTime = 0;
        }
    }
}

bool getButton() {
    if (!inputoutput_inited) initIO();
    if (PUSH_BUTTON_OUT != -1) {
        pinMode(PUSH_BUTTON_OUT, OUTPUT);
        digitalWrite(PUSH_BUTTON_OUT, HIGH);
    }
    bool button = digitalRead(PUSH_BUTTON_IN);
    return button;
}
