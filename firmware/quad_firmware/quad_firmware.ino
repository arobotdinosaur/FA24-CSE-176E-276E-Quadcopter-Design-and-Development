void setup() {
  //rear is where the 6 pins are sticking out
  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 

}

void loop() {
  analogWrite(8, 10);
  analogWrite(3, 10);
  analogWrite(5, 10);
  analogWrite(4, 10);
}

