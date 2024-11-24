#include <radio.h>
#include <Adafruit_Simple_AHRS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <QuadClass_LSM6DSOX.h>

  uint8_t a[15] = {0};
  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 
  uint32_t start_time = 0; 
  const int magicNumber = 57;
  const int magicNumber2 = 53;

  int len = 0;

  float pitch=0.0; 
  float pitch_rate = 0.0;
  float roll=0.0;
  float yaw = 0.0;
  float target_yaw = 0.0;
  QuadClass_LSM6DSOX lsm = QuadClass_LSM6DSOX();
  Adafruit_Simple_AHRS *ahrs = NULL;
  Adafruit_Sensor *_accel = NULL;
  Adafruit_Sensor *_gyro = NULL;
  Adafruit_Sensor *_mag = NULL; 

  //complementary filter param 
  #define RAD_TO_DEG 57.295779513082320876798154814105
  float gain = 0.98; 

  double cf_pitch = 0.0;
  double cf_roll = 0.0;
  double pitch_offset = 1.88+0.51;
  double roll_offset = 0.0; 
  double pitch_corrected = 0.0; 
  double roll_corrected = 0.0;
  double gyro_angle_yaw = 0.0;

  //pid params tuning 
  float PIDp = 0.0;
  float PIDr = 0.0;
  float PIDy = 0.0;
  float PIDy2 = 0.0;
  //int PIDoutputp = 0;
  float setpointPitchp = 0.0;
  float setpointRoll = 0.0;
  float Kpp = 00.0; //0.5  //battery on bottom: this works 0.25  0.20 0.23, 0.22
  float Kip = 0.0; //0.04, 0.05   0.01 0.002 , 0.0025
  float Kdp = 0;//0.4, 0.1  0.06 0.05, 0,032
  float Kpr = 0.0;
  float Kdr = 0.0;
  float Kir = 0.0;
  float previous_yaw_rate_error = 0.0;
  float derivative_yaw_rate = 0.0;
  double integralp = 0.0;
  double integralr = 0.0;
  float integral_errorp = 0.0;
  float previousErrorp = 0.0;
  float max_proportional = 30.0;
  float max_derivative = 40.0;
  float max_integral = 100.0;
  
  float Kpy = 0.4; //0.5
  float Kiy = 0.0; //0.04
  float Kdy = 1.0;//0.4
  double yaw_setpoint = 0.0;  // The desired yaw rate (usually set to 0)
  float yaw_rate = 0.0;      // The current yaw rate (from the gyroscope)
  float yaw_error = 0.0;     // The difference between desired and actual yaw rate
  float previousYawError = 0.0;  // Previous yaw error (for derivative term)
  float integralYaw = 0.0;

  int16_t throttle_left_rear = 0; 
  int16_t throttle_right_rear = 0;
  int16_t throttle_left_top = 0;
  int16_t throttle_right_top = 0;

  int16_t timer = 0;
  
  int16_t throttle = 0;

  double radiotimer = 0.0;

//sensors in imu
void setupSensor()
{


 if (!lsm.begin_I2C()) {
   // Serial.println("Failed to find LSM6DSOX chip");
    while (1) {
      delay(10);
    }
  }
  _accel = lsm.getAccelerometerSensor();
  _gyro = lsm.getGyroSensor();


  ahrs = new Adafruit_Simple_AHRS(_accel, _mag, _gyro);
#
}

void setup() {
  //copied from rfecho
  rfBegin(16);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)
  int disarm = 1;
  disarm = 0;
  uint8_t b[4] = {0};
  b[0] = magicNumber2;
  b[1]= 57;
  b[2]=disarm;
  b[3] = b[0]+b[1]+b[2];
  rfWrite(b,4);

  //rear is where the 6 pins are sticking out
  const int SERIAL_BAUD = 19200 ;        // Baud rate for serial port 
	Serial.begin(SERIAL_BAUD);          // Start up serial

  //calling sensor meansurement func
  setupSensor();
  const int BAT_SENSE_PIN = A7; 
  pinMode(BAT_SENSE_PIN, INPUT);
  analogReference(INTERNAL);

  // global declaration of LEDs 
  const int LED1 = 16;
  const int LED2 = 17; 
  const int LED3 = 18; 
  const int LED4 = 36; 
  const int PRETTY_LEDS = 34;

  //startupRamp(); 
}

