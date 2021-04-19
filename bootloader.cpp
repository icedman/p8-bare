#include "bootloader.h"
#include "sleep.h"

#include <nrf_nvic.h>//interrupt controller stuff
#include <nrf_sdm.h>
#include <nrf_soc.h>

void startBootloader(bool without_sd) {
    disableHardware();
    if (without_sd) {
        NRF_POWER->GPREGRET = 0x01;
        NVIC_SystemReset();
    } else {
        sd_power_gpregret_set(0x01);
        sd_nvic_SystemReset();
    }
    sd_nvic_SystemReset();
    while (1) {};
}

void systemReboot() {
    disableHardware();
    sd_nvic_SystemReset();
    while (1) {};
}

void systemOff()
{}