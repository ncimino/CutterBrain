/**
 * Mino Solutions LLC.
 * Copyright 2014
 **/

#include <SoftwareSerial.h>
#include <Keypad.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define DEBUG 1

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

#define STATE_INITIAL   0
#define STATE_MAIN      1
#define STATE_PRESETS   2
#define STATE_SIZE      3
#define MAX_MENU_COUNT  10      

SoftwareSerial lcdSerial( PIN_FROM_LCD, PIN_TO_LCD );

SoftwareSerial uartSerial( PIN_TO_UART, PIN_FROM_UART );

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
//typedef enum StateType { S_INITIAL, S_MAIN, S_PRESETS, NUMBER_OF_STATES } StateType;
//typedef struct FunctionPointer FunctionPointerType;
//typedef StateType StateFunctionType( FunctionPointerType* pFunctionName );
//enum StateType { S_INITIAL, S_MAIN, S_PRESETS, NUMBER_OF_STATES };
//typedef enum { S_INITIAL, S_MAIN, S_PRESETS, NUMBER_OF_STATES } StateType;

//typedef enum StateType { 
//  S_INITIAL,
//  S_MAIN,
//  S_PRESETS,
//  NUMBER_OF_STATES
//};
//
//StateType g_kCurrentState;
int g_iCurrentState = STATE_INITIAL;
int g_iNextState = STATE_MAIN;
int g_iCurrentMenuPageCounter = 0;
char** g_menuItems;

const char* getStateSting( int _iState )
{
  switch( _iState ) {
    case STATE_INITIAL:
      return "Initial";
      break;
    case STATE_MAIN:
      return "Main";
      break;
    case STATE_PRESETS:
      return "Presets";
      break;
    default:
      return "";
      break;
  }
}

//StateType stateInitial( FunctionPointerType* pData );
//StateType stateMain( FunctionPointerType* pData );
//StateType statePresets( FunctionPointerType* pData );

//StateFunctionType* const stateTable[ NUMBER_OF_STATES ] = {
//  stateInitial, stateMain, statePresets
//};
//*
//StateType executeState( StateType g_kCurrentState, FunctionPointerType* pFunctionName ) 
//{
//  return stateTable[ g_kCurrentState ]( pFunctionName );
//}
// Finite State Machine (FSM) - end  */

/*
* LCD and UART
*/

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

// absolutely required, even if it works without this the configuration will change randomly and 
// this is required to control that configuration (would cause a seeminly sporadic failure)
void setLcdBacklight( float brightness ) {
  int level = 128 + ( int ) (  brightness / 100.0 * 30.0 ); // 128 is off and 157 is max
  if ( level > 157 ) level = 157;
  if ( level < 128 ) level = 128;
  lcdSerial.write(124); // command character
  lcdSerial.write(level); // command character
}

void printLcdRow1( const char* output )
{
  clearLcdRow1();
  lcdSerial.write( output );
  if ( DEBUG )
    printUart( strcat( "Row1: ", output ) );
}

void printLcdRow2( const char* output )
{
  clearLcdRow2();
  lcdSerial.write( output );
  if ( DEBUG )
    printUart( strcat( "Row2: ", output ) );
}

void printUart( const char* output )
{
  uartSerial.print( output );
  uartSerial.println();
}

/*
* State Machine
*/

// State Decoder, called on key press

void decodeState( char key )
{
  switch ( g_iCurrentState ) {
    case STATE_INITIAL:
      g_iNextState = STATE_MAIN;
      break;
    case STATE_MAIN:
      decodeMain( key );
      break;
    case STATE_PRESETS:
      decodePresets( key );
      break;
    default:
      printLcdRow1( "Unknown State:" );
      printLcdRow2( &key );
      break;
  }
//  char prev = (char) g_iPreviousState;
//  printUart( &prev );
//  char current = (char) g_iCurrentState;
//  printUart( &current );
}

// Preset Menu & Decoder

void decodePresets( char key )
{
  switch ( key ) {
    case '1':
      g_iNextState = STATE_MAIN;
      break;
    default:
      break;
  }
}

enum presetItems { PPRESETS, PCUT_SIZE, PSETTINGS, PDECIMAL, TOTAL_PRESET_ITEMS }; // enum is just a sequential list of items starting from 0
char* presetMenuItems[ TOTAL_PRESET_ITEMS ] = { "pPresets", "pCut Size", "pSettings", "pDecimal" };

void menuPreset()
{
  g_menuItems = presetMenuItems;
  startMenu();
}

/*
* Main Menu & Decoder
*/
enum mainItems { PRESETS, CUT_SIZE, SETTINGS, DECIMAL, TOTAL_MAIN_ITEMS }; // enum is just a sequential list of items starting from 0
char* mainMenuItems[ TOTAL_MAIN_ITEMS ] = { "1. Presets", "2. Cut Size", "3. Settings", "4. Decimal" };

