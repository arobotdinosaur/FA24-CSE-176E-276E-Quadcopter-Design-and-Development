//this had no compilation issues
#include <radio.h>
#include <Adafruit_Simple_AHRS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <QuadClass_LSM6DSOX.h>

  uint8_t a[4] = {0};
  const int left_rear = 8;
  const int right_rear = 3;
  const int left_top = 5; 
  const int right_top = 4; 
  uint32_t start_time = 0; 
  const int magicNumber = 57;
  const int magicNumber2 = 53;

  int len = 0;

  //uint8_t a[4] = {0};


  float pitch; 
  float pitch_rate;
  float roll;
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
  double yaw_offset = -0.91+0.45;
  double pitch_corrected; 
  double yaw_corrected;

  //pid params tuning 
  float PIDp = 0.0;
  int PIDoutputp = 0;
  float setpointPitchp = 0.0;
  float Kpp = 0.25; //0.5  //battery on bottom: this works 0.25
  float Kip = 0.05; //0.04, 0.05
  float Kdp = 0.0;//0.4, 0.1
  float integralp = 0.0;
  float integral_errorp = 0.0;
  float previousErrorp = 0.0;

  
  float Kpy = 0.5; //0.5
  float Kiy = 0.04; //0.04
  float Kdy = 0.4;//0.4
  float yaw_setpoint = 0.0;  // The desired yaw rate (usually set to 0)
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
  rfBegin(25);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)
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

  

}

