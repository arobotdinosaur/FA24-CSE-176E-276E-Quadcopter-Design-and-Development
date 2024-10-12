#include <radio.h>
#include "quad_remote.h"      // Header file with pin definitions and setup
#include <serLCD.h>
#include <EEPROM.h>
//serLCD lcd; //idk why but this doesn't compile. Maybe changing it to SerLCD fixes it?
struct flightControl{
  
  int roll;
  int pitch;
  int yaw;
  int throttle;
  
  bool top;
  bool bottom;
  bool right;
  bool left;
  bool center;
  bool btn1;
  bool btn2;
  bool ist;
  bool knobpress;
  int knobturn;

  const int magicnumber = 5700;
};

void setup() {
  // put your setup code here, to run once:
	const int SERIAL_BAUD = 9600 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);          // Start up serial
	delay(1000);
  const int BAT_SENSE_PIN = A7; 
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);
	quad_remote_setup();
  rfBegin(12);
  rfPrint("ATmega128RFA1 Dev Board Online!\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  //This was very heavily inspired by the AnalogReadSerial Example
  int leftsideways = analogRead(A0);
  int leftupdown = analogRead(A1);
  int rightsideways = analogRead(A2);
  int rightupdown = analogRead(A3);
  int BAT_SENSE_PIN = analogRead(A7); 
 //Serial.println(leftsideways);
  //Serial.println(leftupdown);
  //Serial.println(rightsideways);
  //Serial.println(rightupdown);
  //Serial.print("Battery Voltage:"); 
  //Serial.println(BAT_SENSE_PIN);
  uint8_t b[256];
  int len;
  if (len = rfAvailable())  // If serial comes in...
  {
    rfRead(b, len);
    b[len] = 0;
    Serial.write((char *)b);
  }

  rfwrite(flightControl);
  delay(1000);  // delay in between reads for stability
}
