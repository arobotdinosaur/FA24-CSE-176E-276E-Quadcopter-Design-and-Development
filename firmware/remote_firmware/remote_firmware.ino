#include <radio.h>
#include "quad_remote.h"  // Header file with pin definitions and setup
#include <serLCD.h>
#include <EEPROM.h>
//serLCD lcd; //idk why but this doesn't compile. Maybe changing it to SerLCD fixes it? Removing it doesn't seem to cause any issue.

uint8_t a[15] = {0};
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

int p_ad = 16;
int i_ad = 18;
int d_ad = 20;

float Kp = 0.7; //0.13 0.5  //battery on bottom: this works 0.25  0.20 0.23, 0.22
float Ki = 0.0; //0.04, 0.05   0.01 0.002 , 0.0025
float Kd = 0.1;//0.04 0.4, 0.1  0.06 0.05, 0,032

float Kpy=5.0;

bool knob_down=0;
uint32_t start_time = 0; 

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
const int magicNumber2 = 53;
const int SERIAL_BAUD = 19200;
const int channel = 18;

const int BAT_SENSE_PIN = A7;
const int max_bat_remote = 57;
const int min_bat_remote = 7;
const int max_bat_quad = 115; // 885; I was getting different values in testing than we recorded earlier
const int min_bat_quad = 25; // 750;
int remotebattery = 0;

int leftupdown = 0;
int rightupdown = 0;
int leftsideways = 0;
int rightsideways = 0;
int armed=0;
int len = 0;
int i = 0;

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

  knobs_update_cb = knobs_update; 
  knob1_btn_cb = knob_pressed;
  btn1_cb = btn1_pressed;
  btn_left_cb =  btn_left_pressed;
	btn_right_cb = btn_right_pressed;
	btn_center_cb =  btn_center_pressed;
  btn_down_cb = btn_down_pressed;
  //bool btn_left= btn_left_pressed;
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
  //Serial.println(leftsideways);
  int leftupdown = analogRead(A1);
  //Serial.print(leftupdown);
  int rightsideways = analogRead(A2);
  int rightupdown = analogRead(A3);
  
  int BAT_SENSE_PIN = analogRead(A7);

  //Serial.println(leftsideways);
  //Serial.println(rightsideways);
  //Serial.println(rightupdown);
  //Serial.print("Battery Voltage:");
 // Serial.println(BAT_SENSE_PIN);

  uint8_t a[15] = {0};
  a[0] = magicNumber;

  a[1]=armed;

  leftupdown = constrain(leftupdown, min_gimbal_l_vert, max_gimbal_l_vert);
  leftupdown = map(leftupdown, min_gimbal_l_vert, max_gimbal_l_vert, 0, 255);
 // Serial.println(leftupdown);

  rightupdown = constrain(rightupdown, max_gimbal_r_vert, min_gimbal_r_vert);
  rightupdown = map(rightupdown, min_gimbal_r_vert, max_gimbal_r_vert, 0,255);

  leftsideways = constrain(leftsideways, min_gimbal_l_hor, max_gimbal_l_vert);
  leftsideways = map(leftsideways, min_gimbal_l_hor, max_gimbal_l_vert, 0, 255);

  rightsideways = constrain(rightsideways, min_gimbal_r_hor, max_gimbal_r_hor);
  rightsideways = map(rightsideways, min_gimbal_r_hor, max_gimbal_r_hor, 0,255);
 // Serial.println(rightupdown);

  if (leftupdown==0 && knob_down == 1){
  update_display();
  lcd.print("Quadcopter Armed");
  armed = 1;
  }

  a[2] = leftupdown;
  a[3] = leftsideways;
  //Serial.println(a[3]);
  a[4] = rightupdown;
  //Serial.println(a[4]);
  a[5] = rightsideways;
  a[6]=Kp*100; //Convert to int
  a[7]=Kd*100;
  a[8]=Ki*100;

  a[9] = Kpy; 
  
  //Serial.println(leftupdown);

  a[14] = a[0]^a[1]^a[2]^a[3]^a[4]^a[5]^a[6]^a[7]^a[8]^a[9]^a[10]^a[11]^a[12]^a[13];




	if (is_pressed(BUTTON_UP_PIN)&&armed==0) {
		calibrate();
    armed = 0;
	}

remotebattery = map(BAT_SENSE_PIN, min_bat_remote, max_bat_remote, 0, 100);

  if (armed==1){
  update_display();
  lcd.print("Armed ");
  lcd.print("remote:");
  lcd.print(remotebattery);
  lcd.print("%");
}
else{
  update_display();
  lcd.print("Unarmed ");
  lcd.print("remote:");
  lcd.print(remotebattery);
  lcd.print("%");
}

  //rfWrite(a, 10);

uint8_t b[4] = {0};
if (len = rfAvailable())  
  {
  if(len!=4 && len!=8 && len!=12){
  rfFlush();
 }  
 else{
    rfRead(b, len);
    //Serial.println(len);
    if (b[0]==magicNumber2 && b[0] + b[1] +b[2] == b[3]){ 
      start_time = 0; 
      //Serial.println(b[1]);885
      int quadbattery=b[1];
      quadbattery = map(quadbattery, min_bat_quad, max_bat_quad, 0, 100);
      lcd.print(" quad:");
      lcd.print(quadbattery);
      lcd.print("%");
      if (b[2]=0){
        armed = 0;
      }
      rfWrite(a,15);
    }
    else{
      rfFlush();
    }
   }
  }
  
  //Makes sure to disarm the remote if connection with the quad is lost
  if (start_time<=254){
  start_time = start_time + 1;
  }
  if (start_time >= 10){ //short time - but seems to work okay
    armed=0;
  }
  
  //delay(100);  // delay in between reads for stability

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

  lcd.print("Move left joystick right");
  delay(5000);
  min_gimbal_l_hor = analogRead(A0);
  update_display();
  lcd.print("Move left joystick left");
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

    lcd.print("Move right joystick left");
  delay(5000);
  min_gimbal_r_hor = analogRead(A2);
  update_display();
  lcd.print("Move right joystick right");
  delay(5000);
  max_gimbal_r_hor = analogRead(A2);
  EEPROM.put(max_gimbal_r_hor_ad,max_gimbal_r_hor);
  EEPROM.put(min_gimbal_r_hor_ad,min_gimbal_r_hor);
  delay(1000);
  update_display();

  lcd.print("Calibration done!");
  uint8_t a[15]={0};
  rfWrite(a,15);


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


