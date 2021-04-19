/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <Arduino.h>

void initFastSpi();
void enableSpi(bool state);
void enableWorkaround(NRF_SPIM_Type *spim, uint32_t ppi_channel, uint32_t gpiote_channel);
void disableWorkaround(NRF_SPIM_Type *spim, uint32_t ppi_channel, uint32_t gpiote_channel);
void writeFastSpi(const uint8_t *ptr, uint32_t len);
void readFastSpi(uint8_t *ptr, uint32_t len);
