#ifndef EDITOR_H
#define EDITOR_H

#include "config.h"

void getLines();
void putChar(char charIn, unsigned int charPos);
void moveCursor(byte distance, char direction);
void showSettings();

#endif