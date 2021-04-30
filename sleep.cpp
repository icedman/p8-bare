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
static bool pendingSleep = false;

void initRTC2();

void initSleep()
{
    // no need?
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    caffeinate();

    initRTC2();
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
    pendingSleep = false;
    setBacklight(0);
    displayEnable(false);
}

void wakeUp()
{
    sleeping = false;
    pendingSleep = false;
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
        if (!pendingSleep && millis() - lastaction > SLEEP_TIME) {
            pendingSleep = true;
        }
    }
}

bool shouldSleep()
{
    return pendingSleep;
}

bool isSleeping()
{
    return sleeping;
}

volatile bool shot;

bool onTwoSeconds() {
    bool temp = shot;
    shot = false;
    return temp;
}

#define LF_FREQUENCY 32768UL
#define SECONDS(x) ((uint32_t)((LF_FREQUENCY * x) + 0.5))
// #define wakeUpSeconds 0.040
#define wakeUpSeconds 2.0

void initRTC2() {

    NVIC_ClearPendingIRQ(RTC2_IRQn);
    NVIC_SetPriority(RTC2_IRQn, 15);
    NVIC_EnableIRQ(RTC2_IRQn);

    NRF_RTC2->PRESCALER = 0;
    NRF_RTC2->CC[0] = SECONDS(wakeUpSeconds);
    NRF_RTC2->INTENSET = RTC_EVTENSET_COMPARE0_Enabled << RTC_EVTENSET_COMPARE0_Pos;
    NRF_RTC2->EVTENSET = RTC_INTENSET_COMPARE0_Enabled << RTC_INTENSET_COMPARE0_Pos;
    NRF_RTC2->TASKS_START = 1;
}

#ifdef __cplusplus
extern "C" {
#endif
void RTC2_IRQHandler(void)
{
    volatile uint32_t dummy;
    if (NRF_RTC2->EVENTS_COMPARE[0] == 1)
    {
        NRF_RTC2->EVENTS_COMPARE[0] = 0;
        NRF_RTC2->CC[0] = NRF_RTC2->COUNTER +  SECONDS(wakeUpSeconds);
        dummy = NRF_RTC2->EVENTS_COMPARE[0];
        dummy;
        shot = true;
        // if (!sleep_sleeping)inc_tick();
        // check_inputoutput_times();
        // if (!get_i2cReading()) get_heartrate_ms();
    }
}
#ifdef __cplusplus
}
#endif
