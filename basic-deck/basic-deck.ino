/*
  Blink evolution is here!
  The system will sleep until you press the button on pin wakeupButton
  Then a magic led dance will start

Schematics: 
  Connect pin 2 -> Button -> GND

  3 Leds on Pin
  TBD 12,11,10

  Operative Leds to ?

  LCD TO pin 20,21 for I2C communicartion


Interaction with LCD is very delicate: AVOID printing inside interrupt

Feature:
+ LCD Logging
+ 2-lines message display with scrolling


 */

// #define DEBUG_LOG yeppa
#define LCD_DEBUG_LOG yeppa
#include "Arduino_FreeRTOS.h"
#include "jj-log.h"
#include <avr/sleep.h>
#include "mem_free.h"


/// CONSTANTS 
const uint16_t SmallStackSize=64*2;



//define task handles
TaskHandle_t majorTaskHandler, blinkTaskHandler;

const TaskHandle_t* listOfHandler2Monitor[]={
      &majorTaskHandler, &blinkTaskHandler     
};

enum DeckMode: uint8_t { Init=0, WOPR=1, };
DeckMode CurrentMode=Init;
const uint8_t BlinkLed=13;

// CHECK ON MEGA The I2C pins on the board are 20 and 21.
// Ref lib https://github.com/johnrickman/LiquidCrystal_I2C 

#include "ArdeckDisplay.h"

ArdecDisplay lcd(0x3F,20,4); // set the LCD address to 0x27 for a 20 chars and 4 line display


// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
const int aliveLed = 13;
const int wakeupButton = 2; // MEGA PINS for interrupt  are 2, 3, 18, 19, 20, 21
#ifdef LCD_DEBUG_LOG
const int DelayTime = 33;  // Lcd debug will slow down a lot
#else
const int DelayTime = 44;   // 44 is good
#endif
/////////////////////////////////////////////////////////////////
char runningMode = 1; // 1 = Start running, 0 = Start Sleeping
/////////////////////////////////////////////////////////////////


int stripeLeds[] = { 12, 13,   9  };
const int stripeLength=sizeof(stripeLeds)/sizeof(stripeLeds[0]);

const int dual_mode_period=5;
/**
 * LCD Status message printing
 * Also support a minimal scrolling mechianics
 * It is a dual mode lcd
 */
inline void sayLcdMsg(String str){
  #ifdef LCD_DEBUG_LOG
  static int8_t super_counter=1;
  static String prevMsg1="";
  static String prevMsg2="";

  // Clearing lcd is a slow procedure but because we need to scroll all display it seems a good move
  // to reduce code size and also increment speed
  lcd.clear();
  if ((super_counter % dual_mode_period) == 0) {
    lcd.setCursor(0,1);
    lcd.print(F("Tnt Box 2023"));
    lcd.print( (float)(millis() / 1000));
    lcd.print(F("s"));
    lcd.setCursor(0,2);    
    String memFree=String(F("Mem:"));
    memFree.concat(freeMemory());  
    lcd.print(memFree);

    lcd.setCursor(0,3);
    lcd.print(F("by Giovanni Giorgi"));
  }else {

    String finalMsg="";
    finalMsg.concat(super_counter);
    finalMsg.concat(" ");
    finalMsg.concat(str);

    
    lcd.setCursor(0,3); 
    lcd.print(finalMsg);  

    // Scrolling logic:
    
    lcd.setCursor(0,2);
    lcd.print(prevMsg1);

    
    lcd.setCursor(0,1);
    lcd.print(prevMsg2);

    prevMsg2=prevMsg1;
    prevMsg1=finalMsg;
    // Print mem free on top

    lcd.setCursor(0,0);
    String memFree=String(F("v1.2.3 Mem:"));
    memFree.concat(freeMemory());
    lcd.print(memFree);
  
  }

 

  // Ensure super counter value never exceed 99
  super_counter= (super_counter+1) % 100;


  #else
    return;
  #endif;
  
}

// Use String constructor to "eat" static strings
#define say(c)      sayLcdMsg(String(F(c)));


// debug will be defined as empty function when debug mode is off
#define debug(c)    sayLcdMsg(String(F(c)));


///// Sleep Mechianics
void sleepModeCheckCallback()
{
  if (runningMode == 1)
  {
    runningMode = 0;
    l("* Requesting Sleep...");
  }
  // Disable interrupt to be one time fire
  detachInterrupt(digitalPinToInterrupt(wakeupButton));
}


void wakeUp(){
  // lcd.backlight();
  l("* COOOME BACK !");
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(wakeupButton));
}

