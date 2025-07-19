#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "config.h"
#include <SD.h>

void saveFile();
void newFile();
void listFile();
void listFolder();
void readFile();
void getPath(FolderData* input);

#endif