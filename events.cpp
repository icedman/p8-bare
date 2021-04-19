#include "events.h"
#include "sleep.h"

static int event_idx = 0;
static event events[MAX_EVENTS];

void initEvents()
{
    for(int i=0; i<MAX_EVENTS; i++) {
        events[i].e = 0;
    }
}

void addEvent(int event, int x, int y, int z)
{
    for(int i=0; i<MAX_EVENTS; i++) {
        if (events[event_idx].e == 0) {
            events[event_idx].e = event;
            events[event_idx].x = x;
            events[event_idx].y = y;
            events[event_idx].z = z;
            event_idx++;
            return;
        }
    }
}

void clearEvents()
{
    if (event_idx != 0) {
        caffeinate();
    }

    for(int i=0; i<MAX_EVENTS; i++) {
        if (events[i].e == 0) break;
        events[i].e = 0;
    }
    event_idx = 0;
}

event* getEvents()
{
    return events;
}