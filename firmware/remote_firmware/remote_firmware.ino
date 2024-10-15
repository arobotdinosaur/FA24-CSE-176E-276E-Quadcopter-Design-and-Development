#include <radio.h>
#include "quad_remote.h"  // Header file with pin definitions and setup
#include <serLCD.h>
#include <EEPROM.h>
//serLCD lcd; //idk why but this doesn't compile. Maybe changing it to SerLCD fixes it? Removing it doesn't seem to cause any issue.

//copied from the knob example
void knobs_update();
void knob_pressed(bool);
void btn1_pressed(bool);
void btn2_pressed(bool);

void btn_up_pressed(bool down);
void btn_down_pressed(bool down);
void btn_left_pressed(bool down);
void btn_right_pressed(bool down);
void btn_center_pressed(bool down);

int max_gimbal_l_vert_ad = 0;
int min_gimbal_l_vert_ad = 2;

int max_gimbal_r_vert_ad = 4;
int min_gimbal_r_vert_ad = 6;

int max_gimbal_l_hor_ad = 8;
int min_gimbal_l_hor_ad = 10;

int max_gimbal_r_hor_ad = 12;
int min_gimbal_r_hor_ad = 14;

bool knob_down=0;

//data corresponding to max and min gimbal (top-bottom)
uint8_t gimbal_value_l = 0;
uint8_t gimbal_value_r = 0;
int16_t max_gimbal_l_vert = 1023;  //uint8_t only goes to 255 so I changed this data type. Also, we want a signed int - we get negative values sometimes
int16_t min_gimbal_l_vert = 0;

int16_t max_gimbal_l_hor = 0;
int16_t min_gimbal_l_hor = 0;

//added for right updown
int16_t max_gimbal_r_vert = 1023;  
int16_t min_gimbal_r_vert = 0;

int16_t max_gimbal_r_hor = 0;
int16_t min_gimbal_r_hor = 0;

const int magicNumber = 57;
const int SERIAL_BAUD = 9600;
const int channel = 12;

const int BAT_SENSE_PIN = A7;

int leftupdown = 0;
int rightupdown = 0;
int leftsideways = 0;
int rightsideways = 0;
int armed=0;

//battery voltage seems to be capped at 57 using analogReference(INTERNAL);
//7 is the lowest I've seen. But not sure if 7 means high or low voltage? seems like 7 should mean lower...
void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);

  quad_remote_setup();

  rfBegin(channel);  //radio stuff
  rfPrint("ATmega128RFA1 Dev Board Online!\r\n");

  max_gimbal_l_vert=EEPROM.get(max_gimbal_l_vert_ad,max_gimbal_l_vert);
  min_gimbal_l_vert=EEPROM.get(min_gimbal_l_vert_ad,min_gimbal_l_vert); //I think we have to increment the EEPROM addresses by two as the gimbal values take up two bytes
  //Serial.println(max_gimbal_l_vert);

  max_gimbal_l_hor=EEPROM.get(max_gimbal_l_hor_ad,max_gimbal_l_hor);
  min_gimbal_l_hor=EEPROM.get(min_gimbal_l_hor_ad,min_gimbal_l_hor);

  //added for right updown
  max_gimbal_r_vert=EEPROM.get(max_gimbal_r_vert_ad,max_gimbal_r_vert);
  min_gimbal_r_vert=EEPROM.get(min_gimbal_r_vert_ad,min_gimbal_r_vert);

  max_gimbal_r_hor=EEPROM.get(max_gimbal_r_hor_ad,max_gimbal_r_hor);
  min_gimbal_r_hor=EEPROM.get(min_gimbal_r_hor_ad,min_gimbal_r_hor);

  knob1_btn_cb = knob_pressed;
  btn1_cb = btn1_pressed;
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
 // int leftsideways = analogRead(A0);
  int leftupdown = analogRead(A1);
  Serial.print(leftupdown);
  //int rightsideways = analogRead(A2);
  //int rightupdown = analogRead(A3);
  int BAT_SENSE_PIN = analogRead(A7);

  //Serial.println(leftsideways);
  //Serial.println(rightsideways);
  //Serial.println(rightupdown);
  //Serial.print("Battery Voltage:");
 // Serial.println(BAT_SENSE_PIN);

  //float max_gimbal_l=855;
  //float min_gimbal_l=128;
  //int throttle=255*((leftupdown-min_gimbal_l)/(max_gimbal_l-min_gimbal_l));

  uint8_t a[4] = {0};
  a[0] = magicNumber;

  a[1]=armed;

  leftupdown = constrain(leftupdown, min_gimbal_l_vert, max_gimbal_l_vert);
  leftupdown = map(leftupdown, min_gimbal_l_vert, max_gimbal_l_vert, 0, 255);
  Serial.println(leftupdown);

  rightupdown = constrain(rightupdown, min_gimbal_r_vert, max_gimbal_r_vert);
  rightupdown = map(rightupdown, min_gimbal_r_vert, max_gimbal_r_vert, 0,255);
  Serial.println(rightupdown);

  if (leftupdown==0 && knob_down == 1){
  update_display();
  lcd.print("Quadcopter Armed");
  armed = 1;
  }

  a[2] = leftupdown;
  Serial.println(leftupdown);
  a[3] = a[0] + a[1] + a[2];


	if (is_pressed(BUTTON_UP_PIN)&&armed==0) {
		calibrate();
    armed = 0;
	}


  if (armed==1){
  update_display();
  lcd.print("Armed");
}
else{
  update_display();
  lcd.print("Press knob to arm");
}

  rfWrite(a, 4);
  delay(10);  // delay in between reads for stability

}

