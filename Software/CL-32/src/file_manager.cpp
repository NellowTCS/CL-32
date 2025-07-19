#include "file_manager.h"
#include "editor.h"
#include <SD.h>

// Define maximum limits if not defined elsewhere
#ifndef MAX_FILES
#define MAX_FILES 100  // Adjust this value based on your memory constraints
#endif

#ifndef MAX_FOLDERS
#define MAX_FOLDERS 50  // Adjust this value based on your memory constraints
#endif

// Helper function to reinitialize SD card if needed
bool ensureSDReady() {
    static unsigned long lastSDCheck = 0;
    static bool sdWasWorking = false;
    static int retryCount = 0;
    
    // Always try to reinitialize if SD wasn't working, or every 5 seconds if it was
    if (!sdWasWorking || (millis() - lastSDCheck > 5000)) {
        lastSDCheck = millis();
        
        Serial.println("Checking SD card accessibility...");
        
        // End any existing SD session first
        SD.end();
        delay(200); // Give more time for cleanup
        
        // Toggle CS pin to reset state
        digitalWrite(CL32_sd_cs, HIGH);
        delay(50);
        digitalWrite(CL32_sd_cs, LOW);
        delay(50);
        digitalWrite(CL32_sd_cs, HIGH);
        delay(100);
        
        // Try different SPI speeds in order of preference
        uint32_t speeds[] = {4000000, 1000000, 400000}; // 4MHz, 1MHz, 400kHz
        bool initialized = false;
        
        for (int i = 0; i < 3 && !initialized; i++) {
            Serial.print("Attempting SD init at ");
            Serial.print(speeds[i]);
            Serial.println(" Hz");
            
            initialized = SD.begin(CL32_sd_cs, SPI, speeds[i]);
            if (!initialized) {
                delay(500);
            }
        }
        
        if (initialized) {
            // Double-check by trying to access root directory
            if (SD.exists("/")) {
                Serial.println("SD card initialized and accessible");
                sdWasWorking = true;
                retryCount = 0;
                return true;
            } else {
                Serial.println("SD initialized but root directory not accessible");
                initialized = false;
            }
        }
        
        if (!initialized) {
            retryCount++;
            Serial.print("SD card initialization failed (attempt ");
            Serial.print(retryCount);
            Serial.println(")");
            sdWasWorking = false;
            return false;
        }
    }
    
    return sdWasWorking;
}

// Hardware diagnostic function - call this in setup() or when SD fails
void diagnoseSD() {
    Serial.println("=== SD Card Diagnostic ===");
    Serial.print("CS Pin: ");
    Serial.println(CL32_sd_cs);
    Serial.print("EPD CS Pin: ");
    Serial.println(CL32_epd_cs);
    
    // Check pin states
    Serial.print("SD CS state: ");
    Serial.println(digitalRead(CL32_sd_cs) ? "HIGH" : "LOW");
    Serial.print("EPD CS state: ");
    Serial.println(digitalRead(CL32_epd_cs) ? "HIGH" : "LOW");
    
    // Test different initialization approaches
    Serial.println("Testing SD initialization approaches...");
    
    // Approach 1: Default settings
    SD.end();
    delay(500);
    digitalWrite(CL32_epd_cs, HIGH);
    digitalWrite(CL32_sd_cs, HIGH);
    delay(100);
    
    if (SD.begin(CL32_sd_cs)) {
        Serial.println("✓ Default init successful");
    } else {
        Serial.println("✗ Default init failed");
        
        // Approach 2: With explicit SPI and lower speed
        SD.end();
        delay(500);
        if (SD.begin(CL32_sd_cs, SPI, 400000)) {
            Serial.println("✓ Low-speed init successful");
        } else {
            Serial.println("✗ Low-speed init failed");
        }
    }
    
    Serial.println("=== End Diagnostic ===");
}

