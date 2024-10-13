#include <radio.h>
#include "quad_remote.h"      // Header file with pin definitions and setup
#include <serLCD.h>
#include <EEPROM.h>
//serLCD lcd; //idk why but this doesn't compile. Maybe changing it to SerLCD fixes it?
const int magicnumber = 5700;
struct flightControl{
  uint8_t magicnumber = 5700;
  
  int roll;
  int pitch;
  int yaw;
  int throttle;
  
  /*bool top;
  bool bottom;
  bool right;
  bool left;
  bool center;
  bool btn1;
  bool btn2;
  bool ist;
  bool knobpress;
  int knobturn;*/

  //const int magicnumber = 5700;
};

//data corresponding to max and min gimbal (top-bottom)
uint8_t gimbal_value_l = 0;
uint8_t gimbal_value_r = 0;
uint8_t max_gimbal = 1023; 
uint8_t min_gimbal = 0; 

//battery voltage seems to be capped at 57 using analogReference(INTERNAL);
void setup() {
  
	const int SERIAL_BAUD = 9600 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);          // Start up serial
	delay(1000);
  const int BAT_SENSE_PIN = A7;   //battery pin 
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);

	quad_remote_setup();

  rfBegin(12); //radio stuff
  rfPrint("ATmega128RFA1 Dev Board Online!\r\n");

  //testing for gimbal calibration 
 /* while (millis() < 1000) {
    gimbal_value_l = analogRead(A1);
    gimbal_value_r = analogRead(A3);

    if (gimbal_value_l > max_gimbal){
      int max_gimbal_l = gimbal_value_l; 
    }

    if (gimbal_value_r > max_gimbal){
      int max_gimbal_r = gimbal_value_r; 
    }
  }
*/
}

void loop() {
  // put your main code here, to run repeatedly:
  //This was very heavily inspired by the AnalogReadSerial Example
  //involves battery and gimbal control
  int leftsideways = analogRead(A0);
  float leftupdown = analogRead(A1);
  int rightsideways = analogRead(A2);
  int rightupdown = analogRead(A3);
  int BAT_SENSE_PIN = analogRead(A7); 
  //part of calibration 
 /* 
  leftsideways = constrain(leftsideways, min_gimbal_l,max_gimbal_l);//constraining as per instructions
  leftupdown = constrain(leftupdown, min_gimbal_l, max_gimbal_l);//constraining as per instructions
  rightsideways = constrain(rightsideways, min_gimbal_r,max_gimbal_r); //constraining as per instructions
  rightupdown = constrain(rightupdown, min_gimbal_r, max_gimbal_r);//constraining as per instructions
  leftsideways = map(leftsideways, min_gimbal_l, max_gimbal_l, 0, 255);
  rightsideways = map(rightsideways, min_gimbal_r, max_gimbal_r, 0, 255);
  rightupdown = map(rightupdown, min_gimbal_r, max_gimbal_r, 0, 255);
  leftupdown = map(leftupdown, min_gimbal_l, max_gimbal_l, 0, 255);*/
 //Serial.println(leftsideways);
  //Serial.println(leftupdown);
  //Serial.println(rightsideways);
  //Serial.println(rightupdown);
  Serial.print("Battery Voltage:"); 
  Serial.println(BAT_SENSE_PIN);


//// this is test code for radio communication 

  //uint8_t b[256];
  //int len;
  //if (len = rfAvailable())  // If serial comes in...
  //{
    //rfRead(b, len);
    //b[len] = 0;
    //Serial.write((char *)b);
  //}

  //rfWrite(flightControl);
  float max_gimbal_l=855;
  float min_gimbal_l=128;
int throttle=255*((leftupdown-min_gimbal_l)/(max_gimbal_l-min_gimbal_l));//
uint8_t a[4];
a[0]=57;//Magic number
a[1]=throttle;
Serial.println(throttle);
a[2]=2;
a[3]=3;
a[4]=4;
  rfWrite(a,5);
  //rfWrite('hi');
  delay(10);  // delay in between reads for stability


}
