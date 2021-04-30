/*
 * Copyright (c) 2020 Aaron Christophel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "display.h"
#include "fastSpi.h"
#include "asteroidFont.h"

uint8_t lcdBuffer[LCD_BUFFER_SIZE + 4];
uint32_t windowArea = 0;
uint32_t windowWidth = 0;
uint32_t windowHeight = 0;

void spiCommand(uint8_t d);

uint8_t* getLCDBuffer()
{
    return lcdBuffer;
}

void displayEnable(bool state) {
    uint8_t temp[2];
    startWriteDisplay();
    if (state) {
        spiCommand(ST77XX_DISPON);
        spiCommand(ST77XX_SLPOUT);
    } else {
        spiCommand(ST77XX_SLPIN);
        spiCommand(ST77XX_DISPOFF);
    }
    endWriteDisplay();
}

void setAddrWindowDisplay(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    uint8_t temp[4];
    //y += 80; // when rotated screen
    spiCommand(0x2A);
    temp[0] = (x >> 8);
    temp[1] = x;
    temp[2] = ((x + w - 1) >> 8);
    temp[3] = (x + w - 1);
    writeFastSpi(temp, 4);
    spiCommand(0x2B);
    temp[0] = (y >> 8 );
    temp[1] = y;
    temp[2] = ((y + h - 1) >> 8);
    temp[3] = ((y + h - 1) & 0xFF);
    writeFastSpi(temp, 4);
    spiCommand(0x2C);
}

void initDisplay() {
    uint8_t temp[25];
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_RS, OUTPUT);
    pinMode(LCD_RESET, OUTPUT);
    pinMode(LCD_DET, OUTPUT);

    digitalWrite(LCD_CS, HIGH);
    digitalWrite(LCD_RS, HIGH);

    digitalWrite(LCD_RESET, HIGH);
    delay(20);
    digitalWrite(LCD_RESET, LOW);
    delay(100);
    digitalWrite(LCD_RESET, HIGH);
    delay(100);
    startWriteDisplay();
    spiCommand(54);
    temp[0] = 0x00;//0xC0 when rotated screen
    writeFastSpi(temp, 1);
    spiCommand(58);
    temp[0] = 5;
    writeFastSpi(temp, 1);
    spiCommand(178);
    temp[0] = 12;
    temp[1] = 12;
    temp[2] = 0;
    temp[3] = 51;
    temp[4] = 51;
    writeFastSpi(temp, 5);
    spiCommand(183);
    temp[0] = 53;
    writeFastSpi(temp, 1);
    spiCommand(187);
    temp[0] = 25;
    writeFastSpi(temp, 1);
    spiCommand(192);
    temp[0] = 44;
    writeFastSpi(temp, 1);
    spiCommand(194);
    temp[0] = 1;
    writeFastSpi(temp, 1);
    spiCommand(195);
    temp[0] = 18;
    writeFastSpi(temp, 1);
    spiCommand(196);
    temp[0] = 32;
    writeFastSpi(temp, 1);
    spiCommand(198);
    temp[0] = 15;
    writeFastSpi(temp, 1);
    spiCommand(208);
    temp[0] = 164;
    temp[1] = 161;
    writeFastSpi(temp, 2);
    spiCommand(224);
    temp[0] = 208;
    temp[1] = 4;
    temp[2] = 13;
    temp[3] = 17;
    temp[4] = 19;
    temp[5] = 43;
    temp[6] = 63;
    temp[7] = 84;
    temp[8] = 76;
    temp[9] = 24;
    temp[10] = 13;
    temp[11] = 11;
    temp[12] = 31;
    temp[13] = 35;
    writeFastSpi(temp, 14);
    spiCommand(225);
    temp[0] = 208;
    temp[1] = 4;
    temp[2] = 12;
    temp[3] = 17;
    temp[4] = 19;
    temp[5] = 44;
    temp[6] = 63;
    temp[7] = 68;
    temp[8] = 81;
    temp[9] = 47;
    temp[10] = 31;
    temp[11] = 31;
    temp[12] = 32;
    temp[13] = 35;
    writeFastSpi(temp, 14);
    spiCommand(33);
    spiCommand(17);
    delay(120);
    spiCommand(41);
    spiCommand(0x11);
    spiCommand(0x29);
    endWriteDisplay();
}

void spiCommand(uint8_t d) {
    digitalWrite(LCD_RS, LOW);
    writeFastSpi(&d, 1);
    digitalWrite(LCD_RS, HIGH);
}

void startWriteDisplay(void) {
    enableSpi(true);
    digitalWrite(LCD_CS, LOW);
}

void endWriteDisplay(void) {
    digitalWrite(LCD_CS, HIGH);
    enableSpi(false);
}

/*
  Draw a rect with origin x,y and width w, height h
*/
void drawFilledRect(coord pos, uint32_t w, uint32_t h, uint16_t colour) {
    startWriteDisplay();
    setDisplayWriteRegion({pos.x, pos.y}, w, h);
    spiCommand(0x2C);  //Memory write
    uint32_t numberOfBytesToWriteToLCD;
    uint32_t numberBytesInWindowArea = (windowArea * 2);
    uint32_t lcdBufferSize = LCD_BUFFER_SIZE;  //Size of LCD buffer
    //If we are comfortable that the number of bytes the current window will hold in a buffer is less than the max buffer size:
    if (numberBytesInWindowArea < lcdBufferSize)
        lcdBufferSize = numberBytesInWindowArea;  //Set the buffer size to be that of the window area * 2 (number of bytes that the window would occupy)

    for (int i = 0; i <= lcdBufferSize; i++) {  //Loop through buffer
        //Write every pixel (half-word) into the LCD buffer
        lcdBuffer[i++] = (colour >> 8) & 0xFF;  //Post increment meaning that it first writes to position i, then increments i
        lcdBuffer[i] = colour & 0xFF;           //Writes to the (now) position of i++
    }
    do {
        if (numberBytesInWindowArea >= LCD_BUFFER_SIZE)
            numberOfBytesToWriteToLCD = LCD_BUFFER_SIZE;
        else
            numberOfBytesToWriteToLCD = numberBytesInWindowArea;
        writeFastSpi(lcdBuffer, numberOfBytesToWriteToLCD);
        numberBytesInWindowArea -= numberOfBytesToWriteToLCD;
    } while (numberBytesInWindowArea > 0);
    endWriteDisplay();
}

