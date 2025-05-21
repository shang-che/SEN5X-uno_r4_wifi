#include "RTCModule.h"

RTCModule::RTCModule() {}

bool RTCModule::begin() {
    if (!rtc.begin()) {
        Serial.println("RTC initialization failed!");
        return false;
    }
    Serial.println("RTC initialized successfully.");
    return true;
}

String RTCModule::getTimestamp() {
    DateTime now = rtc.now();
    return String(now.year()) + "/" + String(now.month()) + "/" +
           String(now.day()) + " " + String(now.hour()) + ":" +
           String(now.minute()) + ":" + String(now.second());
}

float RTCModule::getTemperature() { return rtc.getTemperature(); }
