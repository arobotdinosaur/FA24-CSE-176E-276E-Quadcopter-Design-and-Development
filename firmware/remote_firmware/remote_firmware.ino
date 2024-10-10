#include <radio.h>
#include "quad_remote.h"      // Header file with pin definitions and setup
#include <serLCD.h>
//serLCD lcd; //idk why but this doesn't compile. Maybe changing it to SerLCD fixes it?

void setup() {
  // put your setup code here, to run once:
	const int SERIAL_BAUD = 9600 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);          // Start up serial
	delay(1000);
  const int BAT_SENSE_PIN = A7; 
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);
	quad_remote_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  //This was very heavily inspired by the AnalogReadSerial Example
  int leftsideways = analogRead(A0);
  int leftupdown = analogRead(A1);
  int rightsideways = analogRead(A2);
  int rightupdown = analogRead(A3);
  int BAT_SENSE_PIN = analogRead(A7); 
  Serial.println(leftsideways);
  Serial.println(leftupdown);
  Serial.println(rightsideways);
  Serial.println(rightupdown);
  Serial.print("Battery Voltage:"); 
  Serial.println(BAT_SENSE_PIN);
  delay(1000);  // delay in between reads for stability
}
