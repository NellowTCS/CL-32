#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>

#include <CL_32_logo.h>
#include <CL32_logo.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

char lower[80] = {'%' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,0   ,'9' ,'0' ,0   ,'[' ,']' ,'+' ,'"' ,'\'',0   ,0   ,
                  0   ,'q' ,'w' ,'e' ,'r' ,'t' ,'y' ,'u' ,'i' ,0   ,'o' ,'p' ,0   ,'(' ,')' ,'-' ,';' ,':' ,0   ,0   ,
                  0   ,'a' ,'s' ,'d' ,'f' ,'g' ,'h' ,'j' ,'k' ,0   ,'l' ,0   ,'#' ,'{' ,'}' ,'*' ,',' ,'.' ,0   ,0   ,
                  'z' ,'x' ,'c' ,'v' ,'b' ,' ' ,' ' ,'n' ,'m' ,0   ,0   ,0   ,0   ,'<' ,'>' ,'/' ,'\\','=' ,0   ,0   };
char upper[80] = {'%' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,0   ,'9' ,'0' ,0   ,'[' ,']' ,'+' ,'"' ,'\'',0   ,0   ,
                  0   ,'Q' ,'W' ,'E' ,'R' ,'T' ,'Y' ,'U' ,'I' ,0   ,'O' ,'P' ,0   ,'(' ,')' ,'-' ,';' ,':' ,0   ,0   ,
                  0   ,'A' ,'S' ,'D' ,'F' ,'G' ,'H' ,'J' ,'K' ,0   ,'L' ,0   ,'#' ,'{' ,'}' ,'*' ,',' ,'.' ,0   ,0   ,
                  'Z' ,'X' ,'C' ,'V' ,'B' ,' ' ,' ' ,'N' ,'M' ,0   ,0   ,0   ,0   ,'<' ,'>' ,'/' ,'\\','=' ,0   ,0   };
char keyMap[80] ={0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,42  ,0   ,0   ,0   ,0   ,0   ,58  ,0   ,
                  43  ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,40  ,0   ,0   ,0   ,0   ,0   ,59  ,0   ,
                  225 ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,82  ,0   ,0   ,0   ,0   ,0   ,0   ,60  ,0   ,
                  0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,80  ,81  ,79  ,0   ,0   ,0   ,0   ,0   ,61  ,0   };

//store the 'window' of code in an array. the struct will let me store more info about each char like the position in the
//full file and maybe even formatting data for bold and italic later maybe
struct CharData {
  char val;
  unsigned int pos;
};

const byte windowW = 34;
const byte windowH = 12;
//this array stores the 'window' of code, it will be populated based on the position of the data within the file
CharData codeLines[windowH][windowW] ;
unsigned int windowX, windowY;

//lets make a massive array of space to store the files, that will mean some files are too big to open, but
//will also mean that the memory usage will be consistant
char fileBuffer[50000];
//each line has a start and end value, i was just grabbing the next line to get the end of the last one, but that
//will error if you try that on the last line (or at leas behave strangely) so lets store the start and end for each line
struct LineData {
  unsigned int start;
  unsigned int end;
  unsigned int len;
};
//the file could be 50 lines of 100 char of text, or could be 1000 lines with 5 chars of text, so it might be handy to have a
//list of where in the file each line of the text is. therefore we need an array to store that too
LineData lineNumbers[1000];
//and some variables to store how big things are...
unsigned int fileSize, lineCount;
int lineLength;
//does the file have windows style carrage return and line feed?
bool bCRLF;

// Status message for UI feedback
char statusMsg[64] = "";
unsigned long statusMsgTime = 0;

// Function prototypes for file management
void saveFile();
void newFile();
void showSettings();
void getLines(); // Prototype for getLines used in newFile()

struct FolderData {
    FolderData *parent;
    byte layer;
    char name[30];
};

const char *sMenu[] = {"Editor","Save", "Open File", "New File", "Settings", "Beeper"};
char fileName[20] = {""};
char filePath[50] = {"/"};

int iVolt, iLoop, iBatVolt;
bool bMenu;
byte iMode, iShift;
int iRow, iCol;
byte iFontW, iFontH;
int iPage = 10;
SPIClass SPI_SD, SPI_EPD;
char sFileList[20][50] = {""};
FolderData FolderList[50];
int iFolders, iFiles;
int iFol, iFil;
struct tm CL32time;

