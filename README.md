![Raw Quadcopters](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/IMG_1573.HEIC)
Quadcopters designed by Kaustubh Kanagalekar and Alexander Haken

## Introduction

In this project class, we were challenged to design a printed circuit board (PCB) quadcopter and program it using Arduino software in order to achieve flight. 

The class was broken into multiple smaller milestones- PCB design for electronic components, PCB design for quadcopter, sensor filtering and calibration, Proportional, Integral, Derivative (PID) tuning, PCB quadcopter assembly, and flight. As we were designing the PCB quadcopter using computer software, we were provided with a test flight control board (FCB) that we could work on concurrently. We could use either the FCB or the custom PCB quadcopter for demonstration purposes. 

The skills that we learned from the class were-
1. PCB design and fabrication
2. Embedded systems programming 
3. Control theory and sensor calibration 

## Electronic Design

Using Fusion 360, we were tasked with designing the following components from scratch- 

1. Resistor (0805 0Ohm) 
2. Red LED 
3. MOSFET (N-channel) 
4. Inertial measurement unit (IMU) (LSM6DSOXTR 6-Axis) 

We referenced the data sheets for each component online and extensively studied the footprints. 
More specifically, we utilised the footprints tab in Fusion 360 to add SMD pads, silkscreens, keepouts, etc. to accurately replicate the dimensions. 

We also took advantage of the automated building tool at times to accurately build the electronic components.  

Footprint of the resistor
![Footprint of the resistor](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-17%20at%2015.55.44.png) 

Footprint of the LED 
![Footprint of the LED](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-17%20at%2015.57.06.png)

Footprint of the MOSFET 
![MOSFET Footprint](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-17%20at%2015.58.18.png) 

Footprint of the IMU 
![IMU Footprint](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-17%20at%2015.57.47.png)


After creating the footprints, we built symbols for each electronic component using the standard IEEE styling conventions using the symbols tab. 

Finally, we connected the footprints and the symbols of each component in the components tab so that we could use them in our quadcopter schematic and design. 

We ensured that we met all the necessary guidelines and requirements set forth by the PCB manufacturer and instructor in our designs.  

## Quadcopter Design 

We were tasked with creating the schematic for the PCB quadcopter. In addition to our custom made components, we were also provided with additional parts from the instructor. 
The quadcopter had multiple smaller sketches- 
1. The microcontroller connections  
2. Bootloader 
3. IMU 
4. Debug/Rescue Header 
5. Motor Driver 
6. Netbridge and power supply 

### Microcontroller connections
We connected various pins and components to the microcontroller. The most significant connection we had was the balun (used for allowing balanced and unbalanced lines to be interfaced without major disturbances in impedances). This was significant as it allowed us to securely connect the antenna in our design. We were deliberate in our antenna placement as incorrect placements would yield spotty data transmission, and the quadcopter may not be able to fly.  

### Bootloader 
The primary objective of including the bootloader was  to load code to the microcontroller. Pins were connected to the microcontroller from the bootloader. 

### IMU 
We established a communication protocol with the microcontroller using I2C (Inter-Integrated Circuit). We incorporated two wires (Serial Data Line and Serial Clock Line) to transmit data between the microcontroller and IMU. 

### Debug/Rescue Header 
The rescue header was incorporated in order to help with debugging signals during testing. We also added a IMU reuse header in case our soldered IMU had issues. 

### Motor Driver 
We built four copies of the motor driver sketch as there were four motors driving the quadcopter. We included motor pads, mosfets, and a diode in the sketch. More importantly, we also included a decoupling capacitor to help absorb switching noise from the motors.

### Netbridge and Power Supply 
We designed a low-dropout voltage regulator in the schematic to provide a stable power output of 3V to the motors. This was because the batteries were providing power ranging from 4V to 5V. We had two power rails: An unregulated power rail (called VBAT) that provides power to the motors and input of the voltage regulator and is driven directly by the battery, and a regulated supply driven by the voltage regulator (called 3V) 

