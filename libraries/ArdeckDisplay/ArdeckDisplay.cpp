#pragma once
#include "ArdeckDisplay.h"
#include "WString.h"
#include "Arduino_FreeRTOS.h"


ArdecDisplay::ArdecDisplay(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) : LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows){
    // Init support queue
    // Global variables use 806 bytes (9%) of dynamic memory, leaving 7386 bytes for local variables. Maximum is 8192 bytes.
    const int QueueSize= lcd_cols*1; // Support at most 1 row. 
    displayQueue = xQueueCreate( QueueSize, 
                                 sizeof( struct AMessage * ) );

    configASSERT( displayQueue != NULL );

}

void ArdecDisplay::updateDisplayQueue(){
    struct AMessage *pxRxedPointer;
    if( xQueueReceive( displayQueue,
                         &( pxRxedPointer ),
                         ( TickType_t ) 0 ) == pdPASS )
    {
         /* *pxRxedPointer now points to xMessage. */
         cleanupRow(0);
         // TODO: Write a char and go to the next until the end


    }
}

/** Low level function used to print on the display: must be redefined to queue bytes and eventually block
 * 
 */
inline size_t ArdecDisplay::write(uint8_t value){
    return LiquidCrystal_I2C::write(value);
    //send(value, Rs);
	//return 1;
}

void ArdecDisplay::cleanupRow(uint8_t r){
      setCursor(0,r);
      // 20 char line cleanup FIXME: must depend on _cols
      print(F("                    "));
      setCursor(0,r);
}