unsigned long  last = millis();
//full battery is at 885 max 
void loop() {
  Serial.print("-90:");
  Serial.println(-90);
    Serial.print("90:");
  Serial.println(90);
  int BAT_VALUE = analogRead(A7); 
  //Serial.print("Battery Voltage:"); 
  //Serial.println(BAT_VALUE);

  //checking data from imu 
  quad_data_t orientation;
  //static unsigned long  last_time = millis();
  unsigned long  now = millis();
  float dt = (now - last);
  if (ahrs->getQuadOrientation(&orientation))
  {
    /* 'orientation' should have valid .roll and .pitch fields */
    //Serial.print(now - last);
    //Serial.print(F(" "));
    pitch = orientation.pitch;
    Serial.print("rawpitch:");
    Serial.println(pitch);

    pitch_rate = orientation.pitch_rate;
    Serial.print(" raw_pitch_rate:");
    Serial.println(pitch_rate);
   // Serial.print(orientation.pitch);
   // Serial.print(F(" "));
   // Serial.print(orientation.pitch_rate);
    //Serial.print(F(" "));
    //implemeting for complemetary filtering (check if this is right)
    roll = orientation.roll;
    
    sensors_event_t gyro_event;
    _gyro->getEvent(&gyro_event);
    lsm.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);
    lsm.setGyroDataRate(LSM6DS_RATE_12_5_HZ);

    // don't know why this doesnt work- check 
   // _accel->getEvent(&accel_event);
    //lsm.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    //lsm.setAccelDataRate(LSM6DS_RATE_12_5_HZ);

    double gyro_raw_pitch = gyro_event.gyro.y; //.z was there initially, may need pid change
    double gyro_raw_roll = gyro_event.gyro.x;
    double gyro_raw_yaw = gyro_event.gyro.z;

    Serial.print("gyro_raw:");
    Serial.println(gyro_raw_pitch*RAD_TO_DEG);

    //Serial.print(gyro_raw_pitch);
    //Serial.print(F(" "));
    //Serial.print(gyro_raw_roll);
    //Serial.print(F(" "));
    //Serial.print(dt);
    //Serial.print(F(" "));
    double gyro_angle_pitch = cf_pitch + (gyro_raw_pitch * RAD_TO_DEG)*dt*0.001;
    double gyro_angle_roll = cf_roll + (gyro_raw_roll * RAD_TO_DEG)*dt*0.001;
    //Serial.print(gyro_angle_pitch);
    //Serial.print(F(" "));
    //Serial.print(gyro_angle_roll);
    //Serial.print(F(" "));
    cf_pitch = (gain * gyro_angle_pitch) + (1.0-gain)*pitch;
    cf_roll = (gain* gyro_angle_roll) + (1.0-gain)*roll;
    Serial.print(" cf_pitch:");
    Serial.println(cf_pitch);
    
    //Serial.print("cf roll:");
   // Serial.println(cf_roll);
    pitch_corrected = pitch_offset + cf_pitch; 
    yaw_corrected = yaw_offset + cf_roll; 
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
    integralp = errorPitch * dt*0.001+integralp*3/4;
   //}

  float derivativep = (errorPitch - previousErrorp) / (dt*0.001);
  PIDp = (Kpp * errorPitch) + Kip * integralp + Kdp * derivativep;
  previousErrorp = errorPitch;

  PIDoutputp = constrain(PIDp, -200, 200); //prev -50 to 50
  //Serial.print("PIDoutputp:");
 // Serial.println(PIDoutputp);

  
  //Serial.print(" gyro_yaw:");
 // Serial.println(gyro_raw_yaw);
    mixing();

    yaw_error = yaw_setpoint - gyro_raw_yaw;

    integralYaw += yaw_error * dt;
  
  float derivativeYaw = (yaw_error - previousYawError) / dt;
  float PIDyaw = Kpy * yaw_error + Kiy * integralYaw + Kdy * derivativeYaw;
  previousYawError = yaw_error;
  yaw_rate = PIDyaw;

  yaw_rate = constrain(yaw_rate, -255, 255);
  //New mixing algorithm concept - we constrain throttle to set range that is between 0 and like 200, and then use pid to deviate from that range by +-50 or so.
  // I think it sense to also have PID deviate below the throttle range for normal flight. The one challenge with this is that at near zero throttle the PID controller cannot deviate as far below, so it may perform differently/less stable if it
  //wants to push the throttle lower but cannot. Worth experimenting with. On the other hand we want the quadcopter to stay level so we do want to figure out to make the negative pid work somehow.
  //Maybe weirdness at the low throttle levels doesn't matter since the quadcopter doesn't fly anyway. So our PID probably doesn't work right at throttle <20% but it never matters.

  //Doing it all down below in a mixing() function eventually we want to print out the individual throttle values for each rotor. For now we just do pitch.

  //analogWrite(left_rear, throttle_left_rear);
 // analogWrite(right_rear, throttle_right_rear);




  }

  last = now;

  
  

  int len;
  
  uint8_t a[4] = {0};
  
 if (len = rfAvailable()){
  //Serial.print("Read packets:");
  //Serial.println(len);
  if(len!=4){
  rfFlush();
 }  
 else{
    rfRead(a, len);
    if (a[0]==magicNumber && a[1]==1 && a[0] + a[1] + a[2] == a[3]){//adding && len==4 check fails - len seems to be assigned to 130 instead... sometimes 126
      //start_time = 0; 
      analogWrite(LED1, 200);
      //analogWrite(LED2, 200);
      throttle=a[2];
      
      //analogWrite(left_rear, throttle);
      //analogWrite(right_rear, throttle);
      //analogWrite(left_top, throttle );
      //analogWrite(right_top, throttle);
    }
    else{
      rfFlush();
    }
   }
  }

 // read_radio();

 /* if (start_time<=4){
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
  }*/
  //Serial.println(start_time);

  uint8_t b[4] = {0};
  b[0] = magicNumber2;
  b[1]= BAT_VALUE;
  b[2]=1;
  b[3] = b[0]+b[1]+b[2];
  //disarm = 0;

  
  timer = timer+dt;
  if (timer>50){
  rfWrite(b,4);
  timer = 0;
  }
  //delay(50); The above implementation stops the rfwrite delays from slowing everything else down.

  //analogWrite(LED3, 200);
  //analogWrite(LED4, 200);

  //analogWrite(PRETTY_LEDS, 200);





}

void mixing(){
throttle=10;
throttle_left_rear = throttle+PIDoutputp;
throttle_left_top = throttle + PIDoutputp;
throttle_right_rear = throttle - PIDoutputp;
throttle_right_top = throttle - PIDoutputp;
Serial.print("PRE PID:");
Serial.println(PIDoutputp);
throttle_left_rear = constrain(throttle_left_rear, 0, 255);
throttle_right_rear = constrain(throttle_right_rear,0,255);
throttle_left_top = constrain(throttle_left_top, 0,255);
throttle_right_top = constrain(throttle_right_top, 0,255);
analogWrite(left_rear, throttle_left_rear);
analogWrite(right_rear, throttle_right_rear);
analogWrite(left_top, throttle_left_top);
analogWrite(right_top, throttle_right_top);

Serial.print("right_rear:");
Serial.println(throttle_right_rear);
Serial.print("left_rear:");
Serial.println(throttle_left_rear);
Serial.print("left_top:");
Serial.println(throttle_left_top);
Serial.print("right_top:");
Serial.println(throttle_right_top);

}