#define KEYBOARD_ADDRESS 0x34
#define RTC_ADDRESS 0x51
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
//some values for modes
#define RUN 0
#define EDIT 1
#define SAVE 2
#define FILE 3
#define NEW 4
#define SET 5
#define BEEP 6

//high res 2.9
#include "bitmaps/Bitmaps168x384.h" // 2.9"  b/w

SPIClass hspi(HSPI);

// --- FILE MANAGEMENT FUNCTIONS ---
// Save file to SD card
// hspi is now declared globally above
void saveFile() {
    File curFile;
    char fullFileName[200];
    sprintf(fullFileName, "%s/%s", filePath, fileName);
    if (SD.begin(CL32_sd_cs, hspi)) {
        curFile = SD.open(fullFileName, FILE_WRITE);
        if (curFile) {
            curFile.seek(0);
            curFile.write((uint8_t*)fileBuffer, fileSize);
            // curFile.truncate(fileSize); // Not available in ESP32 SD library
            curFile.close();
            sprintf(statusMsg, "Saved: %s", fileName);
        } else {
            sprintf(statusMsg, "Save failed!");
        }
    } else {
        sprintf(statusMsg, "SD Card Fail");
    }
    SD.end();
    SPI_SD.end();
    statusMsgTime = millis();
}

// Start a new file (clear buffer and prompt for filename)
// getLines is now declared above
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

// Placeholder for settings
void showSettings() {
    sprintf(statusMsg, "Settings (not implemented)");
    statusMsgTime = millis();
}
GxEPD2_BW<GxEPD2_290_GDEY029T71H, GxEPD2_290_GDEY029T71H::HEIGHT> display(GxEPD2_290_GDEY029T71H(/*CS=D8*/ CL32_epd_cs, /*DC=D3*/ CL32_dc, /*RST=D4*/ CL32_rst, /*BUSY=D2*/ CL32_bsy)); // GDEY029T71H 168x384, SSD1685, (FPC-H004 22.03.24)

//Low res 2.9
//#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w
//GxEPD2_BW<GxEPD2_290_GDEY029T94, GxEPD2_290_GDEY029T94::HEIGHT> display(GxEPD2_290_GDEY029T94(/*CS=D8*/ CL32_epd_cs, /*DC=D3*/ CL32_dc, /*RST=D4*/ CL32_rst, /*BUSY=D2*/ CL32_bsy)); // GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)


void getTime(){

  byte bData;
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x01);//0x01 is where we start
  Wire.endTransmission();
  //read the contents
  Wire.requestFrom(RTC_ADDRESS,7);
  while(Wire.available() < 1);//sit and wait for response
  //read the value
  bData = Wire.read();
  CL32time.tm_sec = ((bData >> 4) * 10) + (bData & 0xf);
  bData = Wire.read();
  CL32time.tm_min = ((bData >> 4) * 10) + (bData & 0xf);
  bData = Wire.read();
  CL32time.tm_hour = ((bData >> 4) * 10) + (bData & 0xf);
  bData = Wire.read();
  CL32time.tm_wday = bData - 1; // day of the week (0-6)
  // day of the month
  bData = Wire.read();
  CL32time.tm_mday = ((bData >> 4) * 10) + (bData & 0xf);
  // month
  bData = Wire.read();
  CL32time.tm_mon = (((bData >> 4) & 1) * 10 + (bData & 0xf)) -1; // 0-11 
  bData = Wire.read();    
  CL32time.tm_year = 100 + ((bData >> 4) * 10) + (bData & 0xf);
}

void setTime(){
  byte bData;
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x01);//0x01 is where we start
  //seconds
  bData = ((CL32time.tm_sec / 10) << 4);
  bData |= (CL32time.tm_sec % 10);
  Wire.write(bData);
  // minutes
  bData = ((CL32time.tm_min / 10) << 4);
  bData |= (CL32time.tm_min % 10);
  Wire.write(bData);
  // hours
  bData = ((CL32time.tm_hour / 10) << 4);
  bData |= (CL32time.tm_hour % 10);
  Wire.write(bData);
  // day of the week
  bData = CL32time.tm_wday + 1;
  Wire.write(bData);
  // day of the month
  bData = (CL32time.tm_mday / 10) << 4;
  bData |= (CL32time.tm_mday % 10);
  Wire.write(bData);
  // month
  uint8_t i;
  i = CL32time.tm_mon+1; // 1-12 on the RTC
  bData = (i / 10) << 4;
  bData |= (i % 10);
  Wire.write(bData);
  // year
  bData = (((CL32time.tm_year % 100)/10) << 4);
  bData |= (CL32time.tm_year % 10);
  Wire.write(bData);
  Wire.endTransmission();

}

