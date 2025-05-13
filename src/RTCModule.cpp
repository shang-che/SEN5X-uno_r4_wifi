#include "RTCModule.h"

RTCModule::RTCModule() {}

bool RTCModule::begin() {
    if (!rtc.begin()) {
        Serial.println("RTC initialization failed!");
        return false;
    }
    Serial.println("RTC initialization succeeded.");
    return true;
}

String RTCModule::getTimestamp() {
    DateTime now = rtc.now();
    return String(now.year()) + "/" + String(now.month()) + "/" +
           String(now.day()) + " " + String(now.hour()) + ":" +
           String(now.minute()) + ":" + String(now.second());
}

float RTCModule::getTemperature() { return rtc.getTemperature(); }
