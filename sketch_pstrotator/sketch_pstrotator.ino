#include <Wire.h>
#include <IRremote.h>
#include <SerLCD.h>
#include <EEPROM.h>

// constants
// HEADER max 16 characters
#define HEADER "W9MDB Ham Shack"
#define HEADER2 "RCA Rotor V1.5"
#define HEADER3 "W9MDB 2022-04-01"
// If degreesPin is shorted we'll use 15 degree intervals
const int degreesPin = 12;
const int sendPin = 3;
IRsend irsend(sendPin);

#define PARKADDR 0xff

// variables
int degreesPer = 30;  // Use 30 for 360 degree and 15 for 180 degree rotation
int debug = 0;
int azimuth = 0;
int azimuthNew = 0;
int delayTime = 5;
int delayTimeStartup = 0;
int startup = 0; // startup time needs to be added for rotor speed sync
bool moving = 0; // we're moving the antenna
String inputString;
String tmpString;
bool stringComplete;
char ircode='A';
SerLCD lcd;

#define PARKED ((unsigned char)0x1f)
#define NOTPARKED ((unsigned char)0x00)

void setup() {
  pinMode(degreesPin, INPUT_PULLUP);
    if (digitalRead(degreesPin) == LOW)
  {
    degreesPer = 15;  
  }
  // LCD stuff
  Serial.begin(4800);
  inputString.reserve(8);
  Wire.begin();
  lcd.begin(Wire);
  delay(500);
  lcd.clear();
  lcd.print(HEADER2);
  lcd.setCursor(0,1);
  lcd.print(HEADER3);
  IrSender.begin(sendPin, ENABLE_LED_FEEDBACK); 
  delay(3000);

  parked();
  if (!parked())
  {
    park();
  }
  else
  {
    lcd.clear();
    lcd.write(HEADER);
    lcd.setCursor(0,1);
    lcd.write("Parked");
  }
  EEPROM_writePark(PARKED);
}

bool parked()
{
 unsigned char value;
 EEPROM.get(PARKADDR, value);
 lcd.print("\nParked="+(String)value);
 return value == PARKED;
}

void park()
{
    lcd.clear();
    lcd.write("Park");
    if (debug) Serial.println("INIT Antenna to azimuth=0\r\n");
    sendNEC(0x6, 0x10, 0x05);
    azimuth = 340;
    azimuthNew = 0;
}

void EEPROM_writePark(unsigned char value)
{
 EEPROM.put(PARKADDR, value);
}

void sendNEC(unsigned char c1, unsigned char c2, unsigned char c3)
{
  int i;
  noInterrupts();
  delay(100);
  for(i=0;i<5;++i)
  {
    IrSender.sendNEC(c1,c2,c3);
    delay(50);
  }
  interrupts();
}


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 0x0a || inChar == 0x0d) continue;
    tmpString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == ';') {
      if (inputString == "")
      {
        setString(tmpString);
        tmpString = "";
      }
      else
      {
        if (debug) Serial.println("Dropping %s" + tmpString);
        tmpString = ""; // drop it if we didn't process last one
      }
    }
  }
}

void setString(String s)
{
  inputString = s;
  stringComplete = true;
}

void ezrotor(String cmd)
{
  int icmd;
  if (debug) Serial.println("ezrotor " + cmd);
  if (sscanf(cmd.c_str(),"AP1%d",&azimuthNew) == 1)
  {
    // bidirectional loop only has to move 0-180
    if (degreesPer == 15) azimuthNew = azimuthNew % 180;
    move(azimuthNew);
    // simulate rotation speed
    ircode = az2char(azimuthNew);
    azimuthNew = (ircode-'A')*degreesPer;
    if (azimuthNew > azimuth) azimuth += 10;
    else azimuth -= 10;
    if (azimuth < 0) azimuth = 0;
    if (abs(azimuth - azimuthNew) < 15) azimuth = azimuthNew;
  }
  else if (sscanf(cmd.c_str(),"AI%d",&icmd)==1)
  {
    char cmd[8];
    switch(icmd)
    {
    case 1:
      sprintf(cmd,";%03d",azimuth);
      Serial.print(cmd);
      break;
    default:
      break;
      // print error to display
    }
  }
  else if (inputString.indexOf(';') > 0)
  {
    inputString = "";
  }
}

// map azimuth to A-L for tuner IR code
char az2char(int azimuth)
{
  char letter;
  if (debug) {Serial.print("azimuth=");Serial.println(azimuth);}
  azimuth = (azimuth+(degreesPer/2))/degreesPer;
  letter = 'A' + (azimuth%12);
  if (debug) {Serial.print(" letter=");Serial.println(letter);}
  return letter;
}

void move(int azimuth)
{
  startup = 1;
  char letter = az2char(azimuth);
  int cmd = 0x04 + (letter - 'A');
  if (debug) {Serial.print("Move to ");Serial.print(azimuth);Serial.print("=");Serial.println(letter);}
  sendNEC(0x6, cmd, 5);
  //lcd.setCursor(0,0);
  //lcd.write("                ");
  //lcd.setCursor(0,0);
  lcd.clear();
  lcd.write("Move to ");lcd.print(azimuth);lcd.print("  ");
  position();
  EEPROM_writePark(NOTPARKED);
}

void position()
{
  lcd.setCursor(0,1);
  lcd.write("Az ");
  lcd.write(String(azimuth).c_str());
  lcd.write("  ");
  lcd.setCursor(10,1);
  lcd.write("Code=");
  lcd.write(ircode);
}

void loop() {
  if (stringComplete)
  {
    String myString = inputString;
    stringComplete = false;
    inputString = "";
    if (debug) Serial.println(myString);
    ezrotor(myString);
  }
  if (azimuth < azimuthNew)
  {
    moving = true;
    if (azimuth > azimuthNew) azimuth = azimuthNew;
    azimuth += 1;
    position();
    delay(delayTime+startup*delayTimeStartup);  // timing approximates tuner rotation time
    startup = 0;
  }
  else if (azimuth > azimuthNew)
  {
    moving = true;
    if (azimuth < azimuthNew) azimuth = azimuthNew;
    azimuth -= 1;
    position();
    delay(delayTime+startup*delayTimeStartup);  // timing approximates tuner rotation time
    startup = 0;
  }
  else if (moving)
  {
    moving = false;
    lcd.setCursor(0,0);
    lcd.write(HEADER);
  }
  if (azimuth == 0) 
  {
    EEPROM_writePark(PARKED);
  }
  else
  {
    EEPROM_writePark(NOTPARKED);
  }
}
