#include <Arduino.h>
#include <lp_timer.h>

#include "pinout.h"
#include "pedometer.h"
#include "watchdog.h"
#include "accl.h"
#include "heart.h"
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

static bool scheduleDraw = false;
const unsigned long DRAW_DELAY_TIME = 250;
const unsigned long EVENTS_DELAY_TIME = 100;
const unsigned long CHECKS_DELAY_TIME = 2000;

void handleDraw()
{
    if (isSleeping()) {
        return;
    }

    if (scheduleDraw) {
        drawApp();
        scheduleDraw = false;
    }
}

void handleEvents()
{
    updateSleep();

    if (shouldSleep()) {
        drawTimer.stop();
        goToSleep();
        return;
    }

    readIO();
    readTouch();
    updateApp();

    clearEvents();
    clearInterrupts();

    // scheduleDraw = true;

    // if (!drawTimer.isRunning()) {
    //     drawTimer.startTimer(DRAW_DELAY_TIME, handleDraw);
    // }
}

void handleChecks()
{
    readBattery();
}

void handleSleep()
{
    sleepWait();

    // waiting only to wake
    interrupt* ints = getInterrupts();
    if (ints->button + ints->touch + getTouch()->down > 0) {
        wakeUp();
        clearInterrupts();
    }
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

    clearDisplay();
    asteroidDrawString(20, 20, "booting...", 1, COLOUR_WHITE);

    initI2C();
    initTouch();
    initBattery();
    initTime();
    initSleep();
    initInterrupt();
    initBLE();
    // initHeart();
    // initAccel();
    // initPedometer();

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
    feedBLE();

    if (isSleeping()) {
        handleSleep();
    } else {
        // readTouch();
        // readAccel();
        // readPedometer();
        // sleep();
        handleEvents();
        handleChecks();
        drawApp();
    }
}