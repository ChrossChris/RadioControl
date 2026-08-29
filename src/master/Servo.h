// Servo.h
#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>

namespace ServoConfig
{
  constexpr uint16_t CENTER_VALUE   = 1500; // 1,5ms: Mittelstellung des PPM-Signals, das den Servo ansteuert
  constexpr uint16_t LEVEL          = 500;  // 0,5ms: Signalhub des PPM-Signals, mit dem der Servo gesteuert wird.

  // PPM-Kanäle (noch nicht ganz sicher, ob das hierhin gehört)
  constexpr uint8_t PPM_MOTOR       = 0;
  constexpr uint8_t PPM_HOEHE       = 1;
  constexpr uint8_t PPM_SEITE       = 2;
  constexpr uint8_t PPM_QUER_LINKS  = 3;
  constexpr uint8_t PPM_QUER_RECHTS = 4;
  constexpr uint8_t PPM_FLAP_LINKS  = 5;
  constexpr uint8_t PPM_FLAP_RECHTS = 6;
}

using PpmValue = uint16_t;

// Definitionen/Konfiguration für die vorliegende RC-Hardware, muss bei anderer Hardware entsprechend ausgelesen und
// angepasst werden.


class Servo
{

public:
  explicit Servo(const uint8_t channel, const int16_t maxControlValue, const uint8_t maxTrimmLevelPercent);

  void setInverseDirection();
  void setForwardDirection();

  void setTrimm(int16_t trimmPercent);

  void addActuation(int16_t addValue);
  void clearServo();

  PpmValue getPpmSignal() const;
  uint8_t  getPpmChannel() const;

private:
  uint8_t  ppmChannel  = 255;
  
  int16_t  controlLimit = 0;
  int16_t  trimmLimit   = 0;
  int16_t  trimmValue   = 0;
  
  bool     inverseDirection = false;
  int32_t  actuationValue;

};


#endif