void saveFile() {
    File curFile;
    char fullFileName[200];
    
    // Build full path properly
    if (strlen(filePath) > 1) {
        snprintf(fullFileName, sizeof(fullFileName), "%s/%s", filePath, fileName);
    } else {
        snprintf(fullFileName, sizeof(fullFileName), "/%s", fileName);
    }
    
    Serial.print("Saving file: ");
    Serial.println(fullFileName);
    
    // Ensure e-paper display is deselected and SD is selected
    digitalWrite(CL32_epd_cs, HIGH);
    delay(20); // Increased delay for signal stability
    digitalWrite(CL32_sd_cs, LOW);
    delay(10);
    Serial.println("SD CS LOW, EPD CS HIGH for saveFile");
    
    // Try multiple times to get SD working
    bool sdReady = false;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (ensureSDReady()) {
            sdReady = true;
            break;
        }
        Serial.print("SD retry attempt ");
        Serial.println(attempt + 1);
        delay(1000); // Wait between attempts
    }
    
    if (!sdReady) {
        Serial.println("SD card not accessible after multiple attempts");
        sprintf(statusMsg, "SD Not Ready");
        statusMsgTime = millis();
        digitalWrite(CL32_sd_cs, HIGH);
        
        // Run diagnostic if SD keeps failing
        diagnoseSD();
        return;
    }
    
    // Remove existing file and create new one to ensure clean write
    if (SD.exists(fullFileName)) {
        if (!SD.remove(fullFileName)) {
            Serial.println("Warning: Could not remove existing file");
        }
    }
    
    curFile = SD.open(fullFileName, FILE_WRITE);
    if (curFile) {
        size_t written = curFile.write((uint8_t*)fileBuffer, fileSize);
        curFile.close();
        
        if (written == fileSize) {
            sprintf(statusMsg, "Saved: %s", fileName);
            Serial.print("File saved successfully, wrote ");
            Serial.print(written);
            Serial.println(" bytes");
        } else {
            sprintf(statusMsg, "Save Incomplete");
            Serial.print("Warning: Expected to write ");
            Serial.print(fileSize);
            Serial.print(" bytes, actually wrote ");
            Serial.println(written);
        }
    } else {
        Serial.println("Failed to open file for writing");
        Serial.println("Running SD diagnostic...");
        diagnoseSD();
        sprintf(statusMsg, "Save Failed");
    }
    statusMsgTime = millis();
    digitalWrite(CL32_sd_cs, HIGH);
    Serial.println("SD CS HIGH after saveFile");
}

void newFile() {
    memset(fileBuffer, 0, sizeof(fileBuffer));
    fileSize = 0;
    strcpy(fileName, "newfile.txt");
    strcpy(filePath, "/"); // Use root directory
    windowX = windowY = iRow = iCol = 0;
    getLines();
    sprintf(statusMsg, "New file started");
    statusMsgTime = millis();
    Serial.println("New file created");
}

void listFile() {
    iFiles = 0;
    char dirPath[200];
    
    // Use current filePath, default to root if empty
    if (strlen(filePath) == 0 || strcmp(filePath, "/sd") == 0) {
        strcpy(dirPath, "/");
    } else {
        strcpy(dirPath, filePath);
    }
    
    Serial.print("Listing files in: ");
    Serial.println(dirPath);
    
    // Ensure e-paper display is deselected
    digitalWrite(CL32_epd_cs, HIGH);
    delay(10); // Small delay for signal stability
    digitalWrite(CL32_sd_cs, LOW);
    Serial.println("SD CS LOW, EPD CS HIGH for listFile");
    
    // Check and reinitialize SD if needed
    if (!ensureSDReady()) {
        Serial.println("SD card not accessible in listFile");
        sprintf(statusMsg, "SD Not Ready");
        statusMsgTime = millis();
        digitalWrite(CL32_sd_cs, HIGH);
        return;
    }
    
    File curFile = SD.open(dirPath);
    if (curFile) {
        while (true) {
            File entry = curFile.openNextFile();
            if (!entry) {
                break;
            }
            if (!entry.isDirectory()) {
                // Add bounds checking back
                if (iFiles < MAX_FILES) {
                    strcpy(sFileList[iFiles], entry.name());
                    iFiles++;
                    Serial.print("Found file: ");
                    Serial.println(sFileList[iFiles - 1]);
                } else {
                    Serial.println("Maximum file limit reached");
                    entry.close();
                    break;
                }
            }
            entry.close();
        }
        curFile.close();
        Serial.print("Total files found: ");
        Serial.println(iFiles);
    } else {
        Serial.print("Failed to open directory: ");
        Serial.println(dirPath);
        sprintf(statusMsg, "Dir Open Fail");
        statusMsgTime = millis();
    }
    digitalWrite(CL32_sd_cs, HIGH);
    Serial.println("SD CS HIGH after listFile");
}

void saveFolder(File dir, int depth, FolderData* parent) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        if (entry.isDirectory()) {
            // Add bounds checking back
            if (iFolders < MAX_FOLDERS) {
                FolderList[iFolders].layer = depth;
                strcpy(FolderList[iFolders].name, entry.name());
                FolderList[iFolders].parent = parent;
                iFolders++;
                Serial.print("Found folder: ");
                Serial.println(FolderList[iFolders - 1].name);
                saveFolder(entry, depth + 1, &FolderList[iFolders - 1]);
            } else {
                Serial.println("Maximum folder limit reached");
                entry.close();
                break;
            }
        }
        entry.close();
    }
}

