/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <Arduino.h>
#include <nrf.h>
#include <Stream.h>
#include <RingBuffer.h>

void initI2C();

void setI2CReading(bool state);
bool getI2CReading();
uint8_t userI2CRead(uint8_t addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t length);
uint8_t userI2CWrite(uint8_t addr, uint8_t reg_addr, const uint8_t *reg_data, uint32_t length);

void WireBegin();
void WireBeginTransmission(uint8_t);
uint8_t WireEndTransmission(bool stopBit = true);
uint8_t WireRequestFrom(uint8_t address, size_t quantity, bool stopBit = true);
void WireWrite(uint8_t data);
int WireRead(void);
bool WireAvailable();