void listFile(){
  iFiles = 0;
  //use the non standard spi pin connection to start an sd card
  if(SD.begin(CL32_sd_cs,hspi)){
    File curFile = SD.open(filePath);
    while (true) {
      File entry =  curFile.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      if (entry.isDirectory() == false) {
        sprintf(sFileList[iFiles], "%s", entry.name());
        iFiles++;
      }
      entry.close();
    } 
  }
  else{
    iFiles = -1;
  }
  SD.end();
  SPI_SD.end();
}


void saveFolder(File dir, int depth, FolderData* parent) {

  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    if (entry.isDirectory()) {
      FolderList[iFolders].layer = depth;
      sprintf(FolderList[iFolders].name,"%s" ,entry.name());
      FolderList[iFolders].parent = parent;
      iFolders++;//increment it here so the funcion call has the corect value for a new entry
      saveFolder(entry, depth + 1,&FolderList[iFolders-1]);//folders count has already been incremented, so we need to knock it back one
    }
    entry.close();
  }
}

void listFolder(){
  iFolders = 0;
  //use the non standard spi pin connection to start an sd card
  if(SD.begin(CL32_sd_cs,hspi)){
    File curFile = SD.open("/");
    //need to save the root folder
    FolderList[iFolders].layer = 0;
    sprintf(FolderList[iFolders].name,"%s","/");
    FolderList[iFolders].parent = NULL;
    iFolders++;
    saveFolder(curFile,1,&FolderList[iFolders-1]);
  }
  else{
    iFolders = -1;
  }
  SD.end();
  SPI_SD.end();
}

void getLines(){
  //process the file to re-populate the line data
  Serial.println("GetLines");
  lineLength=lineCount=0;
  bCRLF = false;
  //store the first char pos as the 
  lineNumbers[lineCount].start = 0;
  for(unsigned int i=0;i<fileSize;i++){
    //lets see if this char is a newline char windows files have 0D 0A linux files just have 0A
    if(fileBuffer[i]==13){
      //its a windows style carrage return
      bCRLF = true;//flag windows stylee
    } 
    if(fileBuffer[i]==10){
      //its a line feed, so the next line starts at the next char...
      lineNumbers[lineCount].end = i;//log the end of the last line
      lineNumbers[lineCount].len = lineNumbers[lineCount].end-lineNumbers[lineCount].start;
      if(lineNumbers[lineCount].len>lineLength){
        lineLength=lineNumbers[lineCount].len;
      }
      lineCount++;
      lineNumbers[lineCount].start = i + 1;//log this as the start of a new line
    }
  }
  lineNumbers[lineCount].end = fileSize;//log the end of the last line
  lineNumbers[lineCount].len = lineNumbers[lineCount].end-lineNumbers[lineCount].start;
  if(lineNumbers[lineCount].len>lineLength){
    lineLength=lineNumbers[lineCount].len;
  }
}

void readFile(){

  //local file read variable
  File curFile;
  char fullFileName[200];
  sprintf(fullFileName,"%s/%s",filePath,fileName);
  //use the non standard spi pin connection to start an sd card
  if(SD.begin(CL32_sd_cs,hspi)){
    if(SD.exists(fullFileName)){
      curFile = SD.open(fullFileName,FILE_READ);
      if(curFile){
        fileSize = curFile.size();  // Get the file size.
        //sanity check
        if(fileSize < sizeof(fileBuffer)){
          for(unsigned int i=0;i<fileSize;i++){
            fileBuffer[i] = curFile.read();         // Read the file into the buffer.
          }
          windowX=windowY=iRow=iCol=0;//reset
          getLines();
        }
        curFile.close();          
      }
    }
    else{
      fileSize = 0;//so we can check later
    }
  }
  else{
    Serial.print("SD Card Fail");
  }
  SD.end();
  SPI_SD.end();
}

void getPath(FolderData* input){
  if(input->parent!=NULL){
    getPath(input->parent);
  }
  sprintf(filePath,"%s/%s",filePath,input->name);
}