inline void lcdInit(){
  lcd.init(); 
  lcd.backlight();
  //lcd.autoscroll();
  lcd.home(); //.setCursor(0,0);
  lcd.blink_on();
  //lcd.cursor_on();
  lcd.print(F("2024 ArdeK "));  
  // CR does not work and jumps from line 0 to line 3 and wrap back on line 2
  // lcd.print(F("Verylong line printing test should workz"));
  lcd.setCursor(0,3);
  lcd.print("Booting");
  //say("Ready.");
}





/** Retrun true on sleep detect */
inline bool checkForSleep(){
  if (runningMode == 0){
    /* For the sleep mode refer to https://thekurks.net/blog/2018/1/24/guide-to-arduino-sleep-mode
     * IT IS VERY VERY IMPORTANT to have some delay to let stabilize it
     * Also it is unclear if stack is correctly preserved after wakeup (it seems)
     */
    l("...Sleeping...");
    
    // Remove old interrupts if any
    detachInterrupt(digitalPinToInterrupt(wakeupButton));

    say("...Sleeping...");    
    delay(200);

    delay(340);

    // Turn off all
    for(int i=0; i<stripeLength; i++) {
        analogWrite(stripeLeds[i], 0 );
    }
    lcd.noBacklight();



    sleep_enable();
    attachInterrupt(digitalPinToInterrupt(wakeupButton), wakeUp, LOW);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    delay(1000);
    sleep_cpu();
    l("WAAAKE UP2 Setting up sleep interrupt again");
    // Try to reinit lcd and show stuff    
    runningMode = 1;
    delay(400);

    lcdInit();    
    
    attachInterrupt(digitalPinToInterrupt(wakeupButton), sleepModeCheckCallback, CHANGE);    
    return true;
  }else{
    return false;
  }
}




///////// TASKS 
void TaskBlink(void *_unused){
  pinMode(BlinkLed, OUTPUT);
  for(;;){
    digitalWrite(BlinkLed, HIGH);    
    // FIXME: Lcd access is not thread safe, a special thread must be set-up to access to this resource in a consistent way
    //cleanupRow(1);
    //lcd.print("Blink");
    delay(100);
    
    digitalWrite(BlinkLed, LOW);    
    // cleanupRow(1);
    // lcd.print("$Blink");
    delay(100);

  }
}
/**
 * This tasks do two things: show a REPORT AND Manage LCD Screen
 * Tasks can ask to show things on the first 3 row, last row is used for system status
 */
void TaskSystemStatus(void *pvParameters){
  const int delayMs=1200;
  for(;;){

    // This task uses 2 lines to present current task and ticks
    // FIXME: Reduce to 1 line
    
    lcd.updateDisplayQueue();
    for(auto handler: listOfHandler2Monitor){
      lcd.cleanupRow(3);      
      lcd.print(pcTaskGetName(*handler)); 
      lcd.print(F(" -> "));
      lcd.print(uxTaskGetStackHighWaterMark(*handler));
      lcd.print(F(" "));
      delay(delayMs);
    }

    // Last line present status
    // cleanupRow(3);
    lcd.setCursor(0,3); // Optimized because this row is always very long
    lcd.print("Tsks:");
    lcd.print(uxTaskGetNumberOfTasks());
    lcd.print(" Ticks:");
    lcd.print(xTaskGetTickCount());
    delay(delayMs);
  }
}
//////////////////////////////////////////////////////////////////////////////
// the setup routine runs once when you press reset:
void setup()
{
  lcdInit();
  lcd.backlight();
  // initialize the digital pin as an output.  
  // pinMode(aliveLed, OUTPUT);
  // pinMode(wakeupButton, INPUT_PULLUP);

  // for(int i=0; i<stripeLength; i++) {
  //   pinMode(stripeLeds[i], OUTPUT);
  //   analogWrite(stripeLeds[i], 0 );
  // }

  // if(runningMode == 1){
  //   // if the initial state is 1 it means we are not sleeping so we must enable the interrupt
  //   attachInterrupt(digitalPinToInterrupt(wakeupButton), sleepModeCheckCallback, CHANGE);
  // }
  lcd.print(".");
  xTaskCreate(TaskSystemStatus   
      , "SysStat"
      , 200
      , NULL
      , configMAX_PRIORITIES-1  // Highest priority to better track down memory
      , &majorTaskHandler /*NULL*/);

  xTaskCreate(TaskBlink,"BLK",SmallStackSize,NULL,0, &blinkTaskHandler);
  lcd.print(".");
  vTaskStartScheduler();
}




// Local variables:
// mode:c++
// mode:company
// End:

