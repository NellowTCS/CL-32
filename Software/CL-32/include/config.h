#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// Pin definitions
#define CL32_sck 9
#define CL32_epd_cs 6
#define CL32_sd_cs 7
#define CL32_mosi 10
#define CL32_miso 11
#define CL32_dc 13
#define CL32_rst 12
#define CL32_bsy 14
#define CL32_sda 1
#define CL32_scl 2
#define CL32_int 3
#define CL32_div 4
#define CL32_buz 5
#define CL32_kill 45
#define KEYBOARD_ADDRESS 0x34
#define RTC_ADDRESS 0x51

// Modes
#define RUN 0
#define EDIT 1
#define SAVE 2
#define FILE 3
#define NEW 4
#define SET 5
#define BEEP 6

// Structs
struct CharData {
    char val;
    unsigned int pos;
};

struct LineData {
    unsigned int start;
    unsigned int end;
    unsigned int len;
};

struct FolderData {
    FolderData *parent;
    byte layer;
    char name[30];
};

// Global variables
extern char lower[80];
extern char upper[80];
extern char keyMap[80];
extern const byte windowW;
extern const byte windowH;
extern CharData codeLines[12][34];
extern unsigned int windowX, windowY;
extern char fileBuffer[50000];
extern LineData lineNumbers[1000];
extern unsigned int fileSize, lineCount;
extern int lineLength;
extern bool bCRLF;
extern char statusMsg[64];
extern unsigned long statusMsgTime;
extern const char *sMenu[];
extern char fileName[20];
extern char filePath[50];
extern int iVolt, iLoop, iBatVolt;
extern bool bMenu;
extern byte iMode, iShift;
extern int iRow, iCol;
extern byte iFontW, iFontH;
extern int iPage;
extern SPIClass SPI_SD, SPI_EPD;
extern char sFileList[20][50];
extern FolderData FolderList[50];
extern int iFolders, iFiles;
extern int iFol, iFil;
extern struct tm CL32time;
extern SPIClass hspi;
extern GxEPD2_BW<GxEPD2_290_GDEY029T71H, GxEPD2_290_GDEY029T71H::HEIGHT> display;

#endif