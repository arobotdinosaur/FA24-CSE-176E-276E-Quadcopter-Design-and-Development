#include arduino.h
#ifndef radio_communications_h
#define radio_communications_h

struct flightControl{
  
  int roll;
  int pitch;
  int yaw;
  int throttle;
  
  bool top;
  bool bottom;
  bool right;
  bool left;
  bool center;
  bool btn1;
  bool btn2;
  bool ist;
  bool knobpress;
  int knobturn;

  const int magicnumber = 5700;
}


#endif
