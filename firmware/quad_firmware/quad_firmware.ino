#include <radio.h>
#include <Adafruit_Simple_AHRS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <QuadClass_LSM6DSOX.h>

  uint8_t a[10] = {0};
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
  QuadClass_LSM6DSOX lsm = QuadClass_LSM6DSOX();
  Adafruit_Simple_AHRS *ahrs = NULL;
  Adafruit_Sensor *_accel = NULL;
  Adafruit_Sensor *_gyro = NULL;
  Adafruit_Sensor *_mag = NULL; 

  //complementary filter param 
  #define RAD_TO_DEG 57.295779513082320876798154814105
  float gain = 0.90; 

  double cf_pitch = 0.0;
  double cf_roll = 0.0;
  double pitch_offset = 1.88+0.51; 
  double pitch_corrected = 0.0; 
  double gyro_angle_yaw = 0.0;

  //pid params tuning 
  float PIDp = 0.0;
  float PIDr = 0.0;
  float PIDy = 0.0;
  //int PIDoutputp = 0;
  float setpointPitchp = 0.0;
  float Kpp = 0.15; //0.5  //battery on bottom: this works 0.25
  float Kip = 0.04; //0.04, 0.05
  float Kdp = 0.05;//0.4, 0.1
  float integralp = 0.0;
  float integral_errorp = 0.0;
  float previousErrorp = 0.0;
  
  float Kpy = 0.4; //0.5
  float Kiy = 0.0; //0.04
  float Kdy = 0.6;//0.4
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
  // Set data rate for G and XL.  Set G low-pass cut off.  (Section 7.12)
 // lsm.write8(XGTYPE, Adafruit_LSM9DS1::LSM9DS1_REGISTER_CTRL_REG1_G,  ODR_952 | G_BW_G_10 );  //952hz ODR + 63Hz cuttof

  // Enable the XL (Section 7.23)
  //lsm.write8(XGTYPE, Adafruit_LSM9DS1::LSM9DS1_REGISTER_CTRL_REG5_XL, XL_ENABLE_X | XL_ENABLE_Y | XL_ENABLE_Z);

  // Set low-pass XL filter frequency divider (Section 7.25)
  //lsm.write8(XGTYPE, Adafruit_LSM9DS1::LSM9DS1_REGISTER_CTRL_REG7_XL, HR_MODE | XL_LP_ODR_RATIO_9);
  

  // This only sets range of measurable values for each sensor.  Setting these manually (I.e., without using these functions) will cause incorrect output from the library.
 // lsm.setupAccel(Adafruit_LSM9DS1::LSM9DS1_ACCELRANGE_2G);
  //lsm.setupMag(Adafruit_LSM9DS1::LSM9DS1_MAGGAIN_4GAUSS);
 // lsm.setupGyro(Adafruit_LSM9DS1::LSM9DS1_GYROSCALE_245DPS);

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
  rfBegin(11);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)
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
    lsm.setGyroDataRate(LSM6DS_RATE_12_5_HZ);

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
   // Serial.println(cf_roll);
    //Serial.print("gyro_angle_yaw:");
    //Serial.println(gyro_angle_yaw);
    pitch_corrected = pitch_offset + cf_pitch; 
    //error values when running lsm.setAccelCompositeFilter 
    //lsm.setAccelCompositeFilter(LSM6DS_CompositeFilter_HPF, LSM6DS_CompositeFilter_ODR_DIV_800);
    

    //Serial.println(pitch_corrected);
   // Serial.print(F(" "));
    //Serial.println(yaw_corrected);
   // Serial.print(F(" "));

   //pid code here 
   float errorPitch = setpointPitchp - pitch_corrected; 
   //if(Kip == 0 ||a[2]<5){ //either Ki = 0 or Speed =0, unsure how to determine speed
      //integralp = 0; 
   //}
   //else{
    integralp = errorPitch * dt*0.001+integralp;
   //}

  float derivativep = (errorPitch - previousErrorp) / (dt*0.001);
  PIDp = (Kpp * errorPitch) + Kip * integralp + Kdp * derivativep;
  previousErrorp = errorPitch;

  //Serial.print(" gyro_raw_yaw:");
  //Serial.println(gyro_raw_yaw);

    yaw_error = yaw_setpoint - gyro_angle_yaw;
    //Serial.print(yaw_error);

    integralYaw += yaw_error * dt;
  
  float derivativeYaw = (yaw_error - previousYawError) / dt;

  PIDy = Kpy * yaw_error + Kiy * integralYaw + Kdy * derivativeYaw;

  previousYawError = yaw_error;
  //Serial.println(a[1]);
  if (a[1]==1){
  mixing();
  //yawcontrol();
  }
  }

  last = now;

  
  

  int len = 0;
  
  //uint8_t a[10] = {0};
  
 if (len = rfAvailable()){
  //Serial.print("Read packets:");
  //Serial.println(len);
  if(len!=10){
  rfFlush();
 }  
 else{
    rfRead(a, len);
    //Serial.println("a9");
    //Serial.println(a[9]);
    if ((a[0]==magicNumber) && (a[1]==1) && (a[9]==(a[0]^a[1]^a[2]^a[3]^a[4]^a[5]^a[6]^a[7]^a[8]))){//adding && len==4 check fails - len seems to be assigned to 130 instead... sometimes 126
      //start_time = 0; 
      analogWrite(LED1, 200);
      //analogWrite(LED2, 200);
      throttle=a[2];
      int16_t a3 = a[3]; //converting to larger data type to avoid loop-around in conversion
      yaw_setpoint = (a[3]-122)*1.41176470588 ;//conversion to deg
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
  if (radiotimer > 2000){
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
throttle_left_rear = throttle+PIDp;
throttle_left_top = throttle + PIDp;
throttle_right_rear = throttle - PIDp;
throttle_right_top = throttle - PIDp;
//Serial.print("PRE PID:");
//Serial.println(PIDp);
throttle_left_rear = constrain(throttle_left_rear, 0, 255);
throttle_right_rear = constrain(throttle_right_rear, 0 ,255);
throttle_left_top = constrain(throttle_left_top, 0,255);
throttle_right_top = constrain(throttle_right_top, 0,255);
analogWrite(left_rear, throttle_left_rear);
analogWrite(right_rear, throttle_right_rear);
analogWrite(left_top, throttle_left_top);
analogWrite(right_top, throttle_right_top);

//Serial.print("right_rear:");
//Serial.println(throttle_right_rear);
//Serial.print("left_rear:");
//Serial.println(throttle_left_rear);
//Serial.print("left_top:");
//Serial.println(throttle_left_top);
//Serial.print("right_top:");
//Serial.println(throttle_right_top);
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

void yawcontrol(){
throttle_left_rear = throttle+PIDy;
throttle_left_top = throttle - PIDy;
throttle_right_rear = throttle - PIDy;
throttle_right_top = throttle + PIDy;
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

//Serial.print("right_rear:");
//Serial.println(throttle_right_rear);
//Serial.print("left_rear:");
//Serial.println(throttle_left_rear);
//Serial.print("left_top:");
//Serial.println(throttle_left_top);
//Serial.print("right_top:");
//Serial.println(throttle_right_top);
}
