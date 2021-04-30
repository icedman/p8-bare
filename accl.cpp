/*
   Copyright (c) 2020 Aaron Christophel

   SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "accl.h"
#include "Arduino.h"
#include "i2c.h"
#include "pinout.h"
#include "watchdog.h"
#include "interrupt.h"
#include "io.h"
#include "ble.h"
#include "sleep.h"

struct accl_data_struct accl_data;

#define LOW_POWER
#include "lis3dh-motion-detection.h"
// #include "Wire.h"

uint16_t sampleRate = 10;  // HZ - Samples per second - 1, 10, 25, 50, 100, 200, 400, 1600, 5000
uint8_t accelRange = 2;   // Accelerometer range = 2, 4, 8, 16g

LIS3DH myIMU(0x18); //Default address is 0x19.

uint16_t errorsAndWarnings = 0;

void init_accl();
void accl_config_read_write(bool rw, uint8_t addr, uint8_t *data, uint32_t len, uint32_t offset);
void reset_accl();
void reset_step_counter();
uint32_t read_step_data();
bool acc_input();
bool get_is_looked_at();
accl_data_struct get_accl_data();
void update_accl_data();
void accl_write_reg(uint8_t reg, uint8_t data);
uint8_t accl_read_reg(uint8_t reg);


void init_accl() {

    if( myIMU.begin(sampleRate, 1, 1, 1, accelRange) != 0 )
    {
        // Serial.print("Failed to initialize IMU.\n");
    }

    //Detection threshold can be from 1 to 127 and depends on the Range
    //chosen above, change it and test accordingly to your application
    //Duration = timeDur x Seconds / sampleRate
    myIMU.intConf(INT_1, DET_MOVE, 13, 2);
    myIMU.intConf(INT_2, DET_STOP, 13, 10, 1);  // also change the polarity to active-low, this will change both Interrupts behavior

    uint8_t readData = 0;

    // Confirm configuration:
    myIMU.readRegister(&readData, LIS3DH_INT1_CFG);
    myIMU.readRegister(&readData, LIS3DH_INT2_CFG);

    // Get the ID:
    // myIMU.readRegister(&readData, LIS3DH_WHO_AM_I);
    myIMU.readRegister(&readData, 0x11);
}

void accl_config_read_write(bool rw, uint8_t addr, uint8_t *data, uint32_t len, uint32_t offset)
{
}

void reset_accl() {
}

void reset_step_counter() {
    accl_data.steps = 0;
}

uint32_t read_step_data() {
    return accl_data.steps;
}

int last_y_acc = 0;
bool acc_input() {
    // update_accl_data();

    if ((accl_data.x + 335) <= 670 && accl_data.z < 0) {
        if (!isSleeping()) {
            if (accl_data.y <= 0) {
                return false;
            } else {
                last_y_acc = 0;
                return false;
            }
        }
        if (accl_data.y >= 0) {
            last_y_acc = 0;
            return false;
        }
        if (accl_data.y + 230 < last_y_acc) {
            last_y_acc = accl_data.y;
            return true;
        }
    }
    return false;
}

bool get_is_looked_at() {
//   update_accl_data();
    if ((accl_data.y + 300) <= 600 && (accl_data.x + 300) <= 600 && accl_data.z < 100)
        return true;
    return false;
}

accl_data_struct get_accl_data() {
    // update_accl_data();
    return accl_data;
}

void update_accl_data() {
    int16_t dataHighres = 0;

    if( myIMU.readRegisterInt16( &dataHighres, LIS3DH_OUT_X_L ) != 0 )
    {
        errorsAndWarnings++;
    }
    accl_data.y = (dataHighres / 0x10);

    if( myIMU.readRegisterInt16( &dataHighres, LIS3DH_OUT_Y_L ) != 0 )
    {
        errorsAndWarnings++;
    }
    accl_data.x = (dataHighres / 0x10);

    if( myIMU.readRegisterInt16( &dataHighres, LIS3DH_OUT_Z_L ) != 0 )
    {
        errorsAndWarnings++;
    }
    accl_data.z = (dataHighres / 0x10);

}

void accl_write_reg(uint8_t reg, uint8_t data) {
}

uint8_t accl_read_reg(uint8_t reg) {
    return 0;
}

void initAccel()
{
    init_accl();
}

void readAccel()
{
    // if (getInterrupts()->accel) {
        update_accl_data();
    // }
}

accl_data_struct getAccelData()
{
    return get_accl_data();
}