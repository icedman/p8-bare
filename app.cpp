#include "app.h"
#include "display.h"
#include "events.h"
#include "io.h"
#include "sleep.h"
#include "time.h"
#include "touch.h"
#include "battery.h"
#include "bootloader.h"
#include "interrupt.h"
#include "watchface.h"
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
    void onMount() {
        App::onMount();
        clearWatchface();
    }

    void onWake() {
        clearWatchface();
    }

    void onDraw()
    {
        drawWatchface();
    }
};

class InfoApp : public App
{
public:

    void onDraw()
    {
        char tmp[32];

        readTouchXY();

        touch_data_t *td = getTouch(); 
        int bat = getBatteryPercent();

        int ii = 0;
        int heart = 0;
        int steps = 0;

        time_data_t time = getTime();
        tmp[ii++] = time.hr;
        tmp[ii++] = time.min;
        tmp[ii++] = time.sec;
        tmp[ii++] = time.day;
        tmp[ii++] = bat;
        tmp[ii++] = heart;
        tmp[ii++] = steps;
        tmp[ii++] = td->x;
        tmp[ii++] = td->y;
        tmp[ii++] = 0;

        uint32_t hs = dataHash(tmp);
        if (hs == hash) {
            // return;
        }
        hash = hs;

        // clearDisplay();

        int x = 24;
        int y = 24;
        y += 24 + 2;

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

        sprintf(tmp, "power: %d", bat);
        padStringWithSpaces(tmp, 4);
        asteroidDrawString(x, y, tmp, 1, COLOUR_WHITE, COLOUR_BLACK, false);
        y += 24;

        sprintf(tmp, "%d %d %d", td->x, td->y, td->gesture);
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


WatchApp watchApp;
InfoApp infoApp;
App* currentApp;

App* apps[] = {
    &watchApp,
    &infoApp,
    0
};

void homeApp()
{
    currentAppIdx = 1;
}

void previousApp()
{
    if (currentAppIdx > 0) currentAppIdx--;
}

void nextApp()
{
    if (currentAppIdx < 1) currentAppIdx++;
}

void initApp()
{
    homeApp();
}

void updateApp()
{
    if (previousAppIdx != currentAppIdx) {
        previousAppIdx = currentAppIdx;
        apps[currentAppIdx]->onMount();
    }
    apps[currentAppIdx]->update();
}

void drawApp()
{
    if (previousAppIdx != currentAppIdx) return;
    apps[currentAppIdx]->draw();
}