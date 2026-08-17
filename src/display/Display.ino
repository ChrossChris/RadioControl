#include <SPI.h>
#include <Adafruit_GFX.h>
#include <ILI9341_Fast.h>
#include "C:/Development/Elektronik/Fernsteuerung_FM6014/git-repository/inc/definitions.h"

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
#define MICRO_SECONDS     16




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


uint16_t taskCtrDisplay = 0;

struct DisplayCache
{
  int8_t screen = -1;
  int16_t value[4] = {-32768, -32768, -32768, -32768};
  uint32_t switches = 0xFFFFFFFFUL;
};

DisplayCache displayCache;

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

  // Erstes Datenpaket sofort anfordern; der Sender hat zusätzlich ein Fallback.
  Serial.write(SERIAL_START);
  Serial.write(SERIAL_END);
}


void invalidateDisplayCache(int8_t screen)
{
  displayCache.screen = screen;
  for (uint8_t i = 0; i < 4; i++) displayCache.value[i] = -32768;
  displayCache.switches = 0xFFFFFFFFUL;
}

void updateMeter(const char *label, int16_t value, int maxValue, uint8_t line, int threshold)
{
  const bool firstDraw = displayCache.value[line] == -32768;
  if (firstDraw || abs((long)value - (long)displayCache.value[line]) > threshold)
  {
    if (firstDraw) setLevelMeter(label, value, 0, maxValue, line);
    else           setLevelMeterPartial(displayCache.value[line], value, 0, maxValue, line);
    displayCache.value[line] = value;
  }
}

size_t payloadSizeForScreen(int8_t screen)
{
  if (screen == SCREEN_JOYSTICK_VALUES) return sizeof(ScreenJoystickValues);
  if (screen == SCREEN_JOYSTICK_TRIM) return sizeof(ScreenJoystickTrim);
  if (screen == SCREEN_POTI_CENTER) return sizeof(ScreenPotiCenter);
  if (screen == SCREEN_POTI_LEFT_RIGHT) return sizeof(ScreenPotiLeftRight);
  if (screen == SCREEN_SWITCHES) return 4;
  return 0;
}

