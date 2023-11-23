#include <SPI.h>
#include <Adafruit_GFX.h>
#include <ILI9341_Fast.h>

#include "RREFont.h"
#include "rre_term_10x16.h"
RREFont font;

// Serielle Schnittstelle
#define BAUD_RATE                 57600
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

#define BUZZER        3
#define UBATTREF      7



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
int   uBattRef       = 0;
int   screen         = 0;
  

void loop(void)
{
  static int counter = 0;
  // ----------------------------------------------------------------------------------
  // Datenstrom entsprechend Protokoll über die serielle Schnittstelle einlesen
  byte rxMsg[20] = {0};
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
  uBattRef = analogRead(UBATTREF);


  // ----------------------------------------------------------------------------------
  // Empfangene Daten verarbeiten und interpretieren
  thrustValue   = (unsigned int)  word(rxMsg[ 0],rxMsg[ 1]);  
  rudderValue   = (unsigned int)  word(rxMsg[ 2],rxMsg[ 2]);  
  elevatorValue = (unsigned int)  word(rxMsg[ 4],rxMsg[ 5]);  
  aileronValue  = (unsigned int)  word(rxMsg[ 6],rxMsg[ 7]);  
  thrustTrim    = (unsigned int)  word(rxMsg[ 8],rxMsg[ 9]);  
  rudderTrim    = (unsigned int)  word(rxMsg[10],rxMsg[11]);  
  elevatorTrim  = (unsigned int)  word(rxMsg[12],rxMsg[13]);  
  aileronTrim   = (unsigned int)  word(rxMsg[14],rxMsg[15]);  
  screen        = (unsigned int)  word(rxMsg[16],rxMsg[17]);
  

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
    setLevelMeter("Battery",        uBattRef,      0, 1023, 0);
    setLevelMeter("Rudder Value",   rudderValue,   0, 1023, 1);
    setLevelMeter("Elevator Value", elevatorValue, 0, 1023, 2);
    setLevelMeter("Aileron Value",  aileronValue,  0, 1023, 3);
  }
//  else
//  {
//    setLevelMeter("Screen",         screen,      0, 1023, 0);
//    setLevelMeter("Screen",         screen,      0, 1023, 1);
//    setLevelMeter("Screen",         screen,      0, 1023, 2);
//    setLevelMeter("Screen",         screen,      0, 1023, 3);
//  }


  counter++;
  if (counter >= 100)
  {
    tone(BUZZER, 500);
    delay(500
    noTone(BUZZER);
    counter = 0;
  }
}