unsigned long  last = millis();
//full battery is at 885 max 
void loop() {

  int BAT_VALUE = analogRead(A7); 
  //Serial.print("Battery Voltage:"); 
  //Serial.println(BAT_VALUE);

  //checking data from imu 
  quad_data_t orientation;
  //static unsigned long  last_time = millis();
  unsigned long  now = millis();
  float dt = (now - last);
  //Serial.println(dt);
  if (ahrs->getQuadOrientation(&orientation))
  {
    /* 'orientation' should have valid .roll and .pitch fields */
    //Serial.print(now - last);
    pitch = orientation.pitch;
    //Serial.print("rawpitch:");
    //Serial.println(pitch);

    pitch_rate = orientation.pitch_rate;
    //Serial.print(" raw_pitch_rate:");
    //Serial.println(pitch_rate);
    //yaw=orientation.yaw;

    roll = orientation.roll;
    
    sensors_event_t gyro_event;
    _gyro->getEvent(&gyro_event);
    lsm.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);
    lsm.setGyroDataRate(LSM6DS_RATE_208_HZ);

    sensors_event_t accel;
    _accel->getEvent(&accel);
    lsm.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    lsm.setAccelDataRate(LSM6DS_RATE_208_HZ);
    lsm.setAccelCompositeFilter(LSM6DS_CompositeFilter_LPF2, LSM6DS_CompositeFilter_ODR_DIV_800);
    double gyro_raw_pitch = gyro_event.gyro.y; //.z was there initially, may need pid change
    double gyro_raw_roll = gyro_event.gyro.x;
    double gyro_raw_yaw = gyro_event.gyro.z;

    //Serial.print("gyro_raw_pitch:");
    //Serial.println(gyro_raw_pitch*RAD_TO_DEG);

    
    double gyro_angle_pitch = cf_pitch + (gyro_raw_pitch * RAD_TO_DEG)*dt*0.001;
    double gyro_angle_roll = cf_roll + (gyro_raw_roll * RAD_TO_DEG)*dt*0.001;
    gyro_angle_yaw = gyro_angle_yaw + (gyro_raw_yaw*RAD_TO_DEG*dt*0.001);

    cf_pitch = (gain * gyro_angle_pitch) + (1.0-gain)*pitch;
    cf_roll = (gain * gyro_angle_roll) + (1.0-gain)*roll;
    //Serial.print(" cf_pitch:");
    //Serial.println(cf_pitch);
    
    //Serial.print("cf roll:");
    //Serial.println(cf_roll);
    //Serial.print("gyro_angle_yaw:");
    //Serial.println(gyro_angle_yaw);
    if (a[13]==1){
      pitch_offset=-cf_pitch;
      roll_offset=-cf_roll;
    }

    pitch_corrected = pitch_offset + cf_pitch; 
    roll_corrected = roll_offset+cf_roll;
    //Serial.print("pitch_corrected:");
    //Serial.println(pitch_corrected);
    //Serial.print("roll_corrected:");
    //Serial.println(roll_corrected);

   //pid code here 
   setpointPitchp = ((a[4] - 127) * (0.0787401));
   setpointRoll = ((a[5]-127)*0.0787401);
   //Serial.println(setpointPitchp);
   float errorPitch = setpointRoll - pitch_corrected; 
   float errorRoll = setpointRoll - roll_corrected;
   if(Kip == 0 ||a[2]<10){ //either Ki = 0 or Speed =0, unsure how to determine speed
      integralp = 0; 
   }
   else{
    integralp = errorPitch * dt*0.001+integralp;

   }
      if(Kir == 0 ||a[2]<10){ //either Ki = 0 or Speed =0, unsure how to determine speed
      integralr = 0; 
   }
   else{
    integralr = errorRoll * dt*0.001+integralr;

   }
  //float derivativep = (errorPitch - previousErrorp) / (dt*0.001);
  float derivativep = gyro_raw_pitch*RAD_TO_DEG;
  float derivativer = gyro_raw_roll*RAD_TO_DEG;
  //derivativep = constrain(derivativep, -max_derivative, max_derivative); //constrain d
  //static float last_derivative = 0.0;
  //float filtered_derivative = 0.8 * last_derivative + 0.2 * gyro_raw_pitch * RAD_TO_DEG;
  //last_derivative = filtered_derivative; //smoothening derivative term
  //float max_integral = 10;
  integralp = constrain(integralp, -max_integral, max_integral); //constrain i
  PIDp = (Kpp * errorPitch) + Kip * integralp - Kdp * derivativep; //changed from derivativep to filtered_derivative
  previousErrorp = errorPitch;
  PIDr= (Kpp * errorRoll) + Kip * integralr - Kdp * derivativer;
  //Serial.print("d:");
  //Serial.println(derivativep*Kdp);
  //Serial.print("p:");
 //Serial.println(errorPitch*Kpp);
  //Serial.print(" gyro_raw_yaw:");
  //Serial.println(gyro_raw_yaw);
    
    yaw_error = yaw_setpoint - gyro_angle_yaw*RAD_TO_DEG;
    //Serial.print(yaw_error);
  //Serial.println(yaw_setpoint);
    integralYaw += yaw_error * dt;
  
  float derivativeYaw = (yaw_error - previousYawError) / dt;

  float yaw_rate_error = yaw_setpoint-gyro_raw_yaw;
  previous_yaw_rate_error = yaw_rate_error;
  
  derivative_yaw_rate = (yaw_rate_error-previous_yaw_rate_error)/dt;


  PIDy = Kpy * yaw_error + Kiy * integralYaw + Kdy * derivativeYaw;
  PIDy2 = Kpy*yaw_rate_error+Kdy*derivative_yaw_rate;

  float yaw_control_derivative = gyro_raw_yaw/dt;
  previousYawError = yaw_error;
  //Serial.println(a[1]);
  if (a[1]==1){
  flying();
  //yawcontrol2();
  }
  }

  last = now;

  
  

  int len = 0;
  
  //uint8_t a[10] = {0};
  
 if (len = rfAvailable()){
  //Serial.print("Read packets:");
  //Serial.println(len);
  if(len!=15){
  rfFlush();
 }  
 else{
    rfRead(a, len);
    //Serial.println("a9");
    //Serial.println(a[9]);
    //Serial.println(len);
    if ((a[0]==magicNumber) && (a[1]==1) && (a[14]==(a[0]^a[1]^a[2]^a[3]^a[4]^a[5]^a[6]^a[7]^a[8]^a[9]^a[10]^a[11]^a[12]^a[13]))){//adding && len==4 check fails - len seems to be assigned to 130 instead... sometimes 126
      //start_time = 0; 
      analogWrite(LED1, 200);
      //analogWrite(LED2, 200);
      throttle=a[2];
      //Serial.println(a[2]);
      //int16_t a3 = a[3]; //converting to larger data type to avoid loop-around in conversion
      yaw_setpoint = (a[3]-123)*0.03 ;//conversion to deg
      //Serial.println(a[3]);
      float a6=a[6];//Stop the values from getting rounded away
      float a7=a[7];
      float a8=a[8];
      Kpp=a6/100;
      Kdp=a7/100;
      Kip=a8/100;
      //Kpy=a6/100;
      //Kdy=a7/100;
      //Kiy=a8/1000;
      Kpy = a[9];
      //Kdy = a[10]; 
      //Serial.print("yaw_setpoint");
      //Serial.println(yaw_setpoint);
      radiotimer=0.0;
    }
    else{
      rfFlush();
    }
   }
  //Serial.print("-90:");
  //Serial.println(-90);
  //Serial.print("90:");
  //Serial.println(90);
  }

  uint8_t b[4] = {0};
  b[0] = magicNumber2;
  b[1]= BAT_VALUE;
  b[2]=1;
  b[3] = b[0]+b[1]+b[2];
  
  timer = timer+dt;
  radiotimer = radiotimer+dt;
  if (timer>50){
  rfWrite(b,4);
  timer = 0;
  }
  if (radiotimer > 1000){
  a[1]=0;
throttle=0;
PIDy=0;
PIDp=0;
analogWrite(left_rear, 0);
analogWrite(right_rear, 0);
analogWrite(left_top, 0);
analogWrite(right_top, 0);
  }
  
}

