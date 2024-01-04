#include <SPI.h>
#include <Adafruit_GFX.h>
#include <ILI9341_Fast.h>
#include "C:/Dokumente/Elektronik & Modellbau/Fernsteuerung_FM6014/git-repository/inc/definitions.h"

#include "RREFont.h"
#include "rre_term_10x16.h"
RREFont font;

// Serielle Schnittstelle
#define BAUD_RATE                 57600
#define SERIAL_START              255
#define SERIAL_END                128

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

void loop(void)
{
  const int uBattRef   = analogRead(PIN_UBATTREF);
  const int uBattLevel = analogRead(PIN_UBATTLEVEL);
  float     uBatt      = (UZENER + UBATTREF * uBattLevel/uBattRef) * 1000.0f;

  static int8_t screen_old = 0;

  static int counter = 0;
  // ----------------------------------------------------------------------------------
  // Datenstrom entsprechend Protokoll über die serielle Schnittstelle einlesen

  byte rxByte = 0;
  while (1)
  {
    if (Serial.available() > 2)
    {
      rxByte = Serial.read();
      if ((rxByte == SERIAL_START) && (Serial.peek() == SERIAL_START)) 
      {
        Serial.read(); // Zweite Start-Byte verwerfen
        break;
      }
    }
  }

  GeneralInfo generalInfo;
  Serial.readBytes((byte*) &generalInfo, sizeof(GeneralInfo));

  // ----------------------------------------------------------------------------------
  // Empfangene Daten verarbeiten und interpretieren
  if (generalInfo.screen != screen_old) tft.clearScreen();

  if (generalInfo.screen == SCREEN_JOYSTICK_VALUES)
  {
    while (Serial.available() < sizeof(ScreenJoystickValues));

    ScreenJoystickValues status_screen;
    Serial.readBytes((byte*) &status_screen, sizeof(ScreenJoystickValues));

    setLevelMeter("Thrust Value",   status_screen.thrustValue,   0, 1023, 0);
    setLevelMeter("Rudder Value",   status_screen.rudderValue,   0, 1023, 1);
    setLevelMeter("Elevator Value", status_screen.elevatorValue, 0, 1023, 2);
    setLevelMeter("Aileron Value",  status_screen.aileronValue,  0, 1023, 3);
  }

  else if (generalInfo.screen == SCREEN_JOYSTICK_TRIM)
  {
    while (Serial.available() < sizeof(ScreenJoystickTrim));

    ScreenJoystickTrim status_screen;
    Serial.readBytes((byte*) &status_screen, sizeof(ScreenJoystickTrim));

    setLevelMeter("Thrust Trim",   status_screen.thrustTrim,   0, 1023, 0);
    setLevelMeter("Rudder Trim",   status_screen.rudderTrim,   0, 1023, 1);
    setLevelMeter("Elevator Trim", status_screen.elevatorTrim, 0, 1023, 2);
    setLevelMeter("Aileron Trim",  status_screen.aileronTrim,  0, 1023, 3);
  }

  else if (generalInfo.screen == SCREEN_POTI_CENTER)
  {
    while (Serial.available() < sizeof(ScreenPotiCenter));

    ScreenPotiCenter status_screen;
    Serial.readBytes((byte*) &status_screen, sizeof(ScreenPotiCenter));

    setLevelMeter("Poti Main",         status_screen.potiMain,       0, 1023,  0);
    setLevelMeter("Poti Center Left",  status_screen.potiCenterLeft, 0, 1023,  1);
    setLevelMeter("Poti Center Right", status_screen.potiCenterRight,0, 1023,  2);
    setLevelMeter("Battery [mV]",      uBatt,                         0, 10000, 3);
  }

  else if (generalInfo.screen == SCREEN_POTI_LEFT_RIGHT)
  {
    while (Serial.available() < sizeof(ScreenPotiLeftRight));

    ScreenPotiLeftRight status_screen;
    Serial.readBytes((byte*) &status_screen, sizeof(ScreenPotiLeftRight));

    setLevelMeter("Poti Left 1",  status_screen.potiLeft1,      0, 1023, 0);
    setLevelMeter("Poti Left 2",  status_screen.potiLeft2,      0, 1023, 1);
    setLevelMeter("Poti Right 1", status_screen.potiRight1,     0, 1023, 2);
    setLevelMeter("Poti Right 2", status_screen.potiRight2Cont, 0, 1023, 3);
  }

  else if (generalInfo.screen == SCREEN_SWITCHES)
  {
    while (Serial.available() < 4);

    uint8_t rxMsg[4];
    Serial.readBytes((byte*) rxMsg, 4);

    int8_t  switchLeft[2]       = {0};
    int8_t  switchRight[2]      = {0};
    int8_t  switchRightRotary   = 0;
    int8_t  buttonRight         = 0;
    int8_t  switchCenter[6]     = {0};
    int8_t  toggleButton[2]     = {0};
    
    bitWrite(switchLeft[0],     0, bitRead(rxMsg[0], 0));
    bitWrite(switchLeft[1],     0, bitRead(rxMsg[0], 1));
    bitWrite(switchRight[0],    0, bitRead(rxMsg[0], 2));
    bitWrite(switchRight[1],    0, bitRead(rxMsg[0], 3));
    bitWrite(switchRight[1],    1, bitRead(rxMsg[0], 4));
    bitWrite(switchRightRotary, 0, bitRead(rxMsg[0], 5));
    bitWrite(switchRightRotary, 1, bitRead(rxMsg[0], 6));
    bitWrite(buttonRight,       0, bitRead(rxMsg[0], 7));

    bitWrite(switchCenter[0],   0, bitRead(rxMsg[1], 0));
    bitWrite(switchCenter[0],   1, bitRead(rxMsg[1], 1));
    bitWrite(switchCenter[1],   0, bitRead(rxMsg[1], 2));
    bitWrite(switchCenter[1],   1, bitRead(rxMsg[1], 3));
    bitWrite(switchCenter[2],   0, bitRead(rxMsg[1], 4));
    bitWrite(switchCenter[2],   1, bitRead(rxMsg[1], 5));
    bitWrite(switchCenter[3],   0, bitRead(rxMsg[1], 6));
    bitWrite(switchCenter[3],   1, bitRead(rxMsg[1], 7));

    bitWrite(switchCenter[4],   0, bitRead(rxMsg[2], 0));
    bitWrite(switchCenter[4],   1, bitRead(rxMsg[2], 1));
    bitWrite(switchCenter[5],   0, bitRead(rxMsg[2], 2));
    bitWrite(switchCenter[5],   1, bitRead(rxMsg[2], 3));

    toggleButton[0] = 0;
    toggleButton[1] = 0;
    bitWrite(toggleButton[0],   0, bitRead(rxMsg[3], 0));
    bitWrite(toggleButton[0],   1, bitRead(rxMsg[3], 1));
    bitWrite(toggleButton[0],   2, bitRead(rxMsg[3], 2));
    bitWrite(toggleButton[1],   0, bitRead(rxMsg[3], 3));
    bitWrite(toggleButton[1],   1, bitRead(rxMsg[3], 4));
    bitWrite(toggleButton[1],   2, bitRead(rxMsg[3], 5));
    toggleButton[0] -= 3;
    toggleButton[1] -= 3;

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

  screen_old = generalInfo.screen;

  Serial.write(SERIAL_START);
  Serial.write(SERIAL_END);

  counter++;
  if (counter >= 100)
  {
    tone(BUZZER, 500);
    delay(150);
    noTone(BUZZER);
    counter = 0;
  }
}
