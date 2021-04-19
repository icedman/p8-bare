#include "sleep.h"
#include "display.h"
#include "backlight.h"
#include "touch.h"
#include "events.h"
#include "interrupt.h"

#include <nrf_nvic.h>//interrupt controller stuff
#include <nrf_sdm.h>
#include <nrf_soc.h>

static long sleeping = false;
static long lastaction = 0;
static long last_sleep_check = 0;

void initSleep()
{
    // no need?
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    caffeinate();
}

void disableHardware() {
    touchEnable(false);
    setBacklight(0);
    displayEnable(false);
    NRF_SAADC ->ENABLE = 0; //disable ADC
    NRF_PWM0  ->ENABLE = 0; //disable all pwm instance
    NRF_PWM1  ->ENABLE = 0;
    NRF_PWM2  ->ENABLE = 0;
}

void sleepWait() {
    sd_app_evt_wait();
    sd_nvic_ClearPendingIRQ(SD_EVT_IRQn);
}

void goToSleep()
{
    sleeping = true;
    setBacklight(0);
    displayEnable(false);
}

void wakeUp()
{
    sleeping = false;
    caffeinate();
    restoreBacklight();
    displayEnable(true);

    addEvent(E_WAKE);
}

void caffeinate()
{
    lastaction = millis();
    last_sleep_check = lastaction;
}

void updateSleep() {
    if (millis() - last_sleep_check > 300) {
        last_sleep_check = millis();
        bool temp_sleep = false;
        if (millis() - lastaction > SLEEP_TIME) {
            temp_sleep = true;
        }
        if (temp_sleep) {
            goToSleep();
        }
    }
}

bool isSleeping()
{
    return sleeping;
}