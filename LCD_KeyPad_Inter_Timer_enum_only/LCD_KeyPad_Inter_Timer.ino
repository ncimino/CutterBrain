/**
 * Mino Solutions LLC.
 * Copyright 2014
 **/

#include <SoftwareSerial.h>
#include <Keypad.h>

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

#define PIN_TO_UART 0 // USB
#define PIN_FROM_UART 1 // USB
#define PIN_TO_LCD 2
#define PIN_FROM_LCD 3 // not connected
#define PIN_KEYPAD_ROW1 5 // -3 from keypad datasheet
#define PIN_KEYPAD_ROW2 10
#define PIN_KEYPAD_ROW3 9
#define PIN_KEYPAD_ROW4 7
#define PIN_KEYPAD_COL1 6
#define PIN_KEYPAD_COL2 4
#define PIN_KEYPAD_COL3 8
#define PIN_FROM_PHOTO 12
#define PIN_TO_LED 13 // hard connection

#define UART_BAUD_RATE 9600
#define LCD_BAUD_RATE 9600

#define LCD_CMD_CHAR 254
#define LCD_ROW1_BASE 128
#define LCD_ROW2_BASE 192
#define LCD_ROW_WIDTH 16

SoftwareSerial lcdSerial( PIN_FROM_LCD, PIN_TO_LCD );

SoftwareSerial uartSerial( PIN_FROM_UART, PIN_TO_UART );

char keys[ KEYPAD_ROWS ][ KEYPAD_COLS ] = {
 {'1','2','3'},
 {'4','5','6'},
 {'7','8','9'},
 {'*','0','#'}
};
byte rowPins[ KEYPAD_ROWS ] = { PIN_KEYPAD_ROW1, PIN_KEYPAD_ROW2, PIN_KEYPAD_ROW3, PIN_KEYPAD_ROW4 }; 
byte colPins[ KEYPAD_COLS ] = { PIN_KEYPAD_COL1, PIN_KEYPAD_COL2, PIN_KEYPAD_COL3 };
Keypad inKeypad = Keypad( makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS );

int photoValue = 0;


// Finite State Machine (FSM) */
// ??? http://stackoverflow.com/questions/133214/is-there-a-typical-state-machine-implementation-pattern
typedef enum State { S_INITIAL, S_MAIN, S_PRESETS, NUMBER_OF_STATES } StateType;
typedef struct FunctionPointer FunctionPointerType;
typedef StateType StateFunctionType( FunctionPointerType* pFunctionName );

StateType stateInitial( FunctionPointerType* pData );
StateType stateMain( FunctionPointerType* pData );
StateType statePresets( FunctionPointerType* pData );

StateFunctionType* const stateTable[ NUMBER_OF_STATES ] = {
  stateInitial, stateMain, statePresets
};
//*
StateType executeState( StateType kCurrentState, FunctionPointerType* pFunctionName ) 
{
  return stateTable[ kCurrentState ]( pFunctionName );
}
// Finite State Machine (FSM) - end  */

void clearLcd()
{
  clearLcdRow1();
  clearLcdRow2();
  moveCursor( LCD_ROW1_BASE );
}

void moveCursor( int location )
{
  lcdSerial.write( LCD_CMD_CHAR );
  lcdSerial.write( location );
}

void clearLcdRow1()
{
  moveCursor( LCD_ROW1_BASE );
  lcdSerial.write( "                " );
  moveCursor( LCD_ROW1_BASE );
}

void clearLcdRow2()
{
  moveCursor( LCD_ROW2_BASE );
  lcdSerial.write( "                " );
  moveCursor( LCD_ROW2_BASE );
}

void printLcdRow1( const char* output )
{
  clearLcdRow1();
  lcdSerial.write( output );
}

void printLcdRow2( const char* output )
{
  clearLcdRow2();
  lcdSerial.write( output );
}

void mainMenu()
{
  printLcdRow1("1. Presets");
  printLcdRow2("2. Cut Size"); 
  delay(2000);
  printLcdRow1("3. Settings");
  printLcdRow2("4. Decimal");
  delay(2000);
}

void initStateMachine()
{
  
}


void setup()
{
  //pinMode( PIN_TO_LED, OUTPUT );
  pinMode( PIN_FROM_PHOTO, INPUT );
  lcdSerial.begin( LCD_BAUD_RATE );
  uartSerial.begin( UART_BAUD_RATE ); 
  delay(500); // wait for display to boot up
}

void loop()
{
  printLcdRow1(" Mino Solutions");
  printLcdRow2(" LLC.");
  delay(1000);
  
  StateType kCurrentState = S_INITIAL;
  FunctionPointerType functionPointer;
  initStateMachine();
  
  while(1) {
    kCurrentState = executeState( kCurrentState, &functionPointer ); // will go into timer interrupt, just change state in here
   
    // replace with timer interrups:
    //  http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
    // Then use polling to decide state:
    const char key = inKeypad.getKey();
//    const char state = S_MAIN;
    const char state = (char)(((int)'0')+S_PRESETS);
    if ( key != NO_KEY ) {
      printLcdRow1( &state );
      //uartSerial.print( state );
      //uartSerial.println();
      clearLcdRow2();
    } 
  
/*    int val = digitalRead( PIN_FROM_PHOTO );   // read the input pin
    digitalWrite( PIN_TO_LED, val );    // sets the LED to the button's value
    printLcdRow1("Photo value: ");
    lcdSerial.print(val);
    lcdSerial.println();
    delay(500);*/
  }
    
  
  /*
  //int i = 0;
  
  while(1) {
    lcdSerial.write(254); // move cursor to beginning of first line
    lcdSerial.write(192);
    
    lcdSerial.write("Read:           "); // clear it

    lcdSerial.write(254); // move cursor to beginning of first line
    lcdSerial.write(198);
    
    char key = inKeypad.getKey();
    
    //i++;

    if (key != NO_KEY){
      lcdSerial.print(key);
    
      //lcdSerial.print(i);
      lcdSerial.print("key: ");
      lcdSerial.print(key);
      uartSerial.print("Key Pressed: ");
      uartSerial.print(key);   
      uartSerial.println();
    
      delay(500);
    }
  }
  //*/
  while(1); // wait forever
}
