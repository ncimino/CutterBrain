// Use the softwareserial library to create a new "soft" serial port
// for the display. This prevents display corruption when uploading code.
#include <SoftwareSerial.h>

// Attach the serial display's RX line to digital pin 2
SoftwareSerial uartSerial(0, 1); // RX, TX

int ledPin = 13; // LED connected to digital pin 13
int photoPin = 12;   // pushbutton connected to digital pin 7
int val = 0;     // variable to store the read value

void setup()
{
  pinMode(ledPin, OUTPUT);      // sets the digital pin 13 as output
  pinMode(photoPin, INPUT);      // sets the digital pin 7 as input
  uartSerial.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up
}

void loop()
{
  val = digitalRead(photoPin);   // read the input pin
  digitalWrite(ledPin, val);    // sets the LED to the button's value
  uartSerial.print("Photo value: ");
  uartSerial.print(val);   
  uartSerial.println();
}