void processPacket(const GeneralInfo &generalInfo, const uint8_t *payload)
{
  taskCtrDisplay++;

  if (generalInfo.screen != displayCache.screen)
  {
    tft.clearScreen();
    invalidateDisplayCache(generalInfo.screen);
  }

  if (generalInfo.screen == SCREEN_JOYSTICK_VALUES)
  {
    ScreenJoystickValues data;
    memcpy(&data, payload, sizeof(data));
    updateMeter("Thrust Value",   data.thrustValue,   1023, 0, 4);
    updateMeter("Rudder Value",   data.rudderValue,   1023, 1, 4);
    updateMeter("Elevator Value", data.elevatorValue, 1023, 2, 4);
    updateMeter("Aileron Value",  data.aileronValue,  1023, 3, 4);
  }
  else if (generalInfo.screen == SCREEN_JOYSTICK_TRIM)
  {
    ScreenJoystickTrim data;
    memcpy(&data, payload, sizeof(data));
    updateMeter("Thrust Trim",   data.thrustTrim,   1023, 0, 4);
    updateMeter("Rudder Trim",   data.rudderTrim,   1023, 1, 4);
    updateMeter("Elevator Trim", data.elevatorTrim, 1023, 2, 4);
    updateMeter("Aileron Trim",  data.aileronTrim,  1023, 3, 4);
  }
  else if (generalInfo.screen == SCREEN_POTI_CENTER)
  {
    ScreenPotiCenter data;
    memcpy(&data, payload, sizeof(data));
    const int uBattRef = analogRead(PIN_UBATTREF);
    const int uBattLevel = analogRead(PIN_UBATTLEVEL);
    const int uBattRaw = (uBattRef > 0)
      ? (int)((UZENER + UBATTREF * (float)uBattLevel / (float)uBattRef) * 1000.0f)
      : 0;
    static int uBattFiltered = 0;
    if (uBattFiltered == 0) uBattFiltered = uBattRaw;
    else                    uBattFiltered += (uBattRaw - uBattFiltered) / 8;
    updateMeter("Poti Main",         data.potiMain,        1023, 0, 4);
    updateMeter("Poti Center Left",  data.potiCenterLeft,  1023, 1, 4);
    updateMeter("Poti Center Right", data.potiCenterRight, 1023, 2, 4);
    updateMeter("Battery [mV]",      uBattFiltered,       10000, 3, 25);
  }
  else if (generalInfo.screen == SCREEN_POTI_LEFT_RIGHT)
  {
    ScreenPotiLeftRight data;
    memcpy(&data, payload, sizeof(data));
    updateMeter("Poti Left 1",  data.potiLeft1,      1023, 0, 4);
    updateMeter("Poti Left 2",  data.potiLeft2,      1023, 1, 4);
    updateMeter("Poti Right 1", data.potiRight1,     1023, 2, 4);
    updateMeter("Poti Right 2", data.potiRight2Cont, 1023, 3, 4);
  }
  else if (generalInfo.screen == SCREEN_SWITCHES)
  {
    uint8_t rxMsg[4];
    memcpy(rxMsg, payload, sizeof(rxMsg));
    uint32_t packedSwitches;
    memcpy(&packedSwitches, rxMsg, sizeof(packedSwitches));

    if (packedSwitches != displayCache.switches)
    {
      displayCache.switches = packedSwitches;

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
  }

  char text[40];
  sprintf(text,"C%5u/%5u", generalInfo.taskCtr, taskCtrDisplay);
  setText(text,9,ALIGN_RIGHT);

  Serial.write(SERIAL_START);
  Serial.write(SERIAL_END);

  if (generalInfo.tone == 1) tone(BUZZER, 500);
  else                       noTone(BUZZER);

}

void loop(void)
{
  static uint8_t rxBuffer[32];
  static uint8_t rxPos = 0;
  static uint8_t expectedSize = 0;
  static unsigned long lastByteTime = 0;

  // Ein abgebrochenes Paket verwerfen, bevor neue Bytes verarbeitet werden.
  if (rxPos > 0 && (millis() - lastByteTime) > 100)
  {
    rxPos = 0;
    expectedSize = 0;
  }

  while (Serial.available() > 0)
  {
    const uint8_t value = Serial.read();
    lastByteTime = millis();

    if (rxPos == 0)
    {
      if (value == SERIAL_START)
      {
        rxBuffer[rxPos++] = value;
      }
      continue;
    }

    if (rxPos == 1)
    {
      if (value == SERIAL_START)
      {
        rxBuffer[rxPos++] = value;
      }
      else
      {
        rxPos = 0;
      }
      continue;
    }

    if (rxPos >= sizeof(rxBuffer))
    {
      rxPos = 0;
      expectedSize = 0;
      continue;
    }

    rxBuffer[rxPos++] = value;

    if (rxPos == 2 + sizeof(GeneralInfo))
    {
      GeneralInfo header;
      memcpy(&header, &rxBuffer[2], sizeof(header));
      const size_t payloadSize = payloadSizeForScreen(header.screen);
      if (payloadSize == 0)
      {
        rxPos = 0;
        expectedSize = 0;
        continue;
      }
      expectedSize = 2 + sizeof(GeneralInfo) + payloadSize + 2;
    }

    if (expectedSize > 0 && rxPos == expectedSize)
    {
      if (rxBuffer[expectedSize - 2] == SERIAL_END &&
          rxBuffer[expectedSize - 1] == SERIAL_END)
      {
        GeneralInfo header;
        memcpy(&header, &rxBuffer[2], sizeof(header));
        processPacket(header, &rxBuffer[2 + sizeof(GeneralInfo)]);
      }
      rxPos = 0;
      expectedSize = 0;
    }
  }

}
