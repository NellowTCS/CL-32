#include "editor.h"

void getLines() {
    Serial.println("GetLines");
    lineLength = lineCount = 0;
    bCRLF = false;
    lineNumbers[lineCount].start = 0;
    for (unsigned int i = 0; i < fileSize; i++) {
        if (fileBuffer[i] == 13) {
            bCRLF = true;
        }
        if (fileBuffer[i] == 10) {
            lineNumbers[lineCount].end = i;
            lineNumbers[lineCount].len = lineNumbers[lineCount].end - lineNumbers[lineCount].start;
            if (lineNumbers[lineCount].len > lineLength) {
                lineLength = lineNumbers[lineCount].len;
            }
            lineCount++;
            lineNumbers[lineCount].start = i + 1;
        }
    }
    lineNumbers[lineCount].end = fileSize;
    lineNumbers[lineCount].len = lineNumbers[lineCount].end - lineNumbers[lineCount].start;
    if (lineNumbers[lineCount].len > lineLength) {
        lineLength = lineNumbers[lineCount].len;
    }
}

void putChar(char charIn, unsigned int charPos) {
    if (charIn == 8) {
        for (unsigned int x = charPos; x < fileSize; x++) {
            fileBuffer[x] = fileBuffer[x + 1];
        }
        fileSize--;
    } else {
        fileSize++;
        for (unsigned int x = fileSize; x > charPos; x--) {
            fileBuffer[x] = fileBuffer[x - 1];
        }
        fileBuffer[charPos] = charIn;
    }
    getLines();
}

void moveCursor(byte distance, char direction) {
    if (direction == 'N') {
        iRow = iRow - distance;
        if (iRow < 0) {
            iRow = 0;
            windowY = 0;
        } else if (iRow < windowY) {
            windowY--;
        }
    }
    if (direction == 'S') {
        iRow = iRow + distance;
        if (iRow > lineCount) {
            iRow = lineCount;
            if (lineCount < windowH) {
                windowY = 0;
            } else {
                windowY = lineCount - windowH;
            }
        } else if (iRow > (windowY + windowH - 1)) {
            windowY++;
        }
    }
    if (direction == 'E') {
        iCol = iCol + distance;
        if (iCol > lineLength - 1) {
            iCol = lineLength - 1;
            if (lineLength - 2 < windowW) {
                windowX = 0;
            } else {
                windowX = lineLength - windowW + 2;
            }
        } else if (iCol > (windowX + windowW - 3)) {
            windowX++;
        }
    }
    if (direction == 'W') {
        iCol = iCol - distance;
        if (iCol < 0) {
            iCol = 0;
            windowX = 0;
        } else if (iCol < windowX) {
            windowX--;
        }
    }
    Serial.print("iRow=");
    Serial.print(iRow);
    Serial.print(" iCol=");
    Serial.print(iCol);
    Serial.print(" winX=");
    Serial.print(windowX);
    Serial.print(" winY=");
    Serial.print(windowY);
    Serial.print(" len=");
    Serial.println(lineLength);
}

void showSettings() {
    sprintf(statusMsg, "Settings (not implemented)");
    statusMsgTime = millis();
}
