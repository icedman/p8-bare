#pragma once

#include <Arduino.h>
#include <TimeLib.h>

typedef struct {
    int year;
    int month;
    int day;
    int hr;
    int min;
    int sec;
} time_data_t;

void initTime();
time_data_t getTime();
void getTimeWithSecs(char* str);
void getTime(char* str);
void getDate(char* str);
void getDay(char* str);
void setTimeWrapper(int yr, int mth, int day, int hr, int min, int sec);
// void getStopWatchTime(char* str, int startTime, int currentTime);
uint8_t getDayOfWeek(int d, int m, int y);
uint8_t getDayOfWeek();

void SetDateTimeString(String datetime);
String GetDateTimeString();