//unused as of right now
/*
void quad_bat(){
  uint8_t b[256];
  int len2;
  if (len2= rfAvailable()){
    rfRead(b, len2);
    b[len2] =0;
    Serial.write((char *)b);
  }
}*/

void btn1_pressed(bool down) {
	if(down) {
		Serial.println("btn1 down");
    armed=0;
    uint8_t a[10]={0};
	} //else {
	//	Serial.println("btn1 up");    
	//}
}





void btn_left_pressed(bool down) {
	if(down) {
    update_display();
    lcd.print(knob1.getCurrentPos());
    if(knob1.getCurrentPos()==1){
      Kp=Kp+0.01;
    lcd.print("Kp ");
    lcd.print(Kp);
    Serial.print("Kp");
    Serial.println(Kp);
    }
    if(knob1.getCurrentPos()==2){
      Kp=Kp-0.01;
    lcd.print("Kp ");
    lcd.print(Kp);
    Serial.print("Kp");
    Serial.println(Kp);
    }
    if(knob1.getCurrentPos()==3){
    Kd=Kd+0.01;
    lcd.print("Kd ");
    lcd.print(Kd);
    Serial.print("Kd");
    Serial.println(Kd);
    }
    if(knob1.getCurrentPos()==4){
    Kd=Kd-0.01;
    lcd.print("Kd ");
    lcd.print(Kd);
    Serial.print("Kd");
    Serial.println(Kd);
    }
    if(knob1.getCurrentPos()==5){
    Ki=Ki+0.01;
    lcd.print("Ki");
    lcd.print(Ki);
    Serial.print("Ki");
    Serial.println(Ki);
    }
    if(knob1.getCurrentPos()==6){
    Ki=Ki-0.01;
    lcd.print("Ki*10 ");
    lcd.print(Ki*10);
    Serial.print("Ki");
    Serial.println(Ki);
    }
    if(knob1.getCurrentPos()==7){
      Kpy = Kpy+1.0;
      lcd.print("Kpy ");
      lcd.print(Kpy);
      Serial.print("Kpy");
      Serial.println(Kpy);
    }
    if(knob1.getCurrentPos()==8){
      Kpy = Kpy-1.0;
      lcd.print("Kpy ");
      lcd.print(Kpy);
      Serial.print("Kpy");
      Serial.println(Kpy);
    }
		//Serial.println("left down");;
    //Kp=Kp+knob1.getCurrentPos()*0.01;
    //knob1.setCurrentPos(0);
		//column = (column - 1) %16;
		//update_display();
    //lcd.print(Kp);
	} else {
		//Serial.println("left up");
	}
}

void btn_right_pressed(bool down) {
	if(down) {
    update_display();
    lcd.print(knob1.getCurrentPos());
    if(knob1.getCurrentPos()==1){
      Kp=Kp-0.01;
    lcd.print("Kp ");
    lcd.print(Kp);
    Serial.print("Kp");
    Serial.println(Kp);
    }
    if(knob1.getCurrentPos()==2){
      Kp=Kp+0.01;
    lcd.print("Kp ");
    lcd.print(Kp);
    Serial.print("Kp");
    Serial.println(Kp);
    }
    if(knob1.getCurrentPos()==3){
    Kd=Kd-0.01;
    lcd.print("Kd ");
    lcd.print(Kd);
    Serial.print("Kd");
    Serial.println(Kd);
    }
    if(knob1.getCurrentPos()==4){
    Kd=Kd+0.01;
    lcd.print("Kd ");
    lcd.print(Kd);
    Serial.print("Kd");
    Serial.println(Kd);
    }
    if(knob1.getCurrentPos()==5){
    Ki=Ki-0.01;
    lcd.print("Ki");
    lcd.print(Ki);
    Serial.print("Ki");
    Serial.println(Ki);
    }
    if(knob1.getCurrentPos()==6){
    Ki=Ki+0.01;
    lcd.print("Ki");
    lcd.print(Ki);
    Serial.print("Ki");
    Serial.println(Ki);
    }
		//Serial.println("left down");;
    //Kp=Kp+knob1.getCurrentPos()*0.01;
    //knob1.setCurrentPos(0);
		//column = (column - 1) %16;
		//update_display();
    //lcd.print(Kp);
	} else {
		//Serial.println("left up");
	}
}

    
void btn_center_pressed(bool down) {
	if(down) {
		Serial.println("center down");
		update_display();
	} else {
		Serial.println("center up");    
	}
}

void btn_down_pressed(bool down) {
	if(down) {
		Serial.println("bottom down");
    a[13]=1;
    rfWrite(a,15);
	} else {
    a[13]=0;
    rfWrite(a,15);
		Serial.println("bottom up");    
	}
}

void knobs_update() {
	Serial.print("Knob: ");
	Serial.println(knob1.getCurrentPos());
}