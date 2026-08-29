#include "FreeRTOS.h"
#include "task.h"
void vApplicationStackOverflowHook(TaskHandle_t task, char *name) { (void)task;(void)name; taskDISABLE_INTERRUPTS(); for(;;){} }
void vApplicationMallocFailedHook(void) { taskDISABLE_INTERRUPTS(); for(;;){} }
