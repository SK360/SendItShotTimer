#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <M5StickCPlus2.h>
#include "config.h" // For enums if needed by display logic, and constants

void displayBootScreen(const char* line1a, const char* line1b, const char* line2);
void displayMenu(const char* title, const char* items[], int count, int selection, int scrollOffset);
void displayTimingScreen(float elapsedTime, int count, float lastSplit);
void displayStoppedScreen();
void displayShotReviewScreen(int index);
void displayEditScreen();
void displayCalibrationScreen(const char* title, float peakValue, const char* unit);
void displayDeviceStatusScreen();
void displayListFilesScreen();
void displayDryFireReadyScreen();
void displayDryFireRunningScreen(bool waiting, int beepNum, int totalBeeps);
void drawLowBatteryIndicator();
String getUpButtonLabel();
String getDownButtonLabel();
void displayBluetoothScanResults(); // Moved here from bluetooth_utils for logical grouping

#endif // DISPLAY_UTILS_H