A netbridge was also created in order to electrically connect two nets in a schematic while keeping the nets separate in schematic . This was done to connect digital ground (BAT_GND) to regular ground. 

Image of the overall schematic 
![Schematic](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-17%20at%2022.47.36.png)

### PCB Design 
We created a custom shape for the quadcopter and placed all the necessary components from the sketch to the board. The placement of some components were extremely important- for example, the antenna had to be placed near the edge of the board for uninterrupted communication, and the IMU had to be placed at the centre for accurate measurements. The crystal was placed close to the microcontroller, and the motor controllers were placed at the edges of the board. 

We primarily used the autorouter to route all wirings and connections, except in the case of the antenna and parts of the microcontroller. The antenna had to be connected with a large wire (50 mm) manually to the balun and other components (such as the corresponding  microcontroller pins and capacitors). The crystal also had to manually routed with specific pins of the microcontroller.  

The board had to fit under a 13.9 cm square requirement  with motor holes of 12.2 mm for optimal placement. 
In Fusion 360, the board was aligned to a 1 mm grid for aesthetic reasons. 

We had four layers on the board in order to manage all the wirings and connections succinctly. We had a top layer, bottom layer, and two internal layers (one for 3V/VBAT and the other for GND/BAT_GND). The top and bottom layers involved signal routing for connecting various electronic components. The bottom plan was used whenever possible to reduce electrical noise. The 3V/VBAT and GND/BAT_GND layers were exclusively used for delivering power and ground signals, respectively. The traces that carried both power and ground signals were as short as possible to provide a low-resistance path for current to flow. 
One notable exception was that the IMU did not have access to any layer of the board; this was because of the recommendations made by the IMU manufacturer. 
 
Image of the connected boards in Fusion 360.
![Board 1](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-18%20at%2015.51.15.png)
![Board 2](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/Screenshot%202024-12-18%20at%2016.17.27.png)

## Sensor Filtering and Calibration 

Prior to reading data from the IMU, we had to establish a connection between a test remote and the FCB; we accomplished this using a check-sum that would transmit data sent from the remote (such as throttle value, pitch, roll, yaw) to the FCB. In addition, we also wrote code that read the existing gimbal value from the remote and transmitted it to the FCB to achieve flight. 
The check-sum was created in order to prevent bad data packets from getting sent over and to only accept packets that were sent from our remote (and not from other groups). 

On the FCB, we had to read data from the IMU to accurately measure the orientation. We used an existing library corresponding to the IMU called Adafruit_AHRS- from this, we used the `Adafruit_Simple_AHRS::getQuadOrientation()` function to gather all the necessary data.
We obtained the pitch rate, roll, and yaw from the quad orientation function.  
After gathering these data from the IMU, we created a complementary filter to filter out some of the noise created by the accelerometer and gyroscope. The accelerometer (Euler angles of the orientation) and gyroscope readings (the angular accelerations) were taken from the IMU. 
The reason we created a complementary filter was to gather the best possible data from both the gyroscope and the accelerometer as each inertial sensor has positives and drawbacks. The accelerometers do not drift over time, but they are quite noisy when the motors are running. Whereas, gyroscopes are less noisy than  the accelerometers, but the resulting measurement will drift over time due to accumulated error. We tried to set a high gain such that we could extract more data from the gyroscope in our code. 
We used a standard formula shown below with a gain of 0.98- `cf_angle = (gain) * (cf_angle + (gyro_raw * RAD_TO_DEG * dt)) + (1-gain) * (acc_angle)`

`RAD_TO_DEG = 57.295779513082320876798154814105`
`cf_angle` is the angle resulted from the complementary  filter, `gyro_raw` is the gyroscope measurement, `acc_angle` is the Euler angles, `dt` is the change in time from the measurements.  

We also accounted for any offsets caused by the IMU in the code to have it read zero when levelled. 
We only performed complementary filtering on the roll and pitch axes. 

