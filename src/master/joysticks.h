// Joysticks.h
#ifndef JOYSTICKS_H
#define JOYSTICKS_H

#include <Arduino.h>

// Analoge Kanäle für die Joysticks und Trimmer
#define JOYSTICK_THRUST           A0
#define TRIMM_THRUST              A1
#define JOYSTICK_RUDDER           A2
#define TRIMM_RUDDER              A3
#define JOYSTICK_ELEVATOR         A4
#define TRIMM_ELEVATOR            A5
#define JOYSTICK_AILERON          A6
#define TRIMM_AILERON             A7

// Poti-Grenzen der Joysticks
#define POTI_MAX_THRUST           880 
#define POTI_MIN_THRUST           114
#define POTI_MAX_RUDDER           896 
#define POTI_MIN_RUDDER           106
#define POTI_MAX_ELEVATOR         885 
#define POTI_MIN_ELEVATOR         127
#define POTI_MAX_AILERON          920 
#define POTI_MIN_AILERON          125

// Deadzone um die Mittelstellung der Joysticks, in der der Wert auf 0 gesetzt wird (ADC-Wert)
#define DEADBAND_THRUST           50
#define DEADBAND_RUDDER           25
#define DEADBAND_ELEVATOR         25
#define DEADBAND_AILERON          25

#define CONTROL_LIMIT             10000  // Obergrenze für den normalisierten Joystickwert
#define MAX_XY_POINTS             10     // Maximale Anzahl an Punkten für die alternative Kennlinie (x-y-Kurve)


enum class JoystickType : uint8_t
{
  THRUST,
  RUDDER,
  ELEVATOR,
  AILERON
};

class Joystick
{

public:
  explicit Joystick(const JoystickType type);
  
  void     setup();

  int16_t  update();

  int16_t  getValue()         const { return value;         };
  int16_t  getRawValue()      const { return rawValue;      };
  int16_t  getRawTrimm()      const { return trimm;         };
  int16_t  getControlLimit()  const { return CONTROL_LIMIT; };

  void     setDeadBand(const int16_t centerDeadBand);
  void     setDirection(const bool inverseServoDirection);

  void     setExpoCurve(const int16_t expoRatePercent);
  void     setDualRate(const int16_t upperRatePercent, const int16_t lowerRatePercent);
  void     setXYCurve(const int16_t xCurve[], const int16_t yCurve[], const uint8_t numPoints);
  
private:
  void applyExpoCurve();
  void applyDualRate();
  void applyXYCurve();


  JoystickType joystickType;

  // Meine Empfehlung:
  // - ADC-Werte, Grenzen und Differenzen: int16_t
  // - Pins und kleine Zustände: uint8_t
  // - Mittelwertsumme: int32_t
  // - Zeitwerte von millis(): uint32_t
  // - Zähler nach benötigtem Wertebereich
  // Typkorrektheit und nachvollziehbare Arithmetik sind hier wichtiger als die minimale Ersparnis einzelner CPU-Takte.
  // Die Verwendung von int16_t ist für diese Aufgabe auf einem 8-Bit-Arduino völlig angemessen.
  uint8_t  pin_joystick     = 0;              // ADC-Input-Pin für den Joystick-Poti
  uint8_t  pin_trimm        = 0;              // ADC-Input-Pin für den Trimmer-Poti
  int16_t  rawValue         = 0;              // Rohwert des Joysticks (ADC-Wert)
  int16_t  value            = 0;              // Normalisierter Wert des Joysticks (-controlLimit..+controlLimit)
  int16_t  trimm            = 0;              // Rohwert des Trimmers (ADC-Wert) -> Dieser wird erstmal nicht normalisiert, so ganz weiß ich noch nicht, wie der weiter verarbeitet werden soll.
  int16_t  minPoti          = 0;              // Minimalwert des Joystick-Potis (ADC-Wert)
  int16_t  maxPoti          = 0;              // Maximalwert des Joystick-Potis (ADC-Wert)
  int16_t  centerPos        = 0;              // Mittelwert des Joystick-Potis (ADC-Wert) -> Wird beim Setup aus der Ruhelage des Joysticks ermittelt (außer beim Thrust-Joystick, der hat keine Mittelstellung)
  int16_t  deadBand         = 0;              // +/-Deadzone um die Mittelstellung, in dem der Joystickwert auf 0 gesetzt wird (ADC-Wert), um kleine Abweichungen in der Ruhelage zu ignorieren. 
  bool     thrust_config    = false;          // Zur Unterscheidung, ob es sich um den Thrust-Joystick handelt, der keine Mittelstellung hat und daher nur positive Werte liefert.
  
  // Member-Variablen für die weitere Manipulation der Joystickwerte, z.B. Dual-Rate und Expo-Kurve
  int8_t   upperRatePercent = 100;            // Prozentteil des zu verwenden Joystickwegs für die obere Hälfte des Wertebereichs (0..+controlLimit)
  int8_t   lowerRatePercent = 100;            // Prozentteil des zu verwenden Joystickwegs für die untere Hälfte des Wertebereichs (-controlLimit..0)
  int8_t   expoRatePercent  = 0;              // Exponentialwert für die nichtlineare Kennlinie des Joysticks (0..100)
  int8_t   x[MAX_XY_POINTS] = {0};            // Array für x-y-Kurve zur alternativen Kennlinie, falls die Exponentialfunktion nicht ausreicht.
  int8_t   y[MAX_XY_POINTS] = {0};            // Array für x-y-Kurve zur alternativen Kennlinie, falls die Exponentialfunktion nicht ausreicht.
  uint8_t  xyCurvePoints    = 0;              // Anzahl der Punkte in der x-y-Kurve, die zur alternativen Kennlinie verwendet werden sollen.
  bool     useExpoCurve     = true;           // Flag, ob die Exponentialfunktion oder stattdessen die x-y-Kurve angewendet werden soll.

  bool     inverseDirection = false;          // Zur Umkehrung der Richtung des Joysticks, falls der Servo in die entgegengesetzte Richtung angesteuert werden soll.

};



#endif

