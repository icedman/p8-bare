/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "Arduino.h"
#include "pinout.h"
// #include "asteroidsfont.h"

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29

/*
  This structure is used for positions
 */
typedef struct {
    uint8_t x, y;
} coord;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t pitch;
    uint8_t *pixels;
} framebuffer;

typedef long fixed;

#define LCD_BUFFER_SIZE 16000 

#define FPMUL(x,y)      ((((x)>>6)*((y)>>6))>>4)    // multiply fixed by fixed. returns fixed
#define FPMULH(x,y)     ((((x)>>2)*((y)>>2))>>12)   // higher precision
#define FPDIV(x,y)      ((((x)<<6)/((y)>>6))<<4)    // divide fixed by fixed. returns fixed
#define FPONE           65536
#define FPP             16
#define FPI(x)          ((x)<<FPP)                  // convert int to fixed
#define FPFL(x)         ((int)(x * FPONE))          // convert float to fixed
#define FPABS(n)        (n - ((n+n) & (n>>31)))
#define FEPSILON        1                           // near zero fixed

#define COLOUR_WHITE    0b1111111111111111
#define COLOUR_RED      0b1111100000000000
#define COLOUR_GREEN    0b0000011111100000
#define COLOUR_BLUE     0b0000000000011111
#define COLOUR_YELLOW   0b1111111111100000
#define COLOUR_ORANGE   0b1111101111100000
#define COLOUR_CYAN     0b0000011111111111
#define COLOUR_MAGENTA  0b1111100000011111
#define COLOUR_BLACK    0b0000000000000000

void displayEnable(bool state);

void setAddrWindowDisplay(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void initDisplay();
void startWriteDisplay(void);
void endWriteDisplay(void);

void setDisplayWriteRegion(coord pos, uint32_t w, uint32_t h);
void drawFilledRect(coord pos, uint32_t w, uint32_t h, uint16_t colour);
void clearDisplay(uint16_t colour = 0);
void drawUnfilledRect(coord pos, uint32_t w, uint32_t h, uint8_t lineWidth, uint16_t colour);

void drawPixel(framebuffer *buffer, int x,int y, int color);
void drawLine(framebuffer *buffer, int x,int y,int x2,int y2, int color);

void beginSprite(int x, int y, int w, int h, bool clear=false, int colorBg=0);
void endSprite();
uint8_t* getLCDBuffer();

// utility
void padStringWithSpaces(char *tmp, int c);
uint32_t dataHash(const char *s);