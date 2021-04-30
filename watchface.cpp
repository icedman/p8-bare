#include "watchface.h"
#include "face.h"

#include "display.h"
#include "time.h"
#include "ble.h"
#include "battery.h"

#define READ16(src, p) (((uint16_t)(src[p+1]) << 8) | (uint16_t)(src[p]))
#define READ16_(src, p) (((uint16_t)(src[p]) << 8) | (uint16_t)(src[p+1]))
#define READ32(src, p) (((uint32_t)(src[p+3]) << 24) | ((uint32_t)(src[p+2]) << 16) | \
                    ((uint32_t)(src[p+1]) << 8) | (uint32_t)(src[p]))

#define EntitiesOffset 11
#define SpriteOffsetsOffset 200
#define SpriteSizesOffset 1200
#define SpriteDataOffset 1700

#define MAX_SPRITES 64

static bool hasBg = false;
static uint8_t *currentFace = 0;

static watchface_gfx_spr_t watchface_spr[64];
static watchface_gfx_t watchface_gfx[] = {
    { 0x10, "MonthName" },
    { 0x11, "Month" },
    { 0x12, "YearLeft" },
    { 0x30, "Day" },
    { 0x60, "DayOfWeek" },
    { 0x61, "DayOfWeekCZ" },
    { 0x40, "HoursDigit1" },
    { 0x41, "HoursDigit2" },
    { 0x43, "MinutesDigit1" },
    { 0x44, "MinutesDigit2" },
    { 0x45, "PMIcon" },
    { 0x46, "AMIcon" },
    { 0x70, "StepsProgress" },
    { 0x71, "StepsIcon" },
    { 0x72, "StepsLeft", 0 },
    { 0x73, "StepsCentered", 1 },
    { 0x74, "StepsRight", 2 },
    { 0x76, "StepsGoal" },
    { 0x80, "HeartProgress" },
    { 0x81, "HeartIcon" },
    { 0x82, "HeartLeft", 0 },
    { 0x83, "HeartCentered", 1 },
    { 0x84, "HeartRight", 2 },
    { 0x90, "CaloriesProgress" },
    { 0x91, "CaloriesIcon" },
    { 0x92, "CaloriesLeft", 0 },
    { 0x93, "CaloriesCentered", 1 },
    { 0x94, "CaloriesRight", 2 },
    { 0xa0, "DistanceProgress" },
    { 0xa1, "DistanceIcon" },
    { 0xa2, "DistanceLeft", 0 },
    { 0xa3, "DistanceCentered", 1 },
    { 0xa4, "DistanceRight", 2 },
    { 0xb1, "SleepIcon" },
    { 0xb2, "SleepHours" },
    { 0xb4, "SleepMinutes" },
    { 0xd1, "BatteryIcon" },
    { 0xd2, "BatPercentLeft", 0 },
    { 0xd3, "BatPercentCentered", 1 },
    { 0xd4, "BatPercentRight", 2 },
    { 0xc0, "BLEIcon" },
    { 0xf0, "Icon" },
    { 0x00, "" }
};

void initWatchface(void)
{
    currentFace = (uint8_t*)face;

    for(int j=0; ; j++) {
        watchface_gfx_t *g = &watchface_gfx[j];
        if (!g->type) break;
        g->sprite = 0;
    }

    const uint8_t *ptr = currentFace;

    uint8_t header = ptr[0];
    uint8_t entityCount = ptr[1];
    uint8_t spriteCount = ptr[2];
    uint16_t id = READ16(ptr, 3);
    uint8_t bg = ptr[5];
    uint8_t bg_width = ptr[8];
    uint8_t bg_height = ptr[9];

    hasBg = (bg == 0);

    // printf("header: %d\n", header);
    // printf("entityCount: %d\n", entityCount);
    // printf("spriteCount: %d\n", spriteCount);
    // printf("id: %d\n", id);
    // printf("bg: %d\n", bg);
    // printf("bg_width: %d\n", bg_width);
    // printf("bg_height: %d\n", bg_height);

    for (int i = 0; i < entityCount; i++) {
        const uint8_t offset = EntitiesOffset + i * 6;
        const uint8_t type = ptr[offset];

        watchface_gfx_t *gfx = 0;

        for(int j=0; ; j++) {
            watchface_gfx_t *g = &watchface_gfx[j];
            if (!g->type) break;
            if (g->type == type) {
                gfx = g;
                break;
            }
        }

        if (!gfx) continue;

        gfx->type = type;
        gfx->x = ptr[offset + 1];
        gfx->y = ptr[offset + 2];
        gfx->w = ptr[offset + 3];
        gfx->h = ptr[offset + 4];
        gfx->sprite = ptr[offset + 5];

        if (gfx->sprite >= MAX_SPRITES) gfx->sprite = 0;
    }

    for (int i = 0; i < spriteCount && i < MAX_SPRITES; i++) {
        const uint32_t spriteOffset = SpriteDataOffset + READ32(ptr, SpriteOffsetsOffset + i * 4);
        const uint16_t spriteSize = READ16(ptr, SpriteSizesOffset + i * 2);
        const uint16_t spriteType = READ16_(ptr, spriteOffset);
        watchface_spr[i].offset = spriteOffset + 2;
        watchface_spr[i].size = spriteType == 0x0821 ? spriteSize - 2 : 0;
        // // printf("%d 0x%04x\n", i, spriteType);
    }
}

