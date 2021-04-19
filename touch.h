#pragma once

#include <Arduino.h>

#define TOUCH_INTERRUPT_NOT_AVAILABLE

#define TOUCH_NO_GESTURE 0x00
#define TOUCH_SLIDE_DOWN 0x01
#define TOUCH_SLIDE_UP 0x02
#define TOUCH_SLIDE_LEFT 0x03
#define TOUCH_SLIDE_RIGHT 0x04
#define TOUCH_SINGLE_CLICK 0x05
#define TOUCH_DOUBLE_CLICK 0x0B
#define TOUCH_LONG_PRESS 0x0C

typedef struct {
    byte gesture;
    byte touchpoints;
    byte event;
    int x;
    int y;
    int last_x;
    int last_y;
    uint8_t version15;
    uint8_t versionInfo[3];
} touch_data_t;

void initTouch();
void touchEnable(bool enable);
void readTouch();
void readTouchXY();
touch_data_t* getTouch();