void decodeMain( char key )
{
  switch ( key ) {
    case '1':
      g_iNextState = STATE_PRESETS;
      break;
    default:
      break;
  }
}

void menuMain()
{
  g_menuItems = mainMenuItems;
  startMenu();
}


/* 
* Start Menu Listening and Disply
*/

enum menuItems { M_INITIAL, M_MAIN, M_PRESETS, M_TOTAL_MENUS };
char* menus[ M_TOTAL_MENUS ] = { "Initial", "Main", "Presets" };

void startMenu()
{
  g_iCurrentMenuPageCounter = 0;
  if ( DEBUG ) {
    printUart( strcat( "Starting Menu: ", menus[ g_iCurrentState ] ) ); // Current state should use menuItems type??? everywhere?
    printUart( strcat( "Current Menu Page Counter: ", String(g_iCurrentMenuPageCounter).c_str() ) ); 
  }
  displayMenu();
  startTimerInterrupts();
  startKeyListner();
}

void updateMenu()
{
  ++g_iCurrentMenuPageCounter;
  displayMenu();
  startTimerInterrupts();
  startKeyListner();
}

void displayMenu()
{
  if ( ( g_iCurrentMenuPageCounter % MAX_MENU_COUNT ) == 0 ) {
    int iIndex = g_iCurrentMenuPageCounter / MAX_MENU_COUNT;
    printLcdRow1( g_menuItems[ iIndex ] );
    printLcdRow2( g_menuItems[ iIndex + 1 ] );
  }
}

/*
* Splash Screen
*/

void splashScreen()
{
  printLcdRow1(" Mino Solutions");
  printLcdRow2(" LLC.");
  delay(1000);
}


/*
* Timer Interrupts
*/

void startTimerInterrupts()
{
  // initialize Timer1
  cli();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B

  //TIMSK1 = (1 << TOIE1); 
   
  //TCCR1A |= 0;   
  // not required since WGM11:0, both are zero (0)
   
  //TCCR1B |= (1 << WGM12)|(1 << CS11)|(1 << CS10);   
  // Mode = CTC, Prescaler = 64
 
  // initialize counter
  //TCNT1 = 0;

  // initialize compare value
  //OCR1A = 256;
 
  // set compare match register to desired timer count:
  //OCR1A = 15624;
      
  // turn on CTC mode:
  //TCCR1B |= (1 << WGM12); // only set for compare interrupt
  
  // Set CS10 and CS12 bits for 1024 prescaler:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS11);
  //TCCR1B |= (1 << CS12);
  
  // enable timer compare interrupt:
  //TIMSK1 |= (1 << OCIE1A);
  TIMSK1 = (1 << TOIE1); 
  
  // enable global interrupts:
  sei();
}  

// Interrupt Service Routine

ISR( TIMER1_OVF_vect ) 
{
//  if ( DEBUG )
//    printUart( "Interrupt Serive Routine Called" );
  
  TIMSK1 = (1 << TOIE1); 
  //TIFR1 = 1;
  //TCCR1B |= (1 << WGM12);

  if ( g_iCurrentState == g_iNextState ) {
//    if ( DEBUG ) printUart( "ISR: State has not changed" );
    updateMenu();
    return;
  }
  
  if ( DEBUG ) {
    String sNext = String(g_iNextState);
    const char* cNext = sNext.c_str();
    String sCurrent = String(g_iCurrentState);
    const char* cCurrent = sCurrent.c_str();
    const char* sFrom = "ISR: State changed from: ";
    const char* sTo =   "ISR: State changed to:   ";
    printUart( strcat( sFrom, cCurrent ) );
    printUart( strcat( sTo, cNext ) );
  }

  g_iCurrentState = g_iNextState;
  switch ( g_iCurrentState ) {
    case STATE_MAIN:
      menuMain();
      break;
    case STATE_PRESETS:
      menuPreset();
      break;
    default:
      break;
  }
}

/*
* Key Listner
*/
void startKeyListner() {
  while(1) {
    const char key = inKeypad.getKey();
    if ( key != NO_KEY ) {
      if ( DEBUG )
        printUart( strcat("Key Pressed:",&key) );
      decodeState( key ); 
    }
  }
}

/*
* main
*/ 

void setup()
{
  pinMode( PIN_TO_LED, OUTPUT );
  pinMode( PIN_FROM_PHOTO, INPUT );
  lcdSerial.begin( LCD_BAUD_RATE );
  uartSerial.begin( UART_BAUD_RATE );
  g_iCurrentState = STATE_INITIAL;
  delay(500); // wait for display to boot up
//  setLcdBacklight( 100.0 ); //  need to debug
  splashScreen();
  startTimerInterrupts();
}

void loop()
{ 
  startKeyListner();
  while(1);
}

