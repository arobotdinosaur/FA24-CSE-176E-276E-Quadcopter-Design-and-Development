// global declaration of LEDs 
const int LED1 = 16;
const int LED2 = 17; 
const int LED3 = 18; 
const int LED4 = 36; 
const int PRETTY_LEDS = 34;

void setup() {
  //rear is where the 6 pins are sticking out
  const int SERIAL_BAUD = 9600 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);          // Start up serial
	delay(1000);
  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 
  const int BAT_SENSE_PIN = A7; 
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);

}

//full battery is at 885 max 
void loop() {
   
  analogWrite(8, 10);
  analogWrite(3, 10);
  analogWrite(5, 10);
  analogWrite(4, 10);

  int BAT_SENSE_PIN = analogRead(A7); 
  Serial.print("Battery Voltage:"); 
  Serial.println(BAT_SENSE_PIN);

  analogWrite(LED1, 200);
  analogWrite(LED2, 200);
  analogWrite(LED3, 200);
  analogWrite(LED4, 200);

  analogWrite(PRETTY_LEDS, 200);

  delay(400);
}

