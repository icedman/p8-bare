#include <Arduino.h>
#include <lp_timer.h>

#include "pinout.h"
#include "watchdog.h"
#include "ble.h"
#include "fastSpi.h"
#include "i2c.h"
#include "interrupt.h"
#include "backlight.h"
#include "display.h"
#include "io.h"
#include "touch.h"
#include "events.h"
#include "sleep.h"
#include "time.h"
#include "battery.h"
#include "asteroidFont.h"
#include "watchface.h"

#include "app.h"

lp_timer drawTimer;
lp_timer eventsTimer;
lp_timer checksTimer;

const unsigned long DRAW_DELAY_TIME = 250;
const unsigned long EVENTS_DELAY_TIME = 100;
const unsigned long CHECKS_DELAY_TIME = 2500;

void handleDraw()
{
    if (isSleeping()) {
        return;
    }

    drawApp();
}

void handleEvents()
{
    feedBLE();
    readInterrupts();

    if (!isSleeping()) {
        updateSleep();
    } else {
        interrupt* ints = getInterrupts();
        if (ints->button + ints->touch > 0) {
            wakeUp();
        } else{
            return;
        }
    }

    feedWatchdog();
    readIO();
    readTouch();

    updateApp();

    clearEvents();
    clearInterrupts();
}

void handleChecks()
{
    readBattery();
}

void setup()
{
    delay(500);
    
    if (getButton()) {//if button is pressed on startup goto Bootloader
        NRF_POWER->GPREGRET = 0x01;
        NVIC_SystemReset();
    }

    // initWatchdog();
    initFastSpi();
    initBacklight();
    initDisplay();
    initIO();
    initAsteroids();

    setBacklight(3);    
    displayEnable(true);

    // show something
    clearDisplay();
    asteroidDrawString(20, 20, "booting...", 1, COLOUR_WHITE);

    initI2C();
    initTouch();
    initBattery();
    initTime();
    initSleep();
    initInterrupt();
    initBLE();

    initApp();
    initWatchface();

    touchEnable(true);
    handleEvents();

    drawTimer.startTimer(DRAW_DELAY_TIME, handleDraw);
    eventsTimer.startTimer(EVENTS_DELAY_TIME, handleEvents);
    checksTimer.startTimer(CHECKS_DELAY_TIME, handleChecks);
}

void loop()
{
    sleep();
}