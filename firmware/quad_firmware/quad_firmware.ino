#include <radio.h>

  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 
  uint32_t start_time = 0; 
  const int magicNumber = 57;
  const int magicNumber2 = 53;

  int16_t len = 0;

  uint8_t a[4] = {0};


void setup() {
  //copied from rfecho
  rfBegin(12);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)
  int disarm = 1;
  disarm = 0;
  uint8_t b[4] = {0};
  b[0] = magicNumber2;
  b[1]= 57;
  b[2]=disarm;
  b[3] = b[0]+b[1]+b[2];
  rfWrite(b,4);

  //rear is where the 6 pins are sticking out
  const int SERIAL_BAUD = 9600 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);          // Start up serial

  const int BAT_SENSE_PIN = A7; 
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);

  // global declaration of LEDs 
  const int LED1 = 16;
  const int LED2 = 17; 
  const int LED3 = 18; 
  const int LED4 = 36; 
  const int PRETTY_LEDS = 34;

}

//full battery is at 885 max 
void loop() {
  int BAT_VALUE = analogRead(A7); 
 // Serial.print("Battery Voltage:"); 
 // Serial.println(BAT_VALUE);

  int throttle = 0; 
  int len;
  
uint8_t a[4] = {0};
 if (len = rfAvailable()) 
 Serial.println(len); 
 if (len != 4){
  rfFlush();
 }
  {
    rfRead(a, len);
    if (a[0]==magicNumber && a[1]==1 && a[0] + a[1] + a[2] == a[3]&& len==4){//adding && len==4 check fails - len seems to be assigned to 130 instead... sometimes 126
      start_time = 0; 
      Serial.println("armed, len=4");
      analogWrite(LED1, 200);
      //analogWrite(LED2, 200);
      throttle=a[2];
      analogWrite(left_rear, throttle);
      analogWrite(right_rear, throttle);
      analogWrite(left_top, throttle );
      analogWrite(right_top, throttle);
    }
    else{
      rfFlush();
         Serial.println("Flush");
    }

  }

 // read_radio();

  if (start_time<=254){
  start_time = start_time + 1;
  }
  if (start_time >= 20){
      a[4]={0};
      throttle=0;
      analogWrite(LED1, 0);
      analogWrite(left_rear, 0); 
      analogWrite(right_rear, 0);
      analogWrite(left_top, 0);
      analogWrite(right_top, 0);
  }
  //Serial.println(start_time);

  uint8_t b[4] = {0};
  b[0] = magicNumber2;
  b[1]= BAT_VALUE;
  b[2]=1;
  b[3] = b[0]+b[1]+b[2];
  //disarm = 0;

  rfWrite(b,4);
  //analogWrite(LED3, 200);
  //analogWrite(LED4, 200);

  //analogWrite(PRETTY_LEDS, 200);

delay(100);


}



void read_radio() {
int i = 0;
uint8_t a[4] = {0};
 rfRead(a, 4);
    if (a[0]==magicNumber && a[1]==1 && a[0] + a[1] + a[2] == a[3]){
      start_time = 0; 
      analogWrite(LED1, 200);
      //analogWrite(LED2, 200);
      analogWrite(left_rear, a[2]);
      analogWrite(right_rear, a[2]);
      analogWrite(left_top, a[2]);
      analogWrite(right_top, a[2]);
    }
    else{
      if(i<=254){
       i=i+1;
      rfFlush();
      read_radio();
      }
      else{
      rfFlush();
    }
    }
}