## PID Control and Tuning 
PID is a type of controller used in feedback systems to regulate speed, position, and various other parameters to ensure stability. We used PID to keep the FCB stable and levelled when the motors were running. 
Proportional (P) responds to the current error and provides immediate action 
Integral (I) accumulates past errors 
Derivative (D) predicts future errors based on the rate of change. 
This was implemented for all three axes (roll, pitch, yaw rate) using the following equation- 
`PID = (Kp* error) + Ki* integral - Kd* derivative` 
Where `Kp, Ki, Kd` are constants that are determined by testing, `error` is the difference from the remote gimbal input and the offset of the axis (`error = setpoint - corrected`), `integral` is `integral = error* dt+integral` , and derivative is the raw gyro readings in degrees. 

PID for yaw rate was implemented a bit differently as ‘I’ wasn’t required to control it. Hence, only `Kp` and `Kd` were used in the PID code (with similar implementation as listed above). 

After writing the code for all three axes control, we combined them so that there would be an equal output to the motors. We implemented a mixing function which increased the power for the front motors and decreased the power for the rear motors based on the PID output.

Through trial and error, we were able to find the optimal `Kp, Ki, Kd` values for the various axes to achieve stability. For pitch and roll: a ‘P’ overshoot would have resulted in the FCB “sway” from one end to another, an ‘I’ overshoot would have resulted in levelling at an odd angle, and a ‘D’ overshoot would have resulted in the FCB oscillating rapidly. Undershoots of each would have resulted in the quad not being able to move at all. For yaw rate, overshoots of ‘P’ and ‘D’ resulted in the FCB spinning uncontrollably in space, whereas undershoots would have not caused any motion. 
For determining the yaw rate coefficients, our control was proportional to the yaw rate error (thus we did not have a `Kd` value after testing).  

This is a video of the PID demonstration for the pitch axis
![PID Demo video](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/IMG_5397.mov)


## PCB Assembly 
After our PCB quadcopter boards were manufactured, we started on the assembly portion of the board. We applied solder paste to our boards using a stencil (a thin aluminium sheet with precise holes corresponding to the board). Immediately after, we attached all the necessary electronic components by hand, ensuring accuracy and precision. We later used a reflow oven to connect the components and the PCB pads as a reflow oven guarantees a secure connections between the paste applied to the PCB pads and the attached components. 
 
Picture of assembly after reflow (note that some components were missing due to unavailability at the time of assembly) 
![Incomplete Assembly](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/IMG_5480.jpg)

## Flight 
We demonstrated flight using the provided FCB. 

Flight was broken into the following categories- 
1. "Roombaing" -- moving around the the floor with dowels installed with direction control and minimal yaw
2. Repeated, vertical hopping (>1 inches, > 0.25 second)
3. Repeated, vertical hopping (>6 inches, > 1 second)
4. Repeated, vertical hopping (>12 inches, > 3 seconds)
5. Short flight (>12 inches, > 6 seconds)
6. Sustained flight (> 12 inches, > 10 seconds, demonstrate yaw control)
7. 	Sustained flight (Fly long enough to consume an entire battery)

In this phase, we had also implemented a “trimming” mechanism where we would manually offset any discrepancies in pitch and roll to ensure a level flight takeoff. We would test which direction the FCB would travel towards when throttle was applied and would adjust any errors according. Trimming would have to be done every time we flew from a different location as ground would be levelled differently at each location. 

In our final demonstration, we were able to achieve FL 5. 

Please download the video after clicking the "?" icon to view the flight
![Flight video](https://github.com/KaustubhKanagalekar/FA24-CSE-176E-276E-Quadcopter-Design-and-Development/blob/main/Misc/IMG_5474.mov)

## Acknowledgements
We would like to thank our instructor, Dr. Steven Swanson, our TA, Zifeng Zhang, and our classmates for their assistance and guidance with the project.  
Here is the [link](https://github.com/NVSL/QuadClass-Resources) to the main quadcopter class repository.


