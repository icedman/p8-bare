#include "app.h"
#include "display.h"
#include "events.h"
#include "io.h"
#include "sleep.h"
#include "time.h"
#include "heart.h"
#include "accl.h"
#include "touch.h"
#include "battery.h"
#include "bootloader.h"
#include "interrupt.h"
#include "watchface.h"
#include "pedometer.h"
#include "asteroidFont.h"

static int currentAppIdx = 0;
static int previousAppIdx = -1;

void App::update()
{
    // handle events
    event *evts = getEvents();
    for(int i=0; i<MAX_EVENTS; i++) {
        if (evts[i].e == 0) break;
        switch (evts[i].e) {
            // case E_INTERRUPT_CHARGED: {
            //     showNotice("charged...");
            //     break;
            // }
            // case E_INTERRUPT_CHARGING: {
            //     showNotice("charging...");
            //     break;
            // }
            case E_BUTTON_PRESSED: {
                onButtonDown();
                break;
            }
            case E_BUTTON_RELEASED: {
                onButtonUp();
                break;
            }
            case E_TOUCH_PRESSED: {
                onTouchDown();
                break;
            }
            case E_TOUCH_RELEASED: {
                onTouchUp();
                break;
            }
            case E_TOUCH_GESTURE: {
                onTouchGesture(evts[i].z);
                break;
            }
            case E_BUTTON_LONG_PRESS: {
                systemReboot();
                break;
            }
            case E_STARTUP: {
                onMount();
                break;
            }
            case E_WAKE: {
                onWake();
                break;
            }
        }
    }

    onUpdate();
}

void App::draw()
{
    onDraw();
}

void App::onMount()
{
    hash = 0;
    clearDisplay();
    clearEvents();
    clearTouch();
}

void App::onTouchGesture(int gesture)
{
    switch(gesture) {
        case TOUCH_SLIDE_UP:
            nextApp();
        break;
        case TOUCH_SLIDE_DOWN:
            previousApp();
        break;
    }
}

class WatchApp : public App
{
public:
    void onDraw()
    {
        // clearDisplay();
        hash = drawWatchface(hash);
    }
};

class InfoApp : public App
{
public:

    int presses;
    int releases;
    int gestures;

    void onMount() {
        App::onMount();
        presses = 0;
        releases = 0;
        gestures = 0;
    }

    void onTouchDown() {
        presses++;
    }

    void onTouchUp() {
        releases++;
    }

    void onTouchGesture(int gesture) {
        App::onTouchGesture(gesture);
        gestures++;
    }

    void onDraw()
    {
        char tmp[32];
        int x = 24;
        int y = 24;
        // readTouchData();

        touch_data_t *td = getTouch(); 
        int bat = getBatteryPercent();
        int heart = 0;
        int steps = 0;

        // clearDisplay();

        getTimeWithSecs(tmp);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24 * 2;

        getDay(tmp);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        getDate(tmp);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        sprintf(tmp, "power:%d", bat);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        sprintf(tmp, "touch:v%d.%d.%d",
            td->versionInfo[0],
            td->versionInfo[1],
            td->versionInfo[2]);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        sprintf(tmp, "%d %d %d", td->x, td->y, gestures);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        // sprintf(tmp, "heart: %d",heart);
        // padStringWithSpaces(tmp, 4);
        // asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        // y += 24;

        // sprintf(tmp, "steps: %d",steps);
        // padStringWithSpaces(tmp, 4);
        // asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        // y += 24;

        // asteroidDrawChar(32, 80, 'X', 4, COLOUR_RED);
        // asteroidDrawString(32, 80, "HELLO", 3, COLOUR_RED);
    }
};

class InfoFaceApp : public App
{
public:
    void onDraw() {
        char tmp[32];
        int x = 24;
        int y = 24;

        // sprintf(tmp, "%d", 1234567890);
        // asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        // y += 24;

        for(int j=0; ;j++) {
            watchface_gfx_t *g = &getWatchEntities()[j];
            if (!g->type) break;
            if (!g->sprite) continue;

            // sprintf(tmp, "%d", g->type);
            sprintf(tmp, "%s", g->description);
            asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
            y += 24;

        }
    }
};


class NoticeApp : public App
{
public:

    char *text;

    void onMount() {
        App::onMount();
    }

    void onTouchDown() {
        homeApp();
    }

    void onDraw()
    {
        int x = 24;
        int y = 24;
        asteroidDrawString(x, y, text, 1, COLOUR_WHITE, COLOUR_BLACK, false);
    }
};

class HeartApp : public App
{
public:
    void onDraw() {
        updateHeart();
        
        char tmp[32];
        int x = 24;
        int y = 24;
        sprintf(tmp, "heart:%d", getHeart());
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        accl_data_struct accl = getAccelData();
        sprintf(tmp, "steps:%d", getSteps());
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        sprintf(tmp, "x:%d", accl.x);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;
        sprintf(tmp, "y:%d", accl.y);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;
        sprintf(tmp, "z:%d", accl.z);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;
    }

    void onMount() {
        App::onMount();
        startHeart();
    }

    void onUnmount() {
        endHeart();
    }
};

WatchApp watchApp;
InfoApp infoApp;
// HeartApp heartApp;
InfoFaceApp infoFaceApp;
NoticeApp noticeApp;

App* currentApp = 0;
App* unmountApp = 0;

App* apps[] = {
    &watchApp,
    // &heartApp,
    &infoApp,
    &infoFaceApp,
    0
};

void showNotice(char *notice)
{
    currentApp = &noticeApp;
}

void homeApp()
{
    currentAppIdx = 0;
    currentApp = apps[currentAppIdx];
}

void previousApp()
{
    unmountApp = currentApp;
    if (currentAppIdx > 0) currentAppIdx--;
    currentApp = apps[currentAppIdx];
}

void nextApp()
{
    unmountApp = currentApp;
    if (currentAppIdx < (sizeof(apps)/sizeof(App))) currentAppIdx++;
    currentApp = apps[currentAppIdx];
}

void initApp()
{
    homeApp();
}

void updateApp()
{
    if (previousAppIdx != currentAppIdx) {
        previousAppIdx = currentAppIdx;
        if (unmountApp) {
            unmountApp->onUnmount();
        }
        currentApp->onMount();
    }
    currentApp->update();
}

void drawApp()
{
    if (previousAppIdx != currentAppIdx) return;
    currentApp->draw();
}