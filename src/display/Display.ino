#include <SPI.h>
#include <Adafruit_GFX.h>
#include <ILI9341_Fast.h>

#include "RREFont.h"
#include "rre_term_10x16.h"
RREFont font;

// Serielle Schnittstelle
#define BAUD_RATE                 9600
#define SERIAL_START              255
#define SERIAL_END                128
#define UPDATE_PERIOD             100

// ILI9341 240x320 2.4" tft
#define TFT_WIDTH     320
#define TFT_HEIGHT    240
#define TFT_POWER_ON  7
#define DATA          10
#define CHIP_SELECT   9
#define RESET         8
ILI9341 tft = ILI9341(DATA, RESET, CHIP_SELECT);

#define BUZZER            3
#define PIN_UBATTREF      0
#define PIN_UBATTLEVEL    1
#define UBATTREF          2.918F
#define UZENER            4.58F



// -----------------------------------------------------------------------------------------------
// Tonausgabe
struct ToneSequence
{
  ToneSequence(int _freq, int _duration, int _pause)
  : frequency(_freq)
  , duration(_duration)
  , pause(_pause)
  {};
 
  int frequency = 0;
  int duration  = 0;
  int pause     = 0;
};
typedef ToneSequence TS;

// needed for RREFont library initialization, define your fillRect
void customRect(int x, int y, int w, int h, int c) 
{
  return tft.fillRect(x, y, w, h, c);
}


void setup()
{
 // put your setup code here, to run once:
  pinMode(TFT_POWER_ON, OUTPUT);
  digitalWrite(TFT_POWER_ON, HIGH);

	tft.init();
  tft.fillScreen(BLACK);
  tft.setRotation(3);
  font.init(customRect, TFT_WIDTH, TFT_HEIGHT);
  
  Serial.begin(BAUD_RATE);
  
  pinMode(BUZZER, OUTPUT);

}


int   thrustValue    = 0;
int   rudderValue    = 0;
int   elevatorValue  = 0;
int   aileronValue   = 0;
int   thrustTrim     = 0;
int   rudderTrim     = 0;
int   elevatorTrim   = 0;
int   aileronTrim    = 0;
int   potiMain       = 0;
int   potiCenterLeft = 0;
int   potiCenterRight= 0;
int   potiLeft1      = 0;
int   potiLeft2      = 0;
int   potiRight1     = 0;
int   potiRight2Cont = 0;

int8_t  switchLeft[2]       = {0};
int8_t  switchRight[2]      = {0};
int8_t  switchRightRotary   = 0;
int8_t  buttonRight         = 0;
int8_t  switchCenter[6]     = {0};
int8_t  toggleButton[2]     = {0};

int   screen         = 0;
int   screen_old     = 0;
int   uBattRef       = 0;
int   uBattLevel     = 0;



