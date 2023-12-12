#pragma #once
#include "ArdeckDisplay.h"
#include "WString.h"

ArdecDisplay::ArdecDisplay(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows) : LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows){
    
}


void ArdecDisplay::cleanupRow(uint8_t r){
      setCursor(0,r);
      // 20 char line cleanup FIXME: must depend on _cols
      print(F("                    "));
      setCursor(0,r);
}