void getWindow(){
  //we need to grab the lines to fit on the screen
  //there is an x/y value for the top left corner of the 'window' we can use that to
  //count the lines we are reading, and also use that to offset the x in the future too
  //the scroll of will nudge the window down as you scroll to the end of the window, and back up
  //in the same fashion
  unsigned int thisLine = windowY;
  unsigned int thisChar;
  for(int y = 0;y<windowH;y++){
    thisChar = lineNumbers[thisLine].start+windowX;
    for(int x = 0;x<windowW;x++){
      if (thisLine > lineCount){
        //if this happens, we have run out of lines in the file. just fill the space with nothing
        codeLines[y][x].val = 0;
        codeLines[y][x].pos = 0;
      }
      else{
        //the line is valid, lets check to see if the current x pos has a valid char in it
        if(thisChar>lineNumbers[thisLine].end){
          //the next char in the array is greater than the end of the line, just pad with 0
          codeLines[y][x].val = 0;
          codeLines[y][x].pos = 0;
        }
        else{
          //should be good to go, lets gos
          codeLines[y][x].val = fileBuffer[thisChar];
          codeLines[y][x].pos = thisChar;
        }
      }
      thisChar++;
    }
    thisLine++;
  }

}

void putChar(char charIn,unsigned int charPos){
  //so this lets you modify the file in memory, it basically will start at the end of the array
  //and move each char along one until we hit charPos. then insert the new char in there
  //if the char you pass in is the backspace char, then it works the other way, it moves everything
  //forward in the array...
  if(charIn==8){
    //delete a char
    for(unsigned int x = charPos;x<fileSize;x++){
      fileBuffer[x]=fileBuffer[x+1];
    }
    fileSize--;
  }
  else{
    //add a char
    fileSize++;
    for(unsigned int x = fileSize;x>charPos;x--){
      fileBuffer[x]=fileBuffer[x-1];
    }
    fileBuffer[charPos]=charIn;
  }
  getLines();
}

