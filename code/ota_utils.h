#ifndef OTA_UTILS_H
#define OTA_UTILS_H

#include <M5StickCPlus2.h>
#include "config.h"

void startOtaUpdate();
void handleOtaUpdateLoop();
void stopOtaUpdate();

#endif // OTA_UTILS_H
