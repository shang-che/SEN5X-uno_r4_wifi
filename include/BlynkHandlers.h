#ifndef BLYNK_HANDLERS_H
#define BLYNK_HANDLERS_H

#include <BlynkSimpleWifi.h>
#include "Arduino_LED_Matrix.h"
extern ArduinoLEDMatrix matrix;

// Global sensor variables (defined externally)
extern float g_pm1p0, g_pm2p5, g_pm4p0, g_pm10p0, g_humidity, g_temperature,
    g_vocIndex, g_noxIndex;
extern int16_t g_adsChannel0Raw, g_adsChannel1Raw, g_adsChannel2Raw,
    g_adsChannel3Raw;

#define MAXTRIX_SPEED 50
// extern uint8_t wifiBitmap_On[8][12];

// V8: Refresh sensor data to Blynk
BLYNK_WRITE(V8) {
    bool refresh = bool(param.asInt());
    if (refresh) {
        Blynk.virtualWrite(V0, g_pm1p0);
        Blynk.virtualWrite(V1, g_pm2p5);
        Blynk.virtualWrite(V2, g_pm4p0);
        Blynk.virtualWrite(V3, g_pm10p0);
        Blynk.virtualWrite(V4, g_humidity);
        Blynk.virtualWrite(V5, g_temperature);
        Blynk.virtualWrite(V6, int(g_vocIndex));
        Blynk.virtualWrite(V7, int(g_noxIndex));
        Blynk.virtualWrite(V17, g_adsChannel0Raw);
        Blynk.virtualWrite(V18, g_adsChannel1Raw);
        Blynk.virtualWrite(V19, g_adsChannel2Raw);
        // Blynk.virtualWrite(V20, g_adsChannel3Raw);
    }
}

#define DEFINE_BLYNK_DISPLAY_HANDLER(pin, label, variable)       \
    BLYNK_WRITE(pin) {                                           \
        bool value = bool(param.asInt());                        \
        if (value) {                                             \
            matrix.beginDraw();                                  \
            matrix.stroke(0xFFFFFFFF);                           \
            matrix.textScrollSpeed(MAXTRIX_SPEED);               \
            matrix.textFont(Font_5x7);                           \
            String text = "     " label ": " + String(variable); \
            matrix.beginText(0, 1, 0xFFFFFF);                    \
            matrix.println(text);                                \
            matrix.endText(SCROLL_LEFT);                         \
            matrix.endDraw();                                    \
        }                                                        \
    }

DEFINE_BLYNK_DISPLAY_HANDLER(V9, "PM1.0", g_pm1p0)
DEFINE_BLYNK_DISPLAY_HANDLER(V10, "PM2.5", g_pm2p5)
DEFINE_BLYNK_DISPLAY_HANDLER(V11, "PM4.0", g_pm4p0)
DEFINE_BLYNK_DISPLAY_HANDLER(V12, "PM10.0", g_pm10p0)
DEFINE_BLYNK_DISPLAY_HANDLER(V13, "Humidity", g_humidity)
DEFINE_BLYNK_DISPLAY_HANDLER(V14, "Temperature", g_temperature)
DEFINE_BLYNK_DISPLAY_HANDLER(V15, "VOC", g_vocIndex)
DEFINE_BLYNK_DISPLAY_HANDLER(V16, "NOx", g_noxIndex)

#endif
