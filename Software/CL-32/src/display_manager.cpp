#include "display_manager.h"
#include "file_manager.h"

void getWindow() {
    unsigned int thisLine = windowY;
    unsigned int thisChar;
    for (int y = 0; y < windowH; y++) {
        thisChar = lineNumbers[thisLine].start + windowX;
        for (int x = 0; x < windowW; x++) {
            if (thisLine > lineCount) {
                codeLines[y][x].val = 0;
                codeLines[y][x].pos = 0;
            } else {
                if (thisChar > lineNumbers[thisLine].end) {
                    codeLines[y][x].val = 0;
                    codeLines[y][x].pos = 0;
                } else {
                    codeLines[y][x].val = fileBuffer[thisChar];
                    codeLines[y][x].pos = thisChar;
                }
            }
            thisChar++;
        }
        thisLine++;
    }
}

void drawScreen(bool bFull) {
    if (bFull) {
        display.setFullWindow();
    } else {
        display.setPartialWindow(0, 0, display.width(), display.height());
    }
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont(&FreeMono9pt7b);
        display.setTextWrap(false);
        iFontH = 12;
        iFontW = 12;
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(0, 10);
        if (bMenu) {
            display.print("Menu");
        } else if (iMode == EDIT) {
            display.print(fileName);
        } else {
            display.print(sMenu[iMode - 1]);
        }
        if (strlen(statusMsg) > 0 && millis() - statusMsgTime < 2000) {
            display.setCursor(10, display.height() - 20);
            display.setFont(&FreeMono9pt7b);
            display.print(statusMsg);
        }
        display.setCursor((display.width() / 2) - 21, 10);
        char sTemp[32];
        sprintf(sTemp, "%02d:%02d %d", CL32time.tm_hour, CL32time.tm_min, analogRead(CL32_div));
        display.print(sTemp);
        display.setCursor(display.width() - 50, 10);
        if (iShift == 0) {
            display.print("abc");
        } else if (iShift == 1) {
            display.print("Abc");
        } else {
            display.print("ABC");
        }
        display.drawLine(display.width() - 11, 0, display.width() - 1, 0, GxEPD_BLACK);
        display.drawLine(display.width() - 11, iFontH, display.width() - 1, iFontH, GxEPD_BLACK);
        display.drawLine(display.width() - 11, 0, display.width() - 11, iFontH, GxEPD_BLACK);
        display.drawLine(display.width() - 1, 0, display.width() - 1, iFontH, GxEPD_BLACK);
        display.drawLine(display.width() - 12, 2, display.width() - 12, iFontH - 2, GxEPD_BLACK);
        display.drawLine(display.width() - 13, 2, display.width() - 13, iFontH - 2, GxEPD_BLACK);
        if (iBatVolt > 3300) {
            display.drawLine(display.width() - 3, 2, display.width() - 3, iFontH - 2, GxEPD_BLACK);
        }
        if (iBatVolt > 3300) {
            display.drawLine(display.width() - 4, 2, display.width() - 4, iFontH - 2, GxEPD_BLACK);
        }
        if (iBatVolt > 3700) {
            display.drawLine(display.width() - 5, 2, display.width() - 5, iFontH - 2, GxEPD_BLACK);
        }
        if (iBatVolt > 3800) {
            display.drawLine(display.width() - 6, 2, display.width() - 6, iFontH - 2, GxEPD_BLACK);
        }
        if (iBatVolt > 3900) {
            display.drawLine(display.width() - 7, 2, display.width() - 7, iFontH - 2, GxEPD_BLACK);
        }
        if (iBatVolt > 4000) {
            display.drawLine(display.width() - 8, 2, display.width() - 8, iFontH - 2, GxEPD_BLACK);
        }
        if (iBatVolt > 4100) {
            display.drawLine(display.width() - 9, 2, display.width() - 9, iFontH - 2, GxEPD_BLACK);
        }
        display.drawLine(0, iFontH + 2, display.width() - 1, iFontH + 2, GxEPD_BLACK);
        if (bMenu) {
            for (int y = 0; y < 6; y++) {
                if (y == iRow) {
                    display.setFont(&FreeMonoBold12pt7b);
                } else {
                    display.setFont(&FreeMono12pt7b);
                }
                display.setCursor(30, (iFontH * 2 + 6) + (y * (iFontH * 2)) + 6);
                display.print(sMenu[y]);
            }
        } else if (iMode == FILE) {
            if (iFolders == 0) {
                listFolder();
                sprintf(filePath, "%s", "");
                getPath(&FolderList[iFol]);
                listFile();
            }
            if (iFolders == -1 && iFiles == -1) {
                display.setCursor(100, 100);
                display.print("SD Card Fail!!");
            } else {
                display.drawLine(display.width() / 2, iFontH + 2, display.width() / 2, display.height() - 1, GxEPD_BLACK);
                display.setFont(&FreeMonoBold9pt7b);
                display.setCursor(5, (iFontH * 2.5) + 6);
                if (iCol % 2 == 0) {
                    display.print("> Folders <");
                } else {
                    display.print("  Folders  ");
                }
                display.setCursor((display.width() / 2) + 5, (iFontH * 2.5) + 6);
                if (iCol % 2 == 0) {
                    display.print("  Files  ");
                } else {
                    display.print("> Files <");
                }
                int iMax, iMin;
                if (iFol < (iPage / 2)) {
                    iMin = 0;
                    iMax = min(iPage, iFolders);
                } else if (iFol > (iFolders - (iPage / 2))) {
                    iMin = max(iFolders - iPage, 0);
                    iMax = iFolders;
                } else {
                    iMin = iFol - (iPage / 2);
                    iMax = iFol + (iPage / 2);
                }
                for (int f = iMin; f < iMax; f++) {
                    if (iFol == f) {
                        display.setFont(&FreeMonoBold9pt7b);
                    } else {
                        display.setFont(&FreeMono9pt7b);
                    }
                    display.setCursor(10 + (iFontW * FolderList[f].layer), (iFontH * 4) + 6 + (iFontH * (f - iMin)));
                    if (strlen(FolderList[f].name) > 13) {
                        char shorter[15];
                        sprintf(shorter, "%.13s..", FolderList[f].name);
                        display.print(shorter);
                    } else {
                        display.print(FolderList[f].name);
                    }
                    if (FolderList[f].layer > 0) {
                        display.drawLine(iFontW * FolderList[f].layer, (iFontH * 4) + (iFontH * (f - iMin)),
                                         iFontW * FolderList[f].layer, (iFontH * 4) + 4 + (iFontH * (f - iMin)), GxEPD_BLACK);
                        display.drawLine(iFontW * FolderList[f].layer, (iFontH * 4) + 4 + (iFontH * (f - iMin)),
                                         4 + (iFontW * FolderList[f].layer), (iFontH * 4) + 4 + (iFontH * (f - iMin)), GxEPD_BLACK);
                    }
                }
                if (iFil < (iPage / 2)) {
                    iMin = 0;
                    iMax = min(iPage, iFiles);
                } else if (iFil > (iFiles - (iPage / 2))) {
                    iMin = max(iFiles - iPage, 0);
                    iMax = iFiles;
                } else {
                    iMin = iFil - (iPage / 2);
                    iMax = iFil + (iPage / 2);
                }
                for (int f = iMin; f < iMax; f++) {
                    if (iFil == f) {
                        display.setFont(&FreeMonoBold9pt7b);
                    } else {
                        display.setFont(&FreeMono9pt7b);
                    }
                    display.setCursor(10 + (display.width() / 2), (iFontH * 4) + 6 + (iFontH * (f - iMin)));
                    if (strlen(sFileList[f]) > 15) {
                        char shorter[17];
                        sprintf(shorter, "%.15s..", sFileList[f]);
                        display.print(shorter);
                    } else {
                        display.print(sFileList[f]);
                    }
                }
            }
        } else if (iMode == BEEP) {
            display.setCursor(100, 100);
            display.print("Make some noise!!");
        } else if (iMode == EDIT) {
            if (strlen(fileName) == 0) {
                display.setCursor(40, 100);
                display.print("Please Open A File To Edit");
            } else {
                getWindow();
                display.setCursor(0, (iFontH * 2));
                for (int y = 0; y < windowH; y++) {
                    for (int x = 0; x < windowW; x++) {
                        display.setFont(&FreeMono9pt7b);
                        display.setCursor(x * iFontW, (iFontH * 2) + (y * iFontH) + 4);
                        if (x == iCol - windowX && y == iRow - windowY) {
                            display.setTextColor(GxEPD_WHITE);
                            display.fillRect(x * iFontW, (iFontH) + (y * iFontH) + 5, iFontW, iFontH, GxEPD_BLACK);
                        } else {
                            display.setTextColor(GxEPD_BLACK);
                        }
                        display.print(codeLines[y][x].val);
                    }
                }
            }
        }
    } while (display.nextPage());
    display.hibernate();
}