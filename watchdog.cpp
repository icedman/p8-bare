#include "watchdog.h"

#include <nrf_nvic.h>//interrupt controller stuff
#include <nrf_sdm.h>
#include <nrf_soc.h>

#define WATCHDOG_TIMEOUT 5000

void initWatchdog()
{
    NRF_WDT->CONFIG = NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos);
    NRF_WDT->CRV = (32768 * WATCHDOG_TIMEOUT) / 1000;
    NRF_WDT->RREN |= WDT_RREN_RR0_Msk;
    NRF_WDT->TASKS_START = 1;
}

void feedWatchdog()
{
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}