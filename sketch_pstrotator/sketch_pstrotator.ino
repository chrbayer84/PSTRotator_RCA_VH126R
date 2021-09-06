#include <IRremote.h>

// Codes for RCA rotator IR
/*
init EF10F906
LEFT EE11F906
RIGHT ED12F906
MEM EC13F906
A FB04F906
B fb05F906
C fb06F906
D fb07F906
G fb08F906
H fb09F906
I fb0AF906
J fb0BF906
K fb0CF906
L fb0DF906
*/

// IR receiver pin
const int recvPin = 11;
// IR transmit pin
const int sendPin = 3;
IRrecv irrecv(recvPin);
IRsend irsend(sendPin);
decode_results results;
// Our switch button for parking the antenna
const int buttonPin = 12; // Button pin. Pulled up. Triggered when connected to ground.


// Misc items to keep track of
int azimuth=0;
int azimuthNew=0;
char ircode='A';
int count=0;
String inputString = "";         // a String to hold incoming data
int delayTime = 450;  // ms to use for simulating rotation delay
int debug=0;
int degreesPer = 30;  // Use 30 for 360 degree and 15 for 180 degree rotation

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(4800);
  if (debug)
    Serial.println("EZRotor to RCA controller v1.0 by W9MDB");

  inputString.reserve(8);
    
  pinMode(buttonPin, INPUT_PULLUP);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  irrecv.begin(recvPin, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  IrSender.begin(sendPin, ENABLE_LED_FEEDBACK); 
  delay(1000);
  park();
}

void serialEvent() {
  int icmd;
  
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (debug)
    {
      Serial.write(inChar);
      Serial.flush();
    }
    if (inputString.length() > 8) inputString = ""; // something is wrong so drop it.

    if (inChar == ';' || inChar == 0x0d)
    {
      if ((inputString.length() == 7) && (sscanf(inputString.c_str(),"AP1%d",&azimuthNew) == 1))
      {
        inputString = "";
        move(azimuthNew);
        // simulate rotation speed
        ircode = az2char(azimuthNew);
        azimuthNew = (ircode-'A')*30;
        if (azimuthNew > azimuth) azimuth += 10;
        else azimuth -= 10;
        if (azimuth < 0) azimuth = 0;
        if (abs(azimuth - azimuthNew) < 15) azimuth = azimuthNew;
      }
      else if (sscanf(inputString.c_str(),"AI%d;",&icmd)==1)
      {
        inputString = "";
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

void park()
{
    if (debug) Serial.println("INIT Antenna to azimuth=0\r\n");
    IrSender.sendNEC(0x6, 0x10, 0x05);
}

void move(int azimuth)
{
  char letter = az2char(azimuth);
  int cmd = 0x04 + (letter - 'A');
  if (debug) {Serial.print("Move to ");Serial.print(azimuth);Serial.print("=");Serial.println(letter);}
  IrSender.sendNEC(0x6, cmd, 5);
}

// the loop function runs over and over again forever
void loop() {
  
#if 0 // only if we want to see the codes being transmitted
  if (irrecv.decode())
  {
    //Serial.print("IR Recv: ");
    //irrecv.printIRResultShort(&Serial);
    //Serial.println(results.value, HEX);
    //Serial.flush();
    irrecv.resume();
  }
#endif
  if (azimuth < azimuthNew)
  {
    azimuth += 10;
    if (azimuth > azimuthNew) azimuth= azimuthNew;
    delay(delayTime);  // timing approximates tuner rotation time
  }
  else if (azimuth > azimuthNew)
  {
    azimuth -= 10;
    if (azimuth < azimuthNew) azimuth= azimuthNew;
    delay(delayTime);
  }

  if (digitalRead(buttonPin) == LOW)
  {  
    if (debug) Serial.println("Sending INIT");
    IrSender.sendNEC(0x6, 0x10, 5);
  }
}
