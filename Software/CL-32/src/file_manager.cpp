#include "file_manager.h"
#include "editor.h"

void saveFile() {
    File curFile;
    char fullFileName[200];
    sprintf(fullFileName, "%s/%s", filePath, fileName);
    curFile = SD.open(fullFileName, FILE_WRITE);
    if (curFile) {
        curFile.seek(0);
        curFile.write((uint8_t*)fileBuffer, fileSize);
        curFile.close();
        sprintf(statusMsg, "Saved: %s", fileName);
    } else {
        sprintf(statusMsg, "Save failed!");
    }
    statusMsgTime = millis();
}

void newFile() {
    memset(fileBuffer, 0, sizeof(fileBuffer));
    fileSize = 0;
    strcpy(fileName, "newfile.txt");
    strcpy(filePath, "/");
    windowX = windowY = iRow = iCol = 0;
    getLines();
    sprintf(statusMsg, "New file started");
    statusMsgTime = millis();
}

void listFile() {
    iFiles = 0;
    File curFile = SD.open(filePath);
    if (curFile) {
        while (true) {
            File entry = curFile.openNextFile();
            if (!entry) {
                break;
            }
            if (!entry.isDirectory()) {
                sprintf(sFileList[iFiles], "%s", entry.name());
                iFiles++;
            }
            entry.close();
        }
        curFile.close();
    }
}

void saveFolder(File dir, int depth, FolderData* parent) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        if (entry.isDirectory()) {
            FolderList[iFolders].layer = depth;
            sprintf(FolderList[iFolders].name, "%s", entry.name());
            FolderList[iFolders].parent = parent;
            iFolders++;
            saveFolder(entry, depth + 1, &FolderList[iFolders - 1]);
        }
        entry.close();
    }
}

void listFolder() {
    iFolders = 0;
    File curFile = SD.open("/");
    if (curFile) {
        FolderList[iFolders].layer = 0;
        sprintf(FolderList[iFolders].name, "%s", "/");
        FolderList[iFolders].parent = NULL;
        iFolders++;
        saveFolder(curFile, 1, &FolderList[iFolders - 1]);
        curFile.close();
    }
}

void getPath(FolderData* input) {
    if (input->parent != NULL) {
        getPath(input->parent);
    }
    sprintf(filePath, "%s/%s", filePath, input->name);
}

void readFile() {
    File curFile;
    char fullFileName[200];
    sprintf(fullFileName, "%s/%s", filePath, fileName);
    if (SD.exists(fullFileName)) {
        curFile = SD.open(fullFileName, FILE_READ);
        if (curFile) {
            fileSize = curFile.size();
            if (fileSize < sizeof(fileBuffer)) {
                for (unsigned int i = 0; i < fileSize; i++) {
                    fileBuffer[i] = curFile.read();
                }
                windowX = windowY = iRow = iCol = 0;
                getLines();
            }
            curFile.close();
        }
    } else {
        fileSize = 0;
    }
}