void moveCursor(byte distance,char direction){
  //because the available cursor positions are now limited by the code on the screen
  //its best to have a universal function to decide where the cursor can move, and
  //what to do if it cant move there
  //basically you send how far you want to move (normally 1) and direction N/E/S/W
  if(direction=='N'){
    iRow=iRow-distance;
    if(iRow<0){
      //gone too far, step back
      iRow = 0;
      windowY = 0;
    }
    else if(iRow<(windowY)){
      //if we have dropped off the bottom of the window, lets move the window
      windowY--;
    }
  }
  if(direction=='S'){
    iRow=iRow+distance;
    if(iRow>lineCount){
      //gone too far, step back
      iRow = lineCount;
      if(lineCount<windowH){
        //there are less lines than the height of the window, revert to 0
        windowY = 0;
      }
      else{
        windowY = lineCount-windowH;
      }
    }
    else if(iRow>(windowY+windowH-1)){
      //if we have dropped off the bottom of the window, lets move the window
      windowY++;
    }
  }
  if(direction=='E'){
    iCol=iCol+distance;
    if(iCol > lineLength-1){
      iCol=lineLength-1;
      if(lineLength-2<windowW){
        windowX=0;
      }
      else{
        windowX = lineLength-windowW+2;
      }
    }
    else if(iCol>(windowX+windowW-3)){
      windowX++;
    }
  }
  if(direction=='W'){
    iCol=iCol-distance;
    if(iCol < 0){
      iCol=0;
      windowX=0;
    }
    else if(iCol<(windowX)){
      //if we have dropped off the bottom of the window, lets move the window
      windowX--;
    }
  }
  //the cursor could now be in no mans land at the end of a line that is
  //sorter than the current pos. lets move left until we find a valid char
  // for(byte x = iCol;x>0;x--){
  //   //check to see if there is a position value here, if there is, lets see if its a printable char
  //   if(codeLines[iRow][x].pos>0){
  //     //we have a position value. so this is an item from the file. lets check if its printable
  //     if(codeLines[iRow][x].val!=10&&codeLines[iRow][x].val!=13&&codeLines[iRow][x].val!=0){
  //       iCol=x;
  //       break;
  //     }
  //   }
  // }
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

void drawScreen(bool bFull) {
  
  if(bFull){
    display.setFullWindow();
  }
  else{
    display.setPartialWindow(0,0,display.width(),display.height());
  }
  display.firstPage();
  do{
    display.fillScreen(GxEPD_WHITE); // 0 for EPDs is white (for OneBitDisplay)
    display.setFont(&FreeMono9pt7b);
    display.setTextWrap(false);
    iFontH = 12;
    iFontW = 12;
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0,10);
    if(bMenu){
      display.print("Menu");
    }
    else if(iMode==EDIT){
      display.print(fileName);
    }
    else{
      display.print(sMenu[iMode-1]);
    }
    // Show status message if present
    if (strlen(statusMsg) > 0 && millis() - statusMsgTime < 2000) {
      display.setCursor(10, display.height() - 20);
      display.setFont(&FreeMono9pt7b);
      display.print(statusMsg);
    }

    display.setCursor((display.width()/2)-21,10);
    char sTemp[32];
    getTime();
    sprintf(sTemp, "%02d:%02d %d", CL32time.tm_hour, CL32time.tm_min,analogRead(CL32_div));
    display.print(sTemp);
    //display.print("12:34");
    display.setCursor(display.width()-50,10);
    if (iShift==0){
      display.print("abc");
    }
    else if (iShift==1){
      display.print("Abc");
    }
    else{
      display.print("ABC");
    }
    display.drawLine(display.width()-11,0,display.width()-1,0,GxEPD_BLACK);
    display.drawLine(display.width()-11,iFontH,display.width()-1,iFontH,GxEPD_BLACK);
    display.drawLine(display.width()-11,0,display.width()-11,iFontH,GxEPD_BLACK);
    display.drawLine(display.width()-1,0,display.width()-1,iFontH,GxEPD_BLACK);
    display.drawLine(display.width()-12,2,display.width()-12,iFontH-2,GxEPD_BLACK);
    display.drawLine(display.width()-13,2,display.width()-13,iFontH-2,GxEPD_BLACK);
    //battery bars
    if (iBatVolt > 3300){
    display.drawLine(display.width()-3,2,display.width()-3,iFontH-2,GxEPD_BLACK); //3.3v
    }
    if (iBatVolt > 3300){
      display.drawLine(display.width()-4,2,display.width()-4,iFontH-2,GxEPD_BLACK); //3.5v
    }
    if (iBatVolt > 3700){
      display.drawLine(display.width()-5,2,display.width()-5,iFontH-2,GxEPD_BLACK); //3.7v
    }
    if (iBatVolt > 3800){
      display.drawLine(display.width()-6,2,display.width()-6,iFontH-2,GxEPD_BLACK); //3.8v
    }
    if (iBatVolt > 3900){
      display.drawLine(display.width()-7,2,display.width()-7,iFontH-2,GxEPD_BLACK); //3.9v
    }
    if (iBatVolt > 4000){
      display.drawLine(display.width()-8,2,display.width()-8,iFontH-2,GxEPD_BLACK); //4.0v
    }
    if (iBatVolt > 4100){
      display.drawLine(display.width()-9,2,display.width()-9,iFontH-2,GxEPD_BLACK); //4.1v+
    }
    display.drawLine(0,iFontH+2,display.width()-1,iFontH+2,GxEPD_BLACK);
    
    if (bMenu){
      for(int y = 0;y<6;y++){
        //lets mark the selected item if its selected
        if(y==iRow){
            display.setFont(&FreeMonoBold12pt7b);
        }
        else{
          display.setFont(&FreeMono12pt7b);
        }
        display.setCursor(30,(iFontH*2+6) + (y * (iFontH*2)) + 6);
        display.print(sMenu[y]);
      }
    }
    else if (iMode==FILE){
      if(iFolders==0){
        listFolder();
        sprintf(filePath,"%s","");
        getPath(&FolderList[iFol]);
        listFile();
      }
      if(iFolders==-1&&iFiles==-1){
        display.setCursor(100,100);
        display.print("SD Card Fail!!");
      }
      else{
        display.drawLine(display.width()/2,iFontH+2,display.width()/2,display.height()-1,GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(5,(iFontH*2.5) + 6);
        if(iCol%2==0){
          display.print("> Folders <");
        }
        else{
          display.print("  Folders  ");
        }
        display.setCursor((display.width()/2)+5,(iFontH*2.5) + 6);
        if(iCol%2==0){
          display.print("  Files  ");
        }
        else{
          display.print("> Files <");
        }
        int iMax, iMin;
        if(iFol<(iPage/2)){
          //we are still on the first page
          iMin = 0;
          iMax = min(iPage,iFolders);
        }
        else if(iFol>(iFolders-(iPage/2))){
          //we are in the last page
          iMin = max(iFolders-iPage,0);//if there were 9 folders on a page big enough for 10, this would give us a negative
          iMax = iFolders;
        }
        else{
          //we are somewhere in the middle
          iMin = iFol - (iPage/2);
          iMax = iFol + (iPage/2);
        }

        for(int f = iMin;f<iMax;f++){
          if(iFol==f){
            display.setFont(&FreeMonoBold9pt7b);
          }
          else{
            display.setFont(&FreeMono9pt7b);
          }
          display.setCursor(10+(iFontW*FolderList[f].layer),(iFontH*4) + 6 + (iFontH * (f-iMin)));
          if(strlen(FolderList[f].name)>13){
            char shorter[15];
            sprintf(shorter,"%.13s..",FolderList[f].name);
            display.print(shorter);
          }
          else{
            display.print(FolderList[f].name);
          }
          if(FolderList[f].layer>0){
            display.drawLine(iFontW*FolderList[f].layer,(iFontH*4) + (iFontH * (f-iMin)),iFontW*FolderList[f].layer,(iFontH*4) + 4 + (iFontH * (f-iMin)),GxEPD_BLACK);
            display.drawLine(iFontW*FolderList[f].layer,(iFontH*4) + 4 + (iFontH * (f-iMin)),4+(iFontW*FolderList[f].layer),(iFontH*4) + 4 + (iFontH * (f-iMin)),GxEPD_BLACK);
          }
        }
        if(iFil<(iPage/2)){
          //we are still on the first page
          iMin = 0;
          iMax = min(iPage,iFiles);
        }
        else if(iFil>(iFiles-(iPage/2))){
          //we are in the last page
          iMin = max(iFiles-iPage,0);//if there were 9 files in a 10 file page, the number would be negative
          iMax = iFiles;
        }
        else{
          //we are somewhere in the middle
          iMin = iFil - (iPage/2);
          iMax = iFil + (iPage/2);
        }
        for(int f = iMin;f<iMax;f++){
          if(iFil==f){
            display.setFont(&FreeMonoBold9pt7b);
          }
          else{
            display.setFont(&FreeMono9pt7b);
          }
          display.setCursor(10+(display.width()/2),(iFontH*4) + 6 + (iFontH * (f-iMin)));
          
          if(strlen(sFileList[f])>15){
            char shorter[17];
            sprintf(shorter,"%.15s..",sFileList[f]);
            display.print(shorter);
          }
          else{
            display.print(sFileList[f]);
          }
        }
        
      }
    }
    else if (iMode==BEEP){
      display.setCursor(100,100);
      display.print("Make some noise!!");
    }
    else if (iMode==EDIT){
      if(strlen(fileName)==0){
        display.setCursor(40,100);
        display.print("Please Open A File To Edit");
      }
      else{
        //reload the current view from the big file
        getWindow();
        //draw the edit screen
        display.setCursor(0,(iFontH*2));
        //dump the entire file from sd card
        //display.print(lineCount);
        //display.print(fileBuffer);
        //draw the file from memory
        for(int y = 0;y<windowH;y++){
          for(int x = 0;x<windowW;x++){
            // if(fontLines[y][x]=='B'){
            //   display.setFont(&FreeMonoBold9pt7b);
            // }
            // else if(fontLines[y][x]=='I'){
            //   display.setFont(&FreeMonoOblique9pt7b);
            // }
            // else{
              display.setFont(&FreeMono9pt7b);
            // }
            display.setCursor(x*iFontW,(iFontH*2) + (y * iFontH) + 4);
            if(x==iCol-windowX && y==iRow-windowY){
              //we are on the same position as the cursor, lets do things different
              display.setTextColor(GxEPD_WHITE);
              display.fillRect(x*iFontW,(iFontH) + (y * iFontH) + 5,iFontW,iFontH,GxEPD_BLACK);
            }
            else{
              //normal text
              display.setTextColor(GxEPD_BLACK);

            }

            display.print(codeLines[y][x].val);
          }
        }
      }
    }
  }while (display.nextPage());
  //make it so...
  display.hibernate();
}

void readKeys() {

  byte bCount, bEvent, bEventCode, bEventStat;
  // put your main code here, to run repeatedly:

  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x03);//0x03 is the buffer count
  Wire.endTransmission();
  //read the contents
  Wire.requestFrom(KEYBOARD_ADDRESS,1);
  while(Wire.available() < 1);//sit and wait for response
  //read the value
  bCount = Wire.read();
  //Serial.print("event count ");
  //Serial.println(bCount);
  //if there is anything available, grab events
  while(bCount > 0){
    //show something on the screen so we can force it
    //display.setCursor(100,100);
    //display.print("event");
    //display.display();
    //the buffer is magic, when you read an event, it pops them
    //all back down the list so that you just keep reading the 
    //first item until you run out
    Wire.beginTransmission(KEYBOARD_ADDRESS);
    Wire.write(0x04);//0x04 is the fist event
    Wire.endTransmission();
    //read the contents
    Wire.requestFrom(KEYBOARD_ADDRESS,1);
    while(Wire.available() < 1);//sit and wait for response
    //read the value
    bEvent = Wire.read();
    bEventCode = bEvent & 0x7f;
    bEventStat = (bEvent & 0x80)>>7;

    //first 7 bytes are the key id, the last byte is the status
    //Serial.print("event data ");
    //Serial.print(bEvent,BIN);
    // Serial.print(" code ");
    // Serial.print(bEventCode);    
    // Serial.print(" stat ");
    // Serial.println(bEventStat,BIN);   

    if(bEventStat==1){
      if(iMode==BEEP && !bMenu && keyMap[bEventCode-1]!=60){
        tone(CL32_buz, bEventCode * 100); //Set the voltage to high and makes a noise
      }
      else{
        //display.print(lower[bEventCode-1]);
        if(lower[bEventCode-1]==0){
          if(keyMap[bEventCode-1]==42){//backspace
            if(iMode==EDIT && !bMenu){
              moveCursor(1,'W');
              putChar(8,codeLines[iRow][iCol].pos);
            }
          }
          if(keyMap[bEventCode-1]==40){//Enter
            if(bMenu){
              iMode = iRow +1;
              bMenu = false;
              iCol = 0;
              iRow = 0;
              iFiles = 0;
              iFolders = 0;
              // Immediately perform the action for the selected mode
              if (iMode==FILE){
                // No action needed on menu exit for FILE mode
              }
              else if(iMode==EDIT){
                // No action needed on menu exit for EDIT mode
              }
              else if(iMode==SAVE){
                saveFile();
                iMode = EDIT;
              }
              else if(iMode==NEW){
                newFile();
                iMode = EDIT;
              }
              else if(iMode==SET){
                showSettings();
                iMode = EDIT;
              }
            } 
            else {
              if (iMode==FILE){
                if(iCol%2==0){
                  iCol = 1;
                }
                else{
                  sprintf(fileName,"%s",sFileList[iFil]);
                  readFile();
                  iMode=EDIT;
                  bMenu=false;
                }
              }
              else if(iMode==EDIT){
                putChar(10,codeLines[iRow][iCol].pos+1);
                if(bCRLF){
                  putChar(13,codeLines[iRow][iCol].pos);
                }
              }
              else if(iMode==SAVE){
                saveFile();
                iMode = EDIT;
              }
              else if(iMode==NEW){
                newFile();
                iMode = EDIT;
              }
              else if(iMode==SET){
                showSettings();
                iMode = EDIT;
              }
            }
          }
          if(keyMap[bEventCode-1]==60){//Menu
            bMenu = !bMenu;
            iCol = 0;
            iRow = 0;
          }
          if(keyMap[bEventCode-1]==225){//shift
            iShift++;
            if(iShift>2){
              iShift=0;
            }
          }
          else if(keyMap[bEventCode-1]==79){//right
            if(iShift>0){
              moveCursor(windowW,'E');
              if(iShift==1){
                iShift=0;
              }
            }
            else{
              moveCursor(1,'E');
            }
          }
          else if(keyMap[bEventCode-1]==80){//left
            if(iShift>0){
              moveCursor(windowW,'E');
              if(iShift==1){
                iShift=0;
              }
            }
            else{
              moveCursor(1,'E');
            }
          }
          else if(keyMap[bEventCode-1]==81){//down
            if (iMode==FILE && !bMenu){
              if(iCol%2==0){
                iFol++;
                if(iFol>iFolders-1){
                  iFol=iFolders-1;
                }
                sprintf(filePath,"%s","");
                getPath(&FolderList[iFol]);
                listFile();
              }
              else{
                iFil++;
                if(iFil>iFiles-1){
                  iFil=iFiles-1;
                }
              }
            }
            else if (iMode==EDIT && !bMenu){
              if(iShift>0){
                moveCursor(windowH,'S');
                if(iShift==1){
                  iShift=0;
                }
              }
              else{
                moveCursor(1,'S');
              }
            }
            else{
              iRow++;
              if(iRow > 15){
                iRow=15;
              }
            }
          }
          else if(keyMap[bEventCode-1]==82){//up
            if (iMode==FILE && !bMenu){
              if(iCol%2==0){
                iFol--;
                if(iFol<0){
                  iFol=0;
                }
                sprintf(filePath,"%s","");
                getPath(&FolderList[iFol]);
                listFile();
              }
              else{
                iFil--;
                if(iFil<0){
                  iFil=0;
                }
              }
            }
            else if (iMode==EDIT && !bMenu){
              if(iShift>0){
                moveCursor(windowH,'N');
                if(iShift==1){
                  iShift=0;
                }
              }
              else{
                moveCursor(1,'N');
              }
            }
            else{
              iRow--;
              if(iRow < 0){
                iRow=0;
              }
            }
          }
        }   
        else{
          if(iMode==EDIT && !bMenu){
            if(iShift>0){
              putChar(upper[bEventCode-1],codeLines[iRow][iCol].pos);
              iCol++;
              if(iShift==1){
                iShift=0;
              }
            }
            else{
              putChar(lower[bEventCode-1],codeLines[iRow][iCol].pos);
              iCol++;
            }
            if(iCol>=windowW){
              iCol=0;
              iRow++;
            }
          }
        }
      }
    }
    else{
      if(iMode==BEEP && !bMenu){
        noTone(CL32_buz);//Sets the voltage to low and makes no noise
      }
    }
    bCount --;
  }
  //reset the fact there have been keys pressed
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x02);//0x02 is the interrupt register
  Wire.write(0xff);//setting a bit to 1 clears its interrupt 11111111
  Wire.endTransmission();
  if(iMode!=BEEP || bMenu){
    drawScreen(false);
  }
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin(CL32_sda,CL32_scl);
  // Serial port for debugging purposes
  Serial.begin(115200);
  //readFile();

  hspi.begin(CL32_sck, CL32_miso, CL32_mosi, CL32_epd_cs); // remap hspi for EPD (swap pins)
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  display.init(115200); 
  display.setRotation(3);
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap((display.width()/2) -148,(display.height()/2)-64,image_data_cl32_logo,296,128,GxEPD_WHITE,GxEPD_BLACK);
  display.display(true); // full update
  display.hibernate();
  //set up the keyboard chip
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x01);//0x01 is the config register
  Wire.write(0x19);//same as the driver 00011001
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x02);//0x02 is the interrupt register
  Wire.write(0xff);//setting a bit to 1 clears its interrupt 11111111
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x1d);//0x1d is the row gpio mode flag
  Wire.write(0xff);//all 8 rows 11111111
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x1e);//0x1e is the col gpio mode flag
  Wire.write(0xff);//first 8 cols 11111111
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x1f);//0x1d is the extra col mode flag
  Wire.write(0x01);//last 1 col 00000001
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x29);//0x29 is the row debounce
  Wire.write(0xff);//all 8 rows 11111111
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x2a);//0x2a is the column debounce
  Wire.write(0xff);//first 8 cols 11111111
  Wire.endTransmission();
  Wire.beginTransmission(KEYBOARD_ADDRESS);
  Wire.write(0x2b);//0x2b is the extra column debounce
  Wire.write(0x01);//last 1 col 00000001
  Wire.endTransmission();
  pinMode(CL32_int, INPUT);
  pinMode(CL32_buz, OUTPUT);
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0xc0);//setting for backup fallover
  Wire.write(0x20);// not used 0 - clock out 0 - backup switchover 10 - charger resistor (not needed) 00 - charger off 00
  Wire.endTransmission();
  // hour = 13;
  // minute = 42;
  // monthday = 14;
  // month = 1;
  // year = 2025;
  // setTime();
  iLoop = 0;
  iVolt = 0;
  analogReadResolution(12);
  iBatVolt = (analogRead(CL32_div)/500)*5.3;
  delay(1000);
  iMode = EDIT;
  bMenu = true;
  drawScreen(true);
}

void loop() {
  
  iVolt += analogRead(CL32_div);  // read the input pin
  iLoop++;
  if (iLoop%500==1){
    iBatVolt = (iVolt/500)*5.3;
    iVolt = 0; 
  }
  if(not digitalRead(CL32_int)){
    readKeys();
  }

  delay(100);
}
