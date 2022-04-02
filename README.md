# PSTRotator_RCA_VH126R
Arduino sketch and documentation for RCA VH126R infrared controller to work with PSTRotator as a Rotor-EZ
Assumes RCA unit A-L is programmed for 0-360 in 30 degree increments.   

Jumper or switch can be added to support 15 degree increments for 180 degree loop.  

Sparkfun.com parts list:  
1. DEV-13975 Arduino RedBoard or similar  
2. COM-09349 LED - Infrared 950nm  
3. Optionap: COM-00097 Mini Pushbutton Switch (to Park the antenna)  
  
Wiring: 
1. Arduino Pin 3 to LED Anode (long leg)  
2. Arduino Power GND to 330Ohm to LED Cathode (short leg)  
3. Optional: Arduino Pin 12 to a switch and then GND to support 180 degrees instead of 360 when shorted -- or just use a jumper

Some info on hooking things up
https://learn.sparkfun.com/tutorials/ir-control-kit-hookup-guide#example-3-transmitting
