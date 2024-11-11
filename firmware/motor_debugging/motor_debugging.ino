  int16_t throttle_left_rear = 50; 
  int16_t throttle_right_rear =50;
  int16_t throttle_left_top = 50;
  int16_t throttle_right_top = 50;
  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 
void setup() {
  int i=0;
  for (i=0; i<50; i+=5) {
    analogWrite(left_rear, i);
    analogWrite(right_rear, i);
    analogWrite(left_top, i);
    analogWrite(right_top, i);
    delay(500);
  }
}

void loop() {
analogWrite(left_rear, throttle_left_rear);
analogWrite(right_rear, throttle_right_rear);
analogWrite(left_top, throttle_left_top);
analogWrite(right_top, throttle_right_top);
}