void listFolder() {
    iFolders = 0;
    // Ensure e-paper display is deselected
    digitalWrite(CL32_epd_cs, HIGH);
    delay(10); // Small delay for signal stability
    digitalWrite(CL32_sd_cs, LOW);
    Serial.println("SD CS LOW, EPD CS HIGH for listFolder");
    
    // Check and reinitialize SD if needed
    if (!ensureSDReady()) {
        Serial.println("SD card not accessible in listFolder");
        sprintf(statusMsg, "SD Not Ready");
        statusMsgTime = millis();
        digitalWrite(CL32_sd_cs, HIGH);
        return;
    }
    
    File curFile = SD.open("/"); // Always start from root
    if (curFile) {
        Serial.println("Listing folders");
        if (iFolders < MAX_FOLDERS) {
            FolderList[iFolders].layer = 0;
            strcpy(FolderList[iFolders].name, "/");
            FolderList[iFolders].parent = NULL;
            iFolders++;
            saveFolder(curFile, 1, &FolderList[iFolders - 1]);
        }
        curFile.close();
        Serial.print("Total folders found: ");
        Serial.println(iFolders);
    } else {
        Serial.println("Failed to open root directory /");
        sprintf(statusMsg, "Root Dir Fail");
        statusMsgTime = millis();
    }
    digitalWrite(CL32_sd_cs, HIGH);
    Serial.println("SD CS HIGH after listFolder");
}

void getPath(FolderData* input) {
    // Clear filePath first
    strcpy(filePath, "");
    
    // Build path recursively
    if (input->parent != NULL && input->parent->parent != NULL) {
        getPath(input->parent);
        if (strlen(filePath) > 1) { // Don't add slash if already at root
            strcat(filePath, "/");
        }
    }
    
    if (strcmp(input->name, "/") != 0) { // Don't add root slash again
        strcat(filePath, input->name);
    }
    
    // Ensure we have at least root path
    if (strlen(filePath) == 0) {
        strcpy(filePath, "/");
    }
    
    Serial.print("Constructed path: ");
    Serial.println(filePath);
}

void readFile() {
    File curFile;
    char fullFileName[200];
    
    // Build full path properly
    if (strlen(filePath) > 1) {
        snprintf(fullFileName, sizeof(fullFileName), "%s/%s", filePath, fileName);
    } else {
        snprintf(fullFileName, sizeof(fullFileName), "/%s", fileName);
    }
    
    Serial.print("Reading file: ");
    Serial.println(fullFileName);
    
    // Ensure e-paper display is deselected
    digitalWrite(CL32_epd_cs, HIGH);
    delay(10); // Small delay for signal stability
    digitalWrite(CL32_sd_cs, LOW);
    Serial.println("SD CS LOW, EPD CS HIGH for readFile");
    
    // Check and reinitialize SD if needed
    if (!ensureSDReady()) {
        Serial.println("SD card not accessible in readFile");
        sprintf(statusMsg, "SD Not Ready");
        statusMsgTime = millis();
        digitalWrite(CL32_sd_cs, HIGH);
        return;
    }
    
    if (SD.exists(fullFileName)) {
        curFile = SD.open(fullFileName, FILE_READ);
        if (curFile) {
            fileSize = curFile.size();
            if (fileSize < sizeof(fileBuffer)) {
                // More efficient reading
                size_t bytesRead = curFile.readBytes(fileBuffer, fileSize);
                fileBuffer[fileSize] = '\0'; // Null terminate for safety
                curFile.close();
                
                if (bytesRead == fileSize) {
                    windowX = windowY = iRow = iCol = 0;
                    getLines();
                    Serial.println("File read successfully");
                    sprintf(statusMsg, "Loaded: %s", fileName);
                } else {
                    Serial.print("Warning: Expected to read ");
                    Serial.print(fileSize);
                    Serial.print(" bytes, actually read ");
                    Serial.println(bytesRead);
                    sprintf(statusMsg, "Read Incomplete");
                }
            } else {
                curFile.close();
                Serial.println("File too large for buffer");
                sprintf(statusMsg, "File Too Large");
            }
        } else {
            Serial.println("Failed to open file for reading");
            sprintf(statusMsg, "Read Fail");
        }
    } else {
        fileSize = 0;
        memset(fileBuffer, 0, sizeof(fileBuffer));
        Serial.println("File does not exist");
        sprintf(statusMsg, "File Not Found");
    }
    statusMsgTime = millis();
    digitalWrite(CL32_sd_cs, HIGH);
    Serial.println("SD CS HIGH after readFile");
}