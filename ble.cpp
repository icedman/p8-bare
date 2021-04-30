/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ble.h"

#include <BLEPeripheral.h>

#include "pinout.h"
#include "sleep.h"
#include "time.h"
#include "battery.h"
#include "events.h"
#include "io.h"
#include "backlight.h"
#include "bootloader.h"

#define BLE_DEVICE_NAME "p8Icedman"

BLEPeripheral                   blePeripheral           = BLEPeripheral();
BLEService                      main_service     = BLEService("190A");
BLECharacteristic   TXchar        = BLECharacteristic("0002", BLENotify, 20);
BLECharacteristic   RXchar        = BLECharacteristic("0001", BLEWriteWithoutResponse, 20);

static bool ble_connected = false;

void ble_ConnectHandler(BLECentral& central);
void ble_DisconnectHandler(BLECentral& central);
void ble_DisconnectHandler(BLECentral& central);
void ble_written(BLECentral& central, BLECharacteristic& characteristic);
void ble_write(String Command);
void filterCmd(String Command);

void initBLE() {
    blePeripheral.setLocalName(BLE_DEVICE_NAME);
    blePeripheral.setAdvertisingInterval(500);
    blePeripheral.setDeviceName(BLE_DEVICE_NAME);
    blePeripheral.setAdvertisedServiceUuid(main_service.uuid());
    blePeripheral.addAttribute(main_service);
    blePeripheral.addAttribute(TXchar);
    blePeripheral.addAttribute(RXchar);
    RXchar.setEventHandler(BLEWritten, ble_written);
    blePeripheral.setEventHandler(BLEConnected, ble_ConnectHandler);
    blePeripheral.setEventHandler(BLEDisconnected, ble_DisconnectHandler);
    blePeripheral.begin();
    feedBLE();
}

void feedBLE() {
    blePeripheral.poll();
}

bool isBLEConnected()
{
    return ble_connected;
}

void ble_ConnectHandler(BLECentral& central) {
    ble_connected = true;
    wakeUp();
    addEvent(E_BLE_CONNECTED);
}

void ble_DisconnectHandler(BLECentral& central) {
    ble_connected = false;
    wakeUp();
    addEvent(E_BLE_DISCONNECTED);
}

String answer = "";
String tempCmd = "";
int tempLen = 0, tempLen1;
boolean syn;

void ble_written(BLECentral& central, BLECharacteristic& characteristic) {
    char remoteCharArray[22];
    tempLen1 = characteristic.valueLength();
    tempLen = tempLen + tempLen1;
    memset(remoteCharArray, 0, sizeof(remoteCharArray));
    memcpy(remoteCharArray, characteristic.value(), tempLen1);
    tempCmd = tempCmd + remoteCharArray;
    if (tempCmd[tempLen - 2] == '\r' && tempCmd[tempLen - 1] == '\n') {
        answer = tempCmd.substring(0, tempLen - 2);
        tempCmd = "";
        tempLen = 0;
        filterCmd(answer);
    }
}

void ble_write(String Command) {
    Command = Command + "\r\n";
    while (Command.length() > 0) {
        const char* TempSendCmd;
        String TempCommand = Command.substring(0, 20);
        TempSendCmd = &TempCommand[0];
        TXchar.setValue(TempSendCmd);
        Command = Command.substring(20);
    }
}

void filterCmd(String Command) {
    if (Command == "AT+BOND") {
        ble_write("AT+BOND:OK");
    } else if (Command == "AT+ACT") {
        ble_write("AT+ACT:0");
    } else if (Command.substring(0, 7) == "BT+UPGB") {
        startBootloader();
    } else if (Command.substring(0, 8) == "BT+RESET") {
        systemReboot();
    } else if (Command.substring(0, 8) == "BT+OFF") {
        systemOff();
    } else if (Command.substring(0, 7) == "AT+RUN=") {
        ble_write("AT+RUN:" + Command.substring(7));
    } else if (Command.substring(0, 8) == "AT+USER=") {
        ble_write("AT+USER:" + Command.substring(8));
    } else if (Command == "AT+PACE") {
        // accl_data_struct accl_data = get_accl_data();
        // ble_write("AT+PACE:" + String(accl_data.steps));
    } else if (Command == "AT+BATT") {
        ble_write("AT+BATT:" + String(getBatteryPercent()));
    } else if (Command.substring(0, 8) == "AT+PUSH=") {
        ble_write("AT+PUSH:OK");
        // show_push(Command.substring(8));
    } else if (Command == "BT+VER") {
        ble_write("BT+VER:P8");
    } else if (Command == "AT+VER") {
        ble_write("AT+VER:P8");
    } else if (Command == "AT+SN") {
        ble_write("AT+SN:P8");
    } else if (Command.substring(0, 12) == "AT+CONTRAST=") {
        String contrastTemp = Command.substring(12);
        if (contrastTemp == "100")
            setBacklight(1);
        else if (contrastTemp == "175")
            setBacklight(3);
        else setBacklight(7);
        ble_write("AT+CONTRAST:" + Command.substring(12));
    } else if (Command.substring(0, 10) == "AT+MOTOR=1") {
        String motor_power = Command.substring(10);
        // if (motor_power == "1")
        //     set_motor_power(50);
        // else if (motor_power == "2")
        //     set_motor_power(200);
        // else set_motor_power(350);
        // ble_write("AT+MOTOR:1" + Command.substring(10));
        // set_motor_ms();
    } else if (Command.substring(0, 6) == "AT+DT=") {
        SetDateTimeString(Command.substring(6));
        ble_write("AT+DT:" + GetDateTimeString());
    } else if (Command.substring(0, 5) == "AT+DT") {
        ble_write("AT+DT:" + GetDateTimeString());
    } else if (Command.substring(0, 8) == "AT+HTTP=") {
        // show_http(Command.substring(8));
    }
}
