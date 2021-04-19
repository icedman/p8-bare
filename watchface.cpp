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

static uint32_t prevHash  = 0;
void drawWatchface(void)
{
    time_data_t tm = getTime();
    uint8_t dofw = getDayOfWeek();

    int conn = isBLEConnected();
    int bat = getBatteryPercent();
    int ii = 0;
    char tmp[32];
    tmp[ii++] = tm.hr;
    tmp[ii++] = tm.min;
    tmp[ii++] = dofw;
    tmp[ii++] = conn;
    tmp[ii++] = bat;
    tmp[ii++] = 0;

    // uint32_t hs = dataHash(tmp);
    // if (hs == prevHash) return;
    // prevHash = hs;
  
    // background
    if (hasBg) {
        for (int i = 0; i < 10; i++) {
            // draw_sprite(&watchface_spr[i], 0, 24*i, 240, 24);
        }
    }

    for(int j=0; ;j++) {
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
            id = g->sprite;
            // id = g->sprite + (bat / 100);
            break;
        case 0xc0:
            if (!conn) {
                id = 0;
                coord pos;
                pos.x = g->x;
                pos.y = g->y;
                drawFilledRect(pos, g->w, g->h, 0);

            }
            break;
        }

        if (id == 0)
            continue;

        if (g->type == 0xd1 && bat < 90) {
            draw_sprite(&watchface_spr[id], g->x, g->y, g->w, g->h);
            int p = 2;
            int ww = (g->w - 4) * bat / 100;
            coord pos;
            pos.x = ww + p;
            pos.y = p;
            ww = g->w - ww - (p*2) - 1;
            if (ww > 4) {
                int hh = g->h-(p*2);
                int clr = 0;
                drawFilledRect(pos, ww, hh, 0);

                // uint8_t* buffer = (uint8_t*)getLCDBuffer();
                // for(int j=0; j<hh; j++) {
                //     int sy = j * g->w + p;
                //     for(int k=0; k<ww; k++) {
                //         int sx = ww + p + k + 3;
                //         int idx = (sy + sx) * 2;
                //         buffer[idx] = (clr >> 8) & 0xFF;
                //         buffer[idx + 1] = clr & 0xFF;  
                //     }
                // }
            }
            // endSprite();
            return;
        }

        draw_sprite(&watchface_spr[id], g->x, g->y, g->w, g->h);
    }

}

void clearWatchface()
{
    prevHash = 0;
}