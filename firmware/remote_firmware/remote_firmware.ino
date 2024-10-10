#include <radio.h>
#include "quad_remote.h"      // Header file with pin definitions and setup
#include <serLCD.h>
//serLCD lcd; //idk why but this doesn't compile. Maybe changing it to SerLCD fixes it?

void setup() {
  // put your setup code here, to run once:
	const int SERIAL_BAUD = 9600 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);           // Start up serial
	delay(100);
	quad_remote_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  //This was very heavily inspired by the AnalogReadSerial Example
  int leftsideways = analogRead(A0);
  int leftupdown = analogRead(A1);
  int rightsideways = analogRead(A2);
  int rightupdown = analogRead(A3);
  Serial.println(leftsidways);
  Serial.println(leftupdown);
  Serial.println(rightsidways);
  Serial.println(rightupdown);
  delay(1);  // delay in between reads for stability
}
