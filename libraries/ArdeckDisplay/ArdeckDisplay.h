#pragma once
#ifndef ArdecDisplay_h
#define ArdecDisplay_h
#include "LiquidCrystal_I2C.h"
#include "Arduino_FreeRTOS.h"
#include "queue.h"



struct AMessage
{    
    char c;
};

/**
 * A display which does not print directly on it but queue messages.
 * A specific "dequeue" task can then be planned 
 */
class ArdecDisplay : public LiquidCrystal_I2C {

public:
    ArdecDisplay(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows);
    void cleanupRow(uint8_t r);

    void updateDisplayQueue();
    virtual size_t write(uint8_t);
private:
    // Ref https://www.freertos.org/xQueueCreate.html
    QueueHandle_t displayQueue = NULL;
};
#endif