void draw_sprite(watchface_gfx_spr_t *s, int x, int y, int w, int h, bool deferred = false)
{
    beginSprite(x, y, w, h);
    uint8_t* buffer = (uint8_t*)getLCDBuffer();

    const uint8_t *ptr = currentFace;

    int ii = 0;
    int px = 0;
    int py = 0;
    for (uint32_t i = s->offset; i < s->offset + s->size; i += 3) {
        uint16_t pixel = READ16_(ptr, i);
        uint8_t sz = ptr[i+2];

        for(int j=0; j<sz; j++) {
            if (ii >= LCD_BUFFER_SIZE) break;
            buffer[ii++] = (pixel >> 8) & 0xFF;  //Post increment meaning that it first writes to position i, then increments i
            buffer[ii++] = pixel & 0xFF;
        }
    }

    if (deferred) {
        return;
    }

    endSprite();
}

int drawWatchface(int hs)
{
    time_data_t tm = getTime();
    uint8_t dofw = getDayOfWeek();
    bool conn = isBLEConnected();
    int bat = getBatteryPercent();

    char tmp[32];
    int idx = 0;
    tmp[idx++] = tm.hr;
    tmp[idx++] = tm.min;
    tmp[idx++] = dofw;
    tmp[idx++] = conn;
    tmp[idx++] = bat;
    tmp[idx++] = 0;

    int hash = dataHash(tmp);
    if (hash > 0 && hash == hs) {
        return hash;
    }

    // background
    if (hasBg) {
        for (int i = 0; i < 10; i++) {
            // draw_sprite(&watchface_spr[i], 0, 24*i, 240, 24);
        }
    }

    for(int j=0; ; j++) {
        watchface_gfx_t *g = &watchface_gfx[j];
        if (!g->type) break;
        if (!g->sprite) continue;

        int id = g->sprite;

        switch(g->type) {
        case 0x40:
            id = g->sprite + (tm.hr / 10) % 10;
            break;
        case 0x41:
            id = g->sprite + (tm.hr % 10);
            break;
        case 0x43:
            id = g->sprite + (tm.min / 10) % 10;
            break;
        case 0x44:
            id = g->sprite + (tm.min % 10);
            break;

        case 0x11: // month
            draw_sprite(&watchface_spr[id + ((tm.month / 10) % 10)], g->x, g->y, g->w, g->h);
            draw_sprite(&watchface_spr[id + (tm.month % 10)], g->x + g->w, g->y, g->w, g->h);
            continue;

        case 0x30: // day
            draw_sprite(&watchface_spr[id + ((tm.day / 10) % 10)], g->x, g->y, g->w, g->h);
            draw_sprite(&watchface_spr[id + (tm.day % 10)], g->x + g->w, g->y, g->w, g->h);
            continue;

        case 0x12: // year
        {
            int yt = ((tm.year / 1000) % 10);
            int yh = (((tm.year - yt * 1000) / 100) % 10);
            int yx = (((tm.year - yt * 1000 - yh * 100) / 10) % 10);
            int yl = (((tm.year - yt * 1000 - yh * 100 - yx * 10)) % 10);
            int xx = g->x;

            if (g->align == 0) {
                xx -= g->w + 2;
                xx -= ((g->w + 1) * 4)/2;
            }

            draw_sprite(&watchface_spr[id + yt], xx + (g->w + 1)*1, g->y, g->w, g->h);
            draw_sprite(&watchface_spr[id + yh], xx + (g->w + 1)*2, g->y, g->w, g->h);
            draw_sprite(&watchface_spr[id + yx], xx + (g->w + 1)*3, g->y, g->w, g->h);
            draw_sprite(&watchface_spr[id + yl], xx + (g->w + 1)*4, g->y, g->w, g->h);
        }
        continue;

        case 0x60:
        case 0x61:
            id = g->sprite + (dofw % 7);
            break;

        case 0xd4:
            // id = g->sprite + (10 * (bat / 100));
            break;
        case 0xd1: // battery icon
            // id = g->sprite;
            // id = g->sprite + (bat / 100);
            break;
        case 0xc0:
            if (!conn) {
                id = 0;
                drawFilledRect({g->x, g->y}, g->w, g->h, 0);

            }
            break;
        }

        if (id == 0)
            continue;

        draw_sprite(&watchface_spr[id], g->x, g->y, g->w, g->h);

        if (g->type == 0xd1 && bat < 95) {
            int w = ((g->w - 5) * (100 - bat) / 100);
            int x = g->x + g->w - w - 3;
            drawFilledRect({x, g->y+2}, w, g->h-4, 0);
        }
    }

    return hash;
}

watchface_gfx_spr_t* getWatchSprites() {
    return watchface_spr;
}

watchface_gfx_t* getWatchEntities() {
    return watchface_gfx;
}