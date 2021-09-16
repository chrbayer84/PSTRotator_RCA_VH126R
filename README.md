# PSTRotator_RCA_VH126R
Arduino sketch and documentation for RCA VH126R infrared controller to work with PSTRotator as a Rotor-EZ
Assumes RCA unit A-L is programmed for 0-360 in 30 degree increments.
Jumper or switch can be added to support 15 degree increments for 180 degree loop.
Sparkfun.com parts list
DEV-13975 Arduino RedBoard or similar
COM-09349 LED - Infrared 950nm
COM-00097 Mini Pushbutton Switch (to Park the antenna)

Wiring
Arduino Pin 3 to LED Anode (long leg)
Arduino Power GND to 330Ohm to LED Cathode (short leg)
Arduino Pin 12 and GND to switch or jumper to support 180 degrees instead of 360.
