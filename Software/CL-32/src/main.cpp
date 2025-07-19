#include "config.h"
#include "display_manager.h"
#include "file_manager.h"
#include "keyboard_manager.h"
#include "rtc_manager.h"
#include <Wire.h>
#include <SD.h>
#include <CL_32_logo.h>
#include <CL32_logo.h>
#include "bitmaps/Bitmaps168x384.h"

void setup() {
    Wire.begin(CL32_sda, CL32_scl);
    Serial.begin(115200);
    hspi.begin(CL32_sck, CL32_miso, CL32_mosi, CL32_epd_cs);
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    display.init(115200);
    display.setRotation(3);
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap((display.width() / 2) - 148, (display.height() / 2) - 64, image_data_cl32_logo, 296, 128, GxEPD_WHITE, GxEPD_BLACK);
    display.display(true);
    display.hibernate();
    if (!SD.begin(CL32_sd_cs, hspi)) {
        Serial.println("SD Card initialization failed!");
        sprintf(statusMsg, "SD Card Fail");
        statusMsgTime = millis();
    }
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x01);
    Wire.write(0x19);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x02);
    Wire.write(0xff);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x1d);
    Wire.write(0xff);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x1e);
    Wire.write(0xff);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x1f);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x29);
    Wire.write(0xff);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x2a);
    Wire.write(0xff);
    Wire.endTransmission();
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x2b);
    Wire.write(0x01);
    Wire.endTransmission();
    pinMode(CL32_int, INPUT);
    pinMode(CL32_buz, OUTPUT);
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(0xc0);
    Wire.write(0x20);
    Wire.endTransmission();
    iLoop = 0;
    iVolt = 0;
    analogReadResolution(12);
    iBatVolt = (analogRead(CL32_div) / 500) * 5.3;
    delay(1000);
    iMode = EDIT;
    bMenu = true;
    drawScreen(true);
}

void loop() {
    iVolt += analogRead(CL32_div);
    iLoop++;
    if (iLoop % 500 == 1) {
        iBatVolt = (iVolt / 500) * 5.3;
        iVolt = 0;
    }
    if (!digitalRead(CL32_int)) {
        readKeys();
    }
    delay(100);
}