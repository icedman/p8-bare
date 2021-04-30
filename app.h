#pragma once

#include <Arduino.h>
#include "display.h"

void initApp();
void updateApp();
void drawApp();

void homeApp();
void previousApp();
void nextApp();
void showNotice(char *notice);

class App {
public:

    App() : hash(0) {}

    virtual void onMount();
    virtual void onWake() {
        hash = 0;
    }
    virtual void onUnmount() {}
    virtual void onUpdate() {}
    virtual void onDraw() {}
    virtual void onButtonDown() {}
    virtual void onButtonUp() {
        homeApp();
    }
    virtual void onTouchDown() {}
    virtual void onTouchUp() {}
    virtual void onTouchGesture(int gesture);

    void update();
    void draw();

protected:
    uint32_t hash;
};