void loop(void)
{
  static int counter = 0;
  byte   buffer      = 0;
  // ----------------------------------------------------------------------------------
  // Datenstrom entsprechend Protokoll über die serielle Schnittstelle einlesen
  byte rxMsg[25] = {0};
  byte amount    = 0;

  byte rxByte = 0;
  while (1)
  {
    if (Serial.available() > 2)
    {
      rxByte = Serial.read();
      if ((rxByte == SERIAL_START) && (Serial.peek() == SERIAL_START)) 
      {
        Serial.read(); // Zweite Start-Byte verwerfen
        amount = Serial.read();
        break;
      }
    }
  }
  Serial.readBytes(rxMsg, amount);
  
  uBattRef   = analogRead(PIN_UBATTREF);
  uBattLevel = analogRead(PIN_UBATTLEVEL);


  // ----------------------------------------------------------------------------------
  // Empfangene Daten verarbeiten und interpretieren
  screen = rxMsg[0];
  byte idxMsg = 1;

  switch (screen)
  {
    case 0:
      if (amount != 11) break;
      thrustValue     = word(rxMsg[idxMsg+0],rxMsg[idxMsg+1]);
      rudderValue     = word(rxMsg[idxMsg+2],rxMsg[idxMsg+3]);
      elevatorValue   = word(rxMsg[idxMsg+4],rxMsg[idxMsg+5]);
      aileronValue    = word(rxMsg[idxMsg+6],rxMsg[idxMsg+7]);
      break;
      
    case 1:
      if (amount != 11) break;
      thrustTrim      = word(rxMsg[idxMsg+0],rxMsg[idxMsg+1]);
      rudderTrim      = word(rxMsg[idxMsg+2],rxMsg[idxMsg+3]);
      elevatorTrim    = word(rxMsg[idxMsg+4],rxMsg[idxMsg+5]);
      aileronTrim     = word(rxMsg[idxMsg+6],rxMsg[idxMsg+7]);
      break;
      
    case 2:
      if (amount != 9) break;
      potiMain        = word(rxMsg[idxMsg+0],rxMsg[idxMsg+1]);
      potiCenterLeft  = word(rxMsg[idxMsg+2],rxMsg[idxMsg+3]);
      potiCenterRight = word(rxMsg[idxMsg+4],rxMsg[idxMsg+5]);
      break;

    case 3:
      if (amount != 11) break;
      potiLeft1       = word(rxMsg[idxMsg+0],rxMsg[idxMsg+1]);
      potiLeft2       = word(rxMsg[idxMsg+2],rxMsg[idxMsg+3]);
      potiRight1      = word(rxMsg[idxMsg+4],rxMsg[idxMsg+5]);
      potiRight2Cont  = word(rxMsg[idxMsg+6],rxMsg[idxMsg+7]);
      break;

    case 4:
      if (amount != 7) break;

      buffer = rxMsg[idxMsg+0];
      bitWrite(switchLeft[0],0,bitRead(buffer,0));
      bitWrite(switchLeft[1],0,bitRead(buffer,1));
      bitWrite(switchRight[0],0,bitRead(buffer,2));
      bitWrite(switchRight[0],1,bitRead(buffer,3));
      bitWrite(switchRight[1],0,bitRead(buffer,4));
      bitWrite(switchRight[1],1,bitRead(buffer,5));
      bitWrite(switchRightRotary,0,bitRead(buffer,6));
      bitWrite(switchRightRotary,1,bitRead(buffer,7));

      buffer = rxMsg[idxMsg+1];
      bitWrite(buttonRight,0,bitRead(buffer,0));
      bitWrite(switchCenter[0],0,bitRead(buffer,1));
      bitWrite(switchCenter[0],1,bitRead(buffer,2));
      bitWrite(switchCenter[1],0,bitRead(buffer,3));
      bitWrite(switchCenter[1],1,bitRead(buffer,4));
      bitWrite(switchCenter[2],0,bitRead(buffer,5));
      bitWrite(switchCenter[2],1,bitRead(buffer,6));

      buffer = rxMsg[idxMsg+2];
      bitWrite(switchCenter[3],0,bitRead(buffer,0));
      bitWrite(switchCenter[3],1,bitRead(buffer,1));
      bitWrite(switchCenter[4],0,bitRead(buffer,2));
      bitWrite(switchCenter[4],1,bitRead(buffer,3));
      bitWrite(switchCenter[5],0,bitRead(buffer,4));
      bitWrite(switchCenter[5],1,bitRead(buffer,5));

      buffer = rxMsg[idxMsg+3];
      toggleButton[0] = 0;
      toggleButton[1] = 0;
      bitWrite(toggleButton[0],0,bitRead(buffer,0));
      bitWrite(toggleButton[0],1,bitRead(buffer,1));
      bitWrite(toggleButton[0],2,bitRead(buffer,2));
      bitWrite(toggleButton[1],0,bitRead(buffer,3));
      bitWrite(toggleButton[1],1,bitRead(buffer,4));
      bitWrite(toggleButton[1],2,bitRead(buffer,5));
      toggleButton[0] -= 3;
      toggleButton[1] -= 3;
  }

  if (screen != screen_old) tft.clearScreen();

  if (screen == 0)
  {
    setLevelMeter("Thrust Value",   thrustValue,   0, 1023, 0);
    setLevelMeter("Rudder Value",   rudderValue,   0, 1023, 1);
    setLevelMeter("Elevator Value", elevatorValue, 0, 1023, 2);
    setLevelMeter("Aileron Value",  aileronValue,  0, 1023, 3);
  }
  else if (screen == 1)
  {
    setLevelMeter("Thrust Trim",   thrustTrim,   0, 1023, 0);
    setLevelMeter("Rudder Trim",   rudderTrim,   0, 1023, 1);
    setLevelMeter("Elevator Trim", elevatorTrim, 0, 1023, 2);
    setLevelMeter("Aileron Trim",  aileronTrim,  0, 1023, 3);
  }
  else if (screen == 2)
  {
    float uBatt = (UZENER + UBATTREF * uBattLevel/uBattRef) * 1000.0f;
    setLevelMeter("Poti Main",        potiMain,       0, 1023,  0);
    setLevelMeter("Poti Center Left", potiCenterLeft, 0, 1023,  1);
    setLevelMeter("Poti Center Right",potiCenterRight,0, 1023,  2);
    setLevelMeter("Battery [mV]",     uBatt,          0, 10000, 3);
  }
  else if (screen == 3)
  {
    setLevelMeter("Poti Left 1",      potiLeft1,      0, 1023, 0);
    setLevelMeter("Poti Left 2",      potiLeft2,      0, 1023, 1);
    setLevelMeter("Poti Right 1",     potiRight1,     0, 1023, 2);
    setLevelMeter("Poti Right 2",     potiRight2Cont, 0, 1023, 3);
  }

  else if (screen == 4)
  {
    char text[50];
    sprintf(text,"Schalter und Taster");
    setText(text,0);
    sprintf(text,"----------------------------------");
    setText(text,1);
    
    sprintf(text,"SW_L1: %d   |   SW_L2: %d   |   Button: %d", switchLeft[0], switchLeft[1], buttonRight);
    setText(text,2);
    
    sprintf(text,"SW_R1: %d   |   SW_R2: %d   |   SW_Rotary: %d", switchRight[0], switchRight[1], switchRightRotary);
    setText(text,3);
  
    sprintf(text,"SW_C1: %d   |   SW_C2: %d   |   SW_C3: %d", switchCenter[0], switchCenter[1], switchCenter[2]);
    setText(text,4);
    
    sprintf(text,"SW_C4: %d   |   SW_C5: %d   |   SW_C6: %d", switchCenter[3], switchCenter[4], switchCenter[5]);
    setText(text,5);

    sprintf(text,"ToggleButton1: %3d", toggleButton[0]);
    setText(text,6);
    
    sprintf(text,"ToggleButton2: %3d", toggleButton[1]);
    setText(text,7);

    sprintf(text,"Rising: +/-3   Cons: +/-1  Falling: +/-1");
    setText(text,8,ALIGN_CENTER);
    
    sprintf(text,"Für Screenwechsel zusätzlich Taster!");
    setText(text,9,ALIGN_CENTER);
  }

  screen_old = screen;

//  screen = 4;
//  switchLeft[0]++;
//  switchLeft[1]++;
//  switchRight[0]++;
//  switchRight[1]++;
//  switchRightRotary++;
//  buttonRight++;
//  switchCenter[0]++;
//  switchCenter[1]++;
//  switchCenter[2]++;
//  switchCenter[3]++;
//  switchCenter[4]++;
//  switchCenter[5]++;
//  toggleButton[0]++;
//  toggleButton[1]++;
  


  counter++;
  if (counter >= 100)
  {
    tone(BUZZER, 500);
    delay(150);
    noTone(BUZZER);
    counter = 0;
  }
}