/*
  Draw a rectangle with outline of width lineWidth
 */
void drawUnfilledRect(coord pos, uint32_t w, uint32_t h, uint8_t lineWidth, uint16_t colour) {
    drawFilledRect({pos.x, pos.y}, w, h, COLOUR_BLACK);  //Clear rect of stuff
    //Top rect
    drawFilledRect({pos.x, pos.y}, w, lineWidth, colour);
    //Bottom rect
    drawFilledRect({pos.x, pos.y + h - lineWidth}, w, lineWidth, colour);
    //Left rect
    drawFilledRect({pos.x, pos.y}, lineWidth, h, colour);
    //Right rect
    drawFilledRect({pos.x + w - lineWidth, pos.y}, lineWidth, h, colour);
}

/*
  Set the column and row RAM addresses for writing to the display
  You must select a region in the LCD RAM to write pixel data to
  This region has an xStart, xEnd, yStart and yEnd address
  As you write half-words (pixels) over SPI, the RAM fills horizontally per row
*/
void setDisplayWriteRegion(coord pos, uint32_t w, uint32_t h) {
    uint8_t buf[4];  //Parameter buffer
    windowHeight = h;
    windowWidth = w;
    windowArea = w * h;    //Calculate window area
    spiCommand(0x2A);  //Column address set
    buf[0] = 0x00;         //Padding write value to make it 16 bit
    buf[1] = pos.x;
    buf[2] = 0x00;
    buf[3] = (pos.x + w - 1);
    writeFastSpi(buf, 4);
    spiCommand(0x2B);  //Row address set
    buf[0] = 0x00;
    buf[1] = pos.y;
    buf[2] = 0x00;
    buf[3] = ((pos.y + h - 1) & 0xFF);
    writeFastSpi(buf, 4);
}

/*
  Clear display (clear whole display when no arg (or false) is passed in)
*/
void clearDisplay(uint16_t colour) {
    drawFilledRect({0, 0}, 240, 240, colour);
}

// line renderer
void drawPixel(framebuffer *buffer, int x,int y, int color)
{
    int idx = (x + y * buffer->width) * 2;
    lcdBuffer[idx++] = (color >> 8) & 0xFF;
    lcdBuffer[idx] = color & 0xFF;
}

void drawLine(framebuffer *buffer, int x,int y,int x2,int y2, int color)
{
    int lx=FPABS((x-x2));
    int ly=FPABS((y-y2));

    int l=lx>ly ? lx : ly;
    int fl=(l<<FPP);
    if ((fl>>FPP)==0) fl=FPONE;

    int sx=x<<FPP;
    int sy=y<<FPP;
    int xx=(x2-x)<<FPP;
    int yy=(y2-y)<<FPP;

    xx=FPDIV(xx,fl);
    yy=FPDIV(yy,fl);

    while(l>0) {
        coord pos;
        pos.x = sx >> FPP;
        pos.y = sy >> FPP;
        if (pos.x < buffer->width && pos.y < buffer->height) {
            int idx = (pos.x + pos.y * buffer->width) * 2;
            lcdBuffer[idx++] = (color >> 8) & 0xFF;
            lcdBuffer[idx] = color & 0xFF;
        }
        sx+=xx;
        sy+=yy;
        l--;
    }
}

static uint32_t nobtwtlcd;
void beginSprite(int x, int y, int w, int h, bool clear, int bg)
{
    startWriteDisplay();
    setDisplayWriteRegion({x, y}, w, h);
    nobtwtlcd = w * h * 2;

    if (nobtwtlcd >= LCD_BUFFER_SIZE)
        nobtwtlcd = LCD_BUFFER_SIZE;

    if (clear) {
        int ii = 0;
        for (int i = 0; i <= nobtwtlcd; i+=2) {
            lcdBuffer[ii++] = (bg >> 8) & 0xFF;
            lcdBuffer[ii++] = bg & 0xFF;
        }

    }

    spiCommand(0x2C);  //Memory write
}

void endSprite()
{
    writeFastSpi(lcdBuffer, nobtwtlcd);
    endWriteDisplay();
}

void padStringWithSpaces(char *tmp, int c) {
    int ln = strlen(tmp);
    for(int i=0; i<c; i++) {
        tmp[i + ln] = ' ';
        tmp[i + ln + 1] = 0;
    }
}

#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
#define FIRSTH 37 /* also prime */

uint32_t dataHash(const char* s)
{
    uint32_t h = FIRSTH;
    while (*s) {
        h = (h * A) ^ (s[0] * B);
        s++;
    }
    return h; // or return h % C;
}