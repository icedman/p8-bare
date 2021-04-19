#include "touch.h"
#include "i2c.h"
#include "pinout.h"
#include "interrupt.h"
#include "events.h"

static bool touch_inited = false;
uint8_t touch_dev_addr = 0x15;

touch_data_t touch_data;

static int touchDebounce = 0;
static bool touchDown = false;
static long touchDownTime = 0;
static int previousGesture = TOUCH_NO_GESTURE;

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
}

void readTouch()
{
    if (touch_sleep) return;

    bool prevTouch = touchDown;
    touchDown = getInterrupts()->touch;

    touch_data_t *td = 0;
    if (touchDown) {
        readTouchXY();
        td = getTouch();
        if (previousGesture != td->gesture) {
            previousGesture = td->gesture;
            addEvent(E_TOUCH_GESTURE, td->x, td->y, td->gesture);
        }
    }

    if (prevTouch != touchDown && touchDown) {
        if (touchDebounce == 0) {
            addEvent(E_TOUCH_PRESSED, td->x, td->y);
            touchDownTime = millis();
        }
        touchDebounce = 4;
    } else {

        if (touchDownTime > 0 && millis() - touchDownTime > 5000) {
            addEvent(E_TOUCH_LONG_PRESS, td->x, td->y);
            touchDownTime = 0;
        }
    }

    if (touchDebounce > 0) {
        if (touchDebounce-- < 2) {
            addEvent(E_TOUCH_RELEASED, td->x, td->y);
            touchDebounce = 0;
            previousGesture = TOUCH_NO_GESTURE;
        }
    }
}

void readTouchXY() {
    byte data_raw[8];
    userI2CRead(touch_dev_addr, 0x01, data_raw, 6);

    touch_data.gesture = data_raw[0];
    touch_data.touchpoints = data_raw[1];
    touch_data.event = data_raw[2] >> 6;
    touch_data.x = data_raw[3];
    touch_data.y = data_raw[5];

    if (touch_data.x == 255 && touch_data.y == 255) {
        touch_data.x = touch_data.last_x;
        touch_data.y = touch_data.last_y;
    } else {
        touch_data.last_x = touch_data.x;
        touch_data.last_y = touch_data.y;
    }

}

touch_data_t* getTouch()
{
    return &touch_data;
}