#include <radio.h>

  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 
  uint32_t start_time = 0; 
void setup() {
  //copied from rfecho
  rfBegin(12);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)

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
  int BAT_SENSE_PIN = analogRead(A7); 
 // Serial.print("Battery Voltage:"); 
 // Serial.println(BAT_SENSE_PIN);
  //rfWrite(BAT_SENSE_PIN);


  int throttle = 0; 
  int len;
uint8_t a[4] = {0};
 if (len = rfAvailable())  // If serial comes in...
  {
    rfRead(a, len);
    if (a[0]==57 && a[1]==1 && a[0] + a[1] + a[2] == a[3]){
      start_time = 0; 
      Serial.println(a[0]);
      Serial.println(a[1]);
      throttle=a[2];
      analogWrite(left_rear, throttle);
      analogWrite(right_rear, throttle);
      analogWrite(left_top, throttle );
      analogWrite(right_top, throttle);
    
      

    }
    else{
      rfFlush();
    }

  }



  start_time = start_time + 1;
  if (start_time >= 20){
    analogWrite(left_rear, 0); 
      analogWrite(right_rear, 0);
      analogWrite(left_top, 0);
      analogWrite(right_top, 0);
  }
  delay(10);
  Serial.println(start_time);

  
   
 /* analogWrite(8, 10);
  analogWrite(3, 10);
  analogWrite(5, 10);
  analogWrite(4, 10);*/

  
  //analogWrite(LED1, 200);
  //analogWrite(LED2, 200);
  //analogWrite(LED3, 200);
  //analogWrite(LED4, 200);

  //analogWrite(PRETTY_LEDS, 200);




}


