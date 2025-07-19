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
    Serial.println("Starting setup...");
    
    // Initialize chip select pins
    pinMode(CL32_sd_cs, OUTPUT);
    pinMode(CL32_epd_cs, OUTPUT);
    digitalWrite(CL32_sd_cs, HIGH); // Deselect SD card
    digitalWrite(CL32_epd_cs, HIGH); // Deselect e-paper display
    Serial.println("CS pins initialized: SD HIGH, EPD HIGH");
    
    // Initialize SPI with proper pin assignments
    hspi.begin(CL32_sck, CL32_miso, CL32_mosi, -1); // Don't assign CS here
    
    // Initialize display first
    display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(CL32_epd_cs, LOW); // Select display for initialization
    display.init(115200);
    display.setRotation(3);
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap((display.width() / 2) - 148, (display.height() / 2) - 64, image_data_cl32_logo, 296, 128, GxEPD_WHITE, GxEPD_BLACK);
    display.display(true);
    display.hibernate();
    digitalWrite(CL32_epd_cs, HIGH); // Ensure display is deselected after use
    Serial.println("Display initialized and hibernated");
    
    // Small delay to ensure display operations complete
    delay(100);
    
    // Initialize SD card with proper chip select handling
    bool sdInitialized = false;
    for (int i = 0; i < 3 && !sdInitialized; i++) {
        Serial.print("Attempting SD card initialization, attempt ");
        Serial.println(i + 1);
        
        digitalWrite(CL32_epd_cs, HIGH); // Ensure display is deselected
        digitalWrite(CL32_sd_cs, LOW);   // Select SD card
        Serial.println("SD CS LOW for initialization");
        
        if (SD.begin(CL32_sd_cs, hspi)) {
            // Test SD card access
            File test = SD.open("/");
            if (test) {
                test.close();
                sdInitialized = true;
                Serial.println("SD card initialized and tested successfully");
            } else {
                Serial.println("SD card mount test failed");
            }
        } else {
            Serial.println("SD card initialization failed");
        }
        
        digitalWrite(CL32_sd_cs, HIGH); // Deselect SD card
        Serial.println("SD CS HIGH after initialization attempt");
        
        if (!sdInitialized) {
            delay(500); // Wait before retrying
        }
    }
    
    if (!sdInitialized) {
        Serial.println("SD card initialization failed after 3 attempts!");
        sprintf(statusMsg, "SD Card Fail");
        statusMsgTime = millis();
    } else {
        // Initialize file system variables
        strcpy(filePath, "/"); // Use root directory, not /sd
        newFile(); // Initialize with a new file
    }
    
    // Initialize keyboard
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
    
    // Initialize RTC
    Wire.beginTransmission(RTC_ADDRESS);
    Wire.write(0xc0);
    Wire.write(0x20);
    Wire.endTransmission();
    
    // Initialize variables
    iLoop = 0;
    iVolt = 0;
    analogReadResolution(12);
    iBatVolt = (analogRead(CL32_div) / 500.0) * 5.3; // Fix integer division
    
    delay(1000);
    iMode = EDIT;
    bMenu = true;
    drawScreen(true);
    Serial.println("Setup complete");
}

void loop() {
    iVolt += analogRead(CL32_div);
    iLoop++;
    if (iLoop % 500 == 1) {
        iBatVolt = (iVolt / 500.0) * 5.3; // Fix integer division
        iVolt = 0;
    }
    if (!digitalRead(CL32_int)) {
        readKeys();
    }
    delay(100);
}