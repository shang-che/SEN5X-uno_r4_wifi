#ifndef RTCMODULE_H
#define RTCMODULE_H

#include <Arduino.h>
#include <RTClib.h>

class RTCModule {
   public:
    RTCModule();
    bool begin();
    String getTimestamp();
    String getTimestamphhmm();
    int getTimeminute();
    float getTemperature();

   private:
    RTC_DS3231 rtc;
};

#endif
