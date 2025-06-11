#include "RTCModule.h"

RTCModule::RTCModule() {}

bool RTCModule::begin() {
    if (!rtc.begin()) {
        Serial.println("RTC initialization failed!");
        return false;
    }
    Serial.println("RTC initialized successfully.");
    rtc.adjust(DateTime(__DATE__, __TIME__));
    Serial.println("RTC time set to compile time.");
    return true;
}

String RTCModule::getTimestamp() {
    DateTime now = rtc.now();
    return String(now.year()) + "/" + String(now.month()) + "/" +
           String(now.day()) + " " + String(now.hour()) + ":" +
           String(now.minute()) + ":" + String(now.second());
}

String RTCModule::getTimestamphhmm() {
    DateTime now = rtc.now();
    return String(now.hour()) + ":" + String(now.minute());
}
int RTCModule::getTimeminute() {
    DateTime now = rtc.now();
    return int(now.minute());
}
float RTCModule::getTemperature() { return rtc.getTemperature(); }
