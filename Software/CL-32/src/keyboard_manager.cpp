#include "keyboard_manager.h"
#include "file_manager.h"
#include "editor.h"
#include "display_manager.h"

void readKeys() {
    byte bCount, bEvent, bEventCode, bEventStat;
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x03);
    Wire.endTransmission();
    Wire.requestFrom(KEYBOARD_ADDRESS, 1);
    while (Wire.available() < 1);
    bCount = Wire.read();
    while (bCount > 0) {
        Wire.beginTransmission(KEYBOARD_ADDRESS);
        Wire.write(0x04);
        Wire.endTransmission();
        Wire.requestFrom(KEYBOARD_ADDRESS, 1);
        while (Wire.available() < 1);
        bEvent = Wire.read();
        bEventCode = bEvent & 0x7f;
        bEventStat = (bEvent & 0x80) >> 7;
        if (bEventStat == 1) {
            if (iMode == BEEP && !bMenu && keyMap[bEventCode - 1] != 60) {
                tone(CL32_buz, bEventCode * 100);
            } else {
                if (lower[bEventCode - 1] == 0) {
                    if (keyMap[bEventCode - 1] == 42) {
                        if (iMode == EDIT && !bMenu) {
                            moveCursor(1, 'W');
                            putChar(8, codeLines[iRow][iCol].pos);
                        }
                    }
                    if (keyMap[bEventCode - 1] == 40) {
                        if (bMenu) {
                            iMode = iRow + 1;
                            bMenu = false;
                            iCol = 0;
                            iRow = 0;
                            iFiles = 0;
                            iFolders = 0;
                            if (iMode == FILE) {
                            } else if (iMode == EDIT) {
                            } else if (iMode == SAVE) {
                                saveFile();
                                iMode = EDIT;
                            } else if (iMode == NEW) {
                                newFile();
                                iMode = EDIT;
                            } else if (iMode == SET) {
                                sprintf(statusMsg, "Settings (not implemented)");
                                statusMsgTime = millis();
                                drawScreen(true);
                                delay(1000);
                                statusMsg[0] = '\0';
                                iMode = EDIT;
                                bMenu = true;
                            }
                        } else {
                            if (iMode == FILE) {
                                if (iCol % 2 == 0) {
                                    iCol = 1;
                                } else {
                                    sprintf(fileName, "%s", sFileList[iFil]);
                                    readFile();
                                    iMode = EDIT;
                                    bMenu = false;
                                }
                            } else if (iMode == EDIT) {
                                putChar(10, codeLines[iRow][iCol].pos + 1);
                                if (bCRLF) {
                                    putChar(13, codeLines[iRow][iCol].pos);
                                }
                            } else if (iMode == SAVE) {
                                saveFile();
                                iMode = EDIT;
                            } else if (iMode == NEW) {
                                newFile();
                                iMode = EDIT;
                            } else if (iMode == SET) {
                                sprintf(statusMsg, "Settings (not implemented)");
                                statusMsgTime = millis();
                                drawScreen(true);
                                delay(1000);
                                statusMsg[0] = '\0';
                                iMode = EDIT;
                                bMenu = true;
                            }
                        }
                    }
                    if (keyMap[bEventCode - 1] == 60) {
                        bMenu = !bMenu;
                        iCol = 0;
                        iRow = 0;
                    }
                    if (keyMap[bEventCode - 1] == 225) {
                        iShift++;
                        if (iShift > 2) {
                            iShift = 0;
                        }
                    } else if (keyMap[bEventCode - 1] == 79) {
                        if (iShift > 0) {
                            moveCursor(windowW, 'E');
                            if (iShift == 1) {
                                iShift = 0;
                            }
                        } else {
                            moveCursor(1, 'E');
                        }
                    } else if (keyMap[bEventCode - 1] == 80) {
                        if (iShift > 0) {
                            moveCursor(windowW, 'E');
                            if (iShift == 1) {
                                iShift = 0;
                            }
                        } else {
                            moveCursor(1, 'E');
                        }
                    } else if (keyMap[bEventCode - 1] == 81) {
                        if (iMode == FILE && !bMenu) {
                            if (iCol % 2 == 0) {
                                iFol++;
                                if (iFol > iFolders - 1) {
                                    iFol = iFolders - 1;
                                }
                                sprintf(filePath, "%s", "");
                                getPath(&FolderList[iFol]);
                                listFile();
                            } else {
                                iFil++;
                                if (iFil > iFiles - 1) {
                                    iFil = iFiles - 1;
                                }
                            }
                        } else if (iMode == EDIT && !bMenu) {
                            if (iShift > 0) {
                                moveCursor(windowH, 'S');
                                if (iShift == 1) {
                                    iShift = 0;
                                }
                            } else {
                                moveCursor(1, 'S');
                            }
                        } else {
                            iRow++;
                            if (iRow > 15) {
                                iRow = 15;
                            }
                        }
                    } else if (keyMap[bEventCode - 1] == 82) {
                        if (iMode == FILE && !bMenu) {
                            if (iCol % 2 == 0) {
                                iFol--;
                                if (iFol < 0) {
                                    iFol = 0;
                                }
                                sprintf(filePath, "%s", "");
                                getPath(&FolderList[iFol]);
                                listFile();
                            } else {
                                iFil--;
                                if (iFil < 0) {
                                    iFil = 0;
                                }
                            }
                        } else if (iMode == EDIT && !bMenu) {
                            if (iShift > 0) {
                                moveCursor(windowH, 'N');
                                if (iShift == 1) {
                                    iShift = 0;
                                }
                            } else {
                                moveCursor(1, 'N');
                            }
                        } else {
                            iRow--;
                            if (iRow < 0) {
                                iRow = 0;
                            }
                        }
                    }
                } else {
                    if (iMode == EDIT && !bMenu) {
                        if (iShift > 0) {
                            putChar(upper[bEventCode - 1], codeLines[iRow][iCol].pos);
                            iCol++;
                            if (iShift == 1) {
                                iShift = 0;
                            }
                        } else {
                            putChar(lower[bEventCode - 1], codeLines[iRow][iCol].pos);
                            iCol++;
                        }
                        if (iCol >= windowW) {
                            iCol = 0;
                            iRow++;
                        }
                    }
                }
            }
        } else {
            if (iMode == BEEP && !bMenu) {
                noTone(CL32_buz);
            }
        }
        bCount--;
    }
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x02);
    Wire.write(0xff);
    Wire.endTransmission();
    if (iMode != BEEP || bMenu) {
        drawScreen(false);
    }
}