void mixing(){
//Serial.println("Throttle:");
//Serial.print(throttle);
//PIDp=constrain(PIDp,-5,5);
throttle_left_rear = throttle+PIDp;
throttle_left_top = throttle + PIDp;
throttle_right_rear = throttle - PIDp;
throttle_right_top = throttle - PIDp;
//Serial.print("PRE_PID:");
//Serial.println(PIDp);
throttle_left_rear = constrain(throttle_left_rear, 0, 255);
throttle_right_rear = constrain(throttle_right_rear, 0 ,255);
throttle_left_top = constrain(throttle_left_top, 0,255);
throttle_right_top = constrain(throttle_right_top, 0,255);
analogWrite(left_rear, throttle_left_rear);
analogWrite(right_rear, throttle_right_rear);
analogWrite(left_top, throttle_left_top);
analogWrite(right_top, throttle_right_top);

}

void startupRamp(){
    int i=0;
  for (i=0; i<50; i+=5) {
    analogWrite(left_rear, i);
    analogWrite(right_rear, i);
    analogWrite(left_top, i);
    analogWrite(right_top, i);
    delay(500);
  }
}


void yawcontrol2(){
//PIDy=constrain(PIDy2,-20,20);
throttle_left_rear = throttle+PIDy2;
throttle_left_top = throttle - PIDy2;
throttle_right_rear = throttle - PIDy2;
throttle_right_top = throttle + PIDy2;
//Serial.print("PIDyaw:");
//Serial.println(PIDy);
throttle_left_rear = constrain(throttle_left_rear, 0, 255);
throttle_right_rear = constrain(throttle_right_rear, 0 ,255);
throttle_left_top = constrain(throttle_left_top, 0,255);
throttle_right_top = constrain(throttle_right_top, 0,255);
analogWrite(left_rear, throttle_left_rear);
analogWrite(right_rear, throttle_right_rear);
analogWrite(left_top, throttle_left_top);
analogWrite(right_top, throttle_right_top);
}

void flying(){
//Serial.println("Throttle:");
//Serial.print(throttle);
PIDp=constrain(PIDp,-40,40);
PIDr=constrain(PIDr,-40,40);
//PIDy2=constrain(PIDy2,-10,10);
throttle=constrain(throttle,0,255);
throttle_left_rear = throttle+PIDp-PIDr+PIDy2;
throttle_left_top = throttle + PIDp+PIDr-PIDy2;
throttle_right_rear = throttle - PIDp-PIDr-PIDy2;
throttle_right_top = throttle - PIDp+PIDr+PIDy2;
//Serial.print("PRE_PID:");
//Serial.println(PIDp);
throttle_left_rear = constrain(throttle_left_rear, 0, 255);
throttle_right_rear = constrain(throttle_right_rear, 0 ,255);
throttle_left_top = constrain(throttle_left_top, 0,255);
throttle_right_top = constrain(throttle_right_top, 0,255);
analogWrite(left_rear, throttle_left_rear);
analogWrite(right_rear, throttle_right_rear);
analogWrite(left_top, throttle_left_top);
analogWrite(right_top, throttle_right_top);

}