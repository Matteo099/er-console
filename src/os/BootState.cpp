#include "BootState.h"

BootState gBootState = {0, "Starting"};
SemaphoreHandle_t gBootStateMutex = xSemaphoreCreateMutex();
