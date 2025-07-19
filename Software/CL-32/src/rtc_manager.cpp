#include "rtc_manager.h"

void getTime() {
    byte bData;
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.requestFrom(RTC_ADDRESS, 7);
    while (Wire.available() < 1);
    bData = Wire.read();
    CL32time.tm_sec = ((bData >> 4) * 10) + (bData & 0xf);
    bData = Wire.read();
    CL32time.tm_min = ((bData >> 4) * 10) + (bData & 0xf);
    bData = Wire.read();
    CL32time.tm_hour = ((bData >> 4) * 10) + (bData & 0xf);
    bData = Wire.read();
    CL32time.tm_wday = bData - 1;
    bData = Wire.read();
    CL32time.tm_mday = ((bData >> 4) * 10) + (bData & 0xf);
    bData = Wire.read();
    CL32time.tm_mon = (((bData >> 4) & 1) * 10 + (bData & 0xf)) - 1;
    bData = Wire.read();
    CL32time.tm_year = 100 + ((bData >> 4) * 10) + (bData & 0xf);
}

void setTime() {
    byte bData;
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(0x01);
    bData = ((CL32time.tm_sec / 10) << 4);
    bData |= (CL32time.tm_sec % 10);
    Wire.write(bData);
    bData = ((CL32time.tm_min / 10) << 4);
    bData |= (CL32time.tm_min % 10);
    Wire.write(bData);
    bData = ((CL32time.tm_hour / 10) << 4);
    bData |= (CL32time.tm_hour % 10);
    Wire.write(bData);
    bData = CL32time.tm_wday + 1;
    Wire.write(bData);
    bData = (CL32time.tm_mday / 10) << 4;
    bData |= (CL32time.tm_mday % 10);
    Wire.write(bData);
    uint8_t i;
    i = CL32time.tm_mon + 1;
    bData = (i / 10) << 4;
    bData |= (i % 10);
    Wire.write(bData);
    bData = (((CL32time.tm_year % 100) / 10) << 4);
    bData |= (CL32time.tm_year % 10);
    Wire.write(bData);
    Wire.endTransmission();
}