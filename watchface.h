#pragma once

#include <Arduino.h>

typedef struct {
    uint8_t type;
    const char *description;

    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
    uint8_t align;
    uint8_t sprite;

} watchface_gfx_t;

typedef struct {
    uint32_t offset;
    uint16_t size;
} watchface_gfx_spr_t;

void initWatchface(void);
int drawWatchface(int hs);

watchface_gfx_spr_t* getWatchSprites();
watchface_gfx_t* getWatchEntities();