#ifndef _BLISP_UTIL_H
#define _BLISP_UTIL_H

#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static void sleep_ms(int milliseconds){
#ifdef WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

#endif