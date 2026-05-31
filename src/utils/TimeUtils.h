/******************************************************************
 * Smart Egg Incubator - Time Utilities
 * Helper functions for time formatting and uptime
 ******************************************************************/

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <Arduino.h>

inline String formatUptime(unsigned long ms)
{
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;

    char buf[32];
    snprintf(buf, sizeof(buf), "%lud %02lu:%02lu:%02lu",
             days, hours % 24, minutes % 60, seconds % 60);
    return String(buf);
}

inline String formatMillis(unsigned long ms)
{
    if (ms < 1000) return String(ms) + "ms";
    if (ms < 60000) return String(ms / 1000) + "s";
    if (ms < 3600000) return String(ms / 60000) + "min";
    return String(ms / 3600000) + "h";
}

#endif
