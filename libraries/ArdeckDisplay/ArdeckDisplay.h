#pragma #once
#include "LiquidCrystal_I2C.h"


class ArdecDisplay : public LiquidCrystal_I2C {

public:
    ArdecDisplay(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows);
    void cleanupRow(uint8_t r);
};