#include "touch.h"
#include "i2c.h"
#include "pinout.h"
#include "interrupt.h"
#include "events.h"

static bool touch_inited = false;
uint8_t touch_dev_addr = 0x15;

touch_data_t touch_data;

static bool touchDown = false;
static long touchDownTime = 0;
static int touchGesture = TOUCH_NO_GESTURE;
static long lastGestureConsumed = 0;

void initTouch() {
    if (!touch_inited) {
        touch_inited = true;
        pinMode(TP_RESET, OUTPUT);
        pinMode(TP_INT, INPUT);

        digitalWrite(TP_RESET, HIGH );
        delay(50);
        digitalWrite(TP_RESET, LOW);
        delay(5);
        digitalWrite(TP_RESET, HIGH );
        delay(50);

        userI2CRead(touch_dev_addr, 0x15, &touch_data.version15, 1);
        delay(5);
        userI2CRead(touch_dev_addr, 0xA7, touch_data.versionInfo, 3);
    }

    clearTouch();
}

bool touch_sleep = false;
void touchEnable(bool state) {
    digitalWrite(TP_RESET, LOW);
    delay(5);
    digitalWrite(TP_RESET, HIGH );
    delay(50);
    touch_sleep = !state;
    if (touch_sleep) {
        byte standby_value = 0x03;
        userI2CWrite(touch_dev_addr, 0xA5, &standby_value, 1);
    }

    clearTouch();
}

void readTouch()
{
    if (touch_sleep) return;

    if (getInterrupts()->touch) {
        readTouchData();
        touchDownTime = millis();
        if (!touchDown && (touch_data.x != touch_data.lastX ||
                           touch_data.y != touch_data.lastY)) {
            addEvent(E_TOUCH_PRESSED, touch_data.x, touch_data.y);
            touchDown = true;
            touchGesture = TOUCH_NO_GESTURE;
        }
        getInterrupts()->touch = 0;
    }

    long touchTime = millis() - touchDownTime;
    if (touchDown && touchTime > 250) {
        addEvent(E_TOUCH_RELEASED, touch_data.x, touch_data.y);
        touchDown = false;
        touchGesture = TOUCH_NO_GESTURE;
    }

    if (touchGesture != touch_data.gesture) {
        if (millis() - lastGestureConsumed > 1000) {
            addEvent(E_TOUCH_GESTURE, touch_data.x, touch_data.y, touch_data.gesture);
            touch_data.gesture = TOUCH_NO_GESTURE;
            touchGesture = touch_data.gesture;
            lastGestureConsumed = millis();
        }
    }

    touch_data.down = touchDown;

}

void readTouchData() {
    byte data_raw[8];
    userI2CRead(touch_dev_addr, 0x01, data_raw, 6);

    touch_data.gesture = data_raw[0];
    touch_data.touchpoints = data_raw[1];
    touch_data.event = data_raw[2] >> 6;
    touch_data.x = data_raw[3];
    touch_data.y = data_raw[5];

    if (touch_data.gesture != TOUCH_NO_GESTURE) {
        touch_data.lastGesture = touch_data.gesture;
    }
    if (touch_data.x != 255 || touch_data.y != 255) {
        touch_data.lastX = touch_data.x;
        touch_data.lastY = touch_data.y;
    } else {
        touch_data.x = touch_data.lastX;
        touch_data.y = touch_data.lastY;
    }
}

void clearTouch()
{
    touch_data.down = false;
    touchDown = false;
    touchGesture = TOUCH_NO_GESTURE;
    touchDownTime = 0;
}

touch_data_t* getTouch()
{
    return &touch_data;
}