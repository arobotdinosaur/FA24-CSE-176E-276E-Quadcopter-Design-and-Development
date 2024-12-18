Quadcopters designed by Kaustubh Kanagalekar and Alexander Haken

## Introduction

In this project class, we were challenged to design a printed circuit board (PCB) quadcopter and program it using Arduino software in order to achieve flight. 

The class was broken into multiple smaller milestones- PCB design for electronic components, PCB design for quadcopter, sensor filtering and calibration, Proportional, Integral, Derivative (PID) tuning, PCB quadcopter assembly, and flight. As we were designing the PCB quadcopter using computer software, we were provided with a test flight control board (FCB) that we could work on concurrently. In case our PCB quadcopter wouldn’t work as intended, we could use the FCB for demonstration purposes. 

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

{insert pictures of footprints} 

After creating the footprints, we built symbols for each electronic component using the standard IEEE styling conventions using the symbols tab. 

Finally, we connected the footprints and the symbols of each component in the components tab so that we could use them in our quadcopter schematic and design. 

We ensured that we met all the necessary guidelines and requirements set forth by the PCB manufacturer and instructor in our designs.  

