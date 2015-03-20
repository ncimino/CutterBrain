
// ATMega328p
//16MHz clock
//
//3 Timers :: All use the AVR chip which have the control registers for the timers
//  Timer0 :: 8-bit (0-255) :: Used by delay() and millis()55ccccccccccccccccccccc2 (probabally shouldn't change)
//  Timer1 :: 16-bit (0-65535) :: Use by Arduino Server library, but okay to use
//  Timer2 :: 8-bit (0-255) :: Use by tone()
//
//TCCRxA and TCCRxB, where x == 0, 1, 2; TCCR :: Timer/Counter Control Register (0-2 correlate)
//  Each TCCR has 8 bits:
//    TCCRxA
//      Bit : Read/Write : Initial : Name
//      0 : RW : 0 : WGM10
//      1 : RW : 0 : WGM11
//      2 :  W : 0 : -
//      3 :  W : 0 : -
//      4 : RW : 0 : COM1B0
//      5 : RW : 0 : COM1B1
//      6 : RW : 0 : COM1A0
//      7 : RW : 0 : COM1A1
//    TCCRxB :: 3 most important bits are (CS10, CS11, and CS12)
//      0 : RW : 0 : CS10
//      1 : RW : 0 : CS11
//      2 : RW : 0 : CS12
//      3 : RW : 0 : WGM12
//      4 : RW : 0 : WGM13
//      5 :  W : 0 : -
//      6 : RW : 0 : ICES1
//      7 : RW : 0 : ICNC1
//    
//CSxx (Clock Settings) - In TCCRxB
//  Using the 3 CS control bits dictate the timer clock setting (all default to 0):
//    CS12 : CS11 : CS10 : Description
//       0 :    0 :    0 : No clock source (Timer / Counter stopped)
//       0 :    0 :    1 : clk_io / 1 (no prescaling)
//       0 :    1 :    0 : clk_io / 8 (from prescaler)
//       0 :    1 :    1 : clk_io / 64 (from prescaler)
//       1 :    0 :    0 : clk_io / 256 (from prescaler)
//       1 :    0 :    1 : clk_io / 1024 (from prescaler)
//       1 :    1 :    0 : External clock source on T1 pin - clock on falling edge
//       1 :    1 :    1 : External clock source on T1 pin - clock on falling edge
// 
//TIMSKx Timer/CounterX Interrupt Mask Register
//  To enable Timer1 overflow interrupt (<< left shifts the 1 TOIEx times and TIMSKx is set to that)
//    TIMSK1 = (1 << TOIE1); 
//
//To _disable_ timers set TCCRxB to 0 at anytime
//
//TOIE1 > Timer Overflow Interrupt Enable
//
//Clock Calculations (using Timer1 with default overflow @ 655355 [1111_1111_1111_1111]):
//  Without Prescaling:
//    TCCR1B |= (1 << CS10);
//    Timer1 > 16-bits (0-65535) @ 16MHz 
//    Timer1 clock cycle (period) > TIMER1_P = 1/16e6 seconds = 6.25e-8s
//    Run-time > TIMER1_P * Overflow > 6.25e-8s * 65535 = 0.0041s
//  
//  With Prescaling:
//    TCCR1B |= (1 << CS10);
//    TCCR1B |= (1 << CS12);
//    Timer1 frequency 16MHz / 1024 = 15625Hz
//    Timer1 clock cycle (period) > TIMER1_P = 1/15625 seconds = 0.000064s
//    Run-time > TIMER1_P * Overflow > 0.000064s * 65535 = 4.19424s
//    
//    

// http://maxembedded.com/2011/07/14/avr-timers-ctc-mode/

// http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/

// Arduino timer CTC interrupt example
// www.engblaze.com

// uart
#include <SoftwareSerial.h>
SoftwareSerial uartSerial(0, 1); // RX, TX
 
// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

#define LEDPIN 13

ISR( TIMER1_OVF_vect )
{
  digitalWrite( LEDPIN, !digitalRead( LEDPIN ));
  TIMSK1 = (1 << TOIE1); 
  uartSerial.print("Timed!!");
  uartSerial.println();
  //TIFR1 = 1;
  //TCCR1B |= (1 << WGM12);
}

void setup() {
  // uart
  uartSerial.begin(9600); // set up serial port for 9600 baud
  // put your setup code here, to run once:
  pinMode(LEDPIN, OUTPUT);
 
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
  
  uartSerial.print("TCCR1A:");
  uartSerial.print(TCCR1A);
  uartSerial.println();
    
  uartSerial.print("TCCR1B:");
  uartSerial.print(TCCR1B);
  uartSerial.println();
  
  uartSerial.print("== Started:");
  uartSerial.println();
}

void loop() {
  // put your main code here, to run repeatedly:

  // check whether the flag bit is set
  // if set, it means that there has been a compare match
  // and the timer has been cleared
  // use this opportunity to toggle the led
  /*if (TIFR & (1 << OCF1A)) // NOTE: '>=' used instead of '=='
  {
      PORTC ^= (1 << 0); // toggles the led
  }
//*/
  // wait! we are not done yet!
  // clear the flag bit manually since there is no ISR to execute
  // clear it by writing '1' to it (as per the datasheet)
//  TIFR |= (1 << OCF1A);

  // yeah, now we are done!
  //while(1);
  
  uartSerial.print("TIMSK1:");
  uartSerial.print(TIMSK1);
  uartSerial.println();
  
  uartSerial.print("TIFR1:");
  uartSerial.print(TIFR1);
  uartSerial.println();
  
  delay(200);
}
