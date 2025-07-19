#include "config.h"

char lower[80] = {'%' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,0   ,'9' ,'0' ,0   ,'[' ,']' ,'+' ,'"' ,'\'',0   ,0   ,
                  0   ,'q' ,'w' ,'e' ,'r' ,'t' ,'y' ,'u' ,'i' ,0   ,'o' ,'p' ,0   ,'(' ,')' ,'-' ,';' ,':' ,0   ,0   ,
                  0   ,'a' ,'s' ,'d' ,'f' ,'g' ,'h' ,'j' ,'k' ,0   ,'l' ,0   ,'#' ,'{' ,'}' ,'*' ,',' ,'.' ,0   ,0   ,
                  'z' ,'x' ,'c' ,'v' ,'b' ,' ' ,' ' ,'n' ,'m' ,0   ,0   ,0   ,0   ,'<' ,'>' ,'/' ,'\\','=' ,0   ,0   };
char upper[80] = {'%' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,0   ,'9' ,'0' ,0   ,'[' ,']' ,'+' ,'"' ,'\'',0   ,0   ,
                  0   ,'Q' ,'W' ,'E' ,'R' ,'T' ,'Y' ,'U' ,'I' ,0   ,'O' ,'P' ,0   ,'(' ,')' ,'-' ,';' ,':' ,0   ,0   ,
                  0   ,'A' ,'S' ,'D' ,'F' ,'G' ,'H' ,'J' ,'K' ,0   ,'L' ,0   ,'#' ,'{' ,'}' ,'*' ,',' ,'.' ,0   ,0   ,
                  'Z' ,'X' ,'C' ,'V' ,'B' ,' ' ,' ' ,'N' ,'M' ,0   ,0   ,0   ,0   ,'<' ,'>' ,'/' ,'\\','=' ,0   ,0   };
char keyMap[80] = {0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,42  ,0   ,0   ,0   ,0   ,0   ,58  ,0   ,
                   43  ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,40  ,0   ,0   ,0   ,0   ,0   ,59  ,0   ,
                   225 ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,82  ,0   ,0   ,0   ,0   ,0   ,0   ,60  ,0   ,
                   0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,0   ,80  ,81  ,79  ,0   ,0   ,0   ,0   ,0   ,61  ,0   };
const byte windowW = 34;
const byte windowH = 12;
CharData codeLines[12][34];
unsigned int windowX = 0, windowY = 0;
char fileBuffer[50000];
LineData lineNumbers[1000];
unsigned int fileSize = 0, lineCount = 0;
int lineLength = 0;
bool bCRLF = false;
char statusMsg[64] = "";
unsigned long statusMsgTime = 0;
const char *sMenu[] = {"Editor","Save", "Open File", "New File", "Settings", "Beeper"};
char fileName[20] = {""};
char filePath[50] = {"/"};
int iVolt = 0, iLoop = 0, iBatVolt = 0;
bool bMenu = true;
byte iMode = EDIT, iShift = 0;
int iRow = 0, iCol = 0;
byte iFontW = 12, iFontH = 12;
int iPage = 10;
SPIClass SPI_SD(HSPI), SPI_EPD(HSPI);
char sFileList[20][50] = {""};
FolderData FolderList[50];
int iFolders = 0, iFiles = 0;
int iFol = 0, iFil = 0;
struct tm CL32time;
SPIClass hspi(HSPI);
GxEPD2_BW<GxEPD2_290_GDEY029T71H, GxEPD2_290_GDEY029T71H::HEIGHT> display(
    GxEPD2_290_GDEY029T71H(CL32_epd_cs, CL32_dc, CL32_rst, CL32_bsy));