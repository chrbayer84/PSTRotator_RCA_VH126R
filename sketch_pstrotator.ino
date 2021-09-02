#include <IRremote.h>

const int buttonPin = 12; // Button pin. Pulled up. Triggered when connected to ground.
const int ledPin = 3;
int azimuth=0;
int azimuthRCA=0;
int azimuthNew=0;
char ircode='A';


IRsend irsend;
// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(4800);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
}
  

// map azimuth to A-L for tuner IR code
char az2char(int azimuth)
{
  char letter;
  azimuth = (azimuth+15)/30;
  letter = 'A' + (azimuth%12);
  return letter;
}
// the loop function runs over and over again forever
void loop() {
  char buffer[32];
  char cmd[32];
  
  int bytes = Serial.readBytesUntil(0x3b, buffer, 32);
  if (azimuth < azimuthNew)
  {
    azimuth += 10;
    if (azimuth > azimuthNew) azimuth= azimuthNew;
    delay(930);  // timing approximates tuner rotation time
  }
  else if (azimuth > azimuthNew)
  {
    azimuth -= 10;
    if (azimuth < azimuthNew) azimuth= azimuthNew;
    delay(930);
  }
    
  if (bytes)
  {
    if (strncmp(buffer,"AP1",3)==0)
    {
      sscanf(buffer,"AP1%3d",&azimuthNew);
      ircode = az2char(azimuthNew);
      azimuthNew = (ircode-'A')*30;
      if (azimuthNew > azimuth) azimuth += 10;
      else azimuth -= 10;
      if (azimuth < 0) azimuth = 0;
      if (abs(azimuth - azimuthNew) < 15) azimuth = azimuthNew;
    }
    else if (strncmp(buffer,"AI1",3)==0)
    {
      sprintf(cmd,";%03d",azimuth);
      Serial.write(cmd);
    }
  }
  if (digitalRead(buttonPin) == LOW)
  {  
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(ledPin, HIGH);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(ledPin, LOW);
    //for (int i = 0; i < 3; i++) 
    //{
    //  irsend.sendNEC(0xa90, 12); // Sony TV power code
    //  delay(40);
    //}    
    //delay(2000);
  }
}
