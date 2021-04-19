/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"

void initBacklight();
void setBacklight();
int getBacklight();
void setBacklight(int brightness);
void restoreBacklight();
void incBacklight();
void decBacklight();