void update_display() {
  lcd.clear();
  lcd.setCursor(0, 0);
}


void calibrate() {
  armed = 0;
  update_display();
  lcd.print("Beginning Calibration");
  delay(1000);
  update_display();
  lcd.print("Move left joystick low");
  delay(5000);
  min_gimbal_l_vert = analogRead(A1);
  update_display();
  lcd.print("Move left joystick high");
  delay(5000);
  max_gimbal_l_vert = analogRead(A1);
  EEPROM.put(max_gimbal_l_vert_ad,max_gimbal_l_vert);
  EEPROM.put(min_gimbal_l_vert_ad,min_gimbal_l_vert);
  delay(1000);
  update_display();

  lcd.print("Move left joystick left");
  delay(5000);
  min_gimbal_l_hor = analogRead(A0);
  update_display();
  lcd.print("Move left joystick right");
  delay(5000);
  max_gimbal_l_hor = analogRead(A0);
  EEPROM.put(max_gimbal_l_hor_ad,max_gimbal_l_hor);
  EEPROM.put(min_gimbal_l_hor_ad,min_gimbal_l_hor);
  delay(1000);
  update_display();

  //right side updown
  lcd.print("Move right joystick low");
  delay(5000);
  min_gimbal_r_vert = analogRead(A3);
  update_display();
  lcd.print("Move right joystick high");
  delay(5000);
  max_gimbal_r_vert = analogRead(A3);
  EEPROM.put(max_gimbal_r_vert_ad,max_gimbal_r_vert);
  EEPROM.put(min_gimbal_r_vert_ad,min_gimbal_r_vert);
  delay(1000);
  update_display();

    lcd.print("Move left joystick left");
  delay(5000);
  min_gimbal_r_hor = analogRead(A4);
  update_display();
  lcd.print("Move left joystick right");
  delay(5000);
  max_gimbal_r_hor = analogRead(A4);
  EEPROM.put(max_gimbal_r_hor_ad,max_gimbal_r_hor);
  EEPROM.put(min_gimbal_r_hor_ad,min_gimbal_r_hor);
  delay(1000);
  update_display();

  lcd.print("Calibration done!");
  uint8_t a[4]={0};
  rfWrite(a,4);





  /*
  leftsideways = constrain(leftsideways, min_gimbal_l,max_gimbal_l);
  leftsideways = map(leftsideways, min_gimbal_l, max_gimbal_l, 0, 255);
  pause(5000)
  leftupdown = constrain(leftupdown, min_gimbal_l, max_gimbal_l);
  leftupdown = map(leftupdown, min_gimbal_l, max_gimbal_l, 0, 255);

  rightsideways = constrain(rightsideways, min_gimbal_r,max_gimbal_r);
  rightsideways = map(rightsideways, min_gimbal_r, max_gimbal_r, 0, 255);

  rightupdown = constrain(rightupdown, min_gimbal_r, max_gimbal_r);
  rightupdown = map(rightupdown, min_gimbal_r, max_gimbal_r, 0, 255);
*/

}

 void knob_pressed(bool down) {
	if(down) {
		//Serial.println("knob down");
    knob_down=1;
	}else {
		//Serial.println("knob up");  
    knob_down=0;  
	}
}



void quad_bat(){
  uint8_t b[256];
  int len2;
  if (len2= rfAvailable()){
    rfRead(b, len2);
    b[len2] =0;
    Serial.write((char *)b);
  }
}

void btn1_pressed(bool down) {
	if(down) {
		//Serial.println("btn1 down");
    armed=0;
	} //else {
	//	Serial.println("btn1 up");    
	//}
}