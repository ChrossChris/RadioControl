// Joysticks.h
#ifndef JOYSTICKS_H
#define JOYSTICKS_H

#include <Arduino.h>

// Definitionen/Konfiguration für die vorliegende RC-Hardware, muss bei anderer Hardware entsprechend ausgelesen und
// angepasst werden.

namespace JoystickConfig
{
  // Analoge Kanäle für die Joysticks und Trimmer
  constexpr uint8_t THRUST            = A0;
  constexpr uint8_t TRIMM_THRUST      = A1;
  constexpr uint8_t RUDDER            = A2;
  constexpr uint8_t TRIMM_RUDDER      = A3;
  constexpr uint8_t ELEVATOR          = A4;
  constexpr uint8_t TRIMM_ELEVATOR    = A5;
  constexpr uint8_t AILERON           = A6;
  constexpr uint8_t TRIMM_AILERON     = A7;

  // Poti-Grenzen der Joysticks
  constexpr int16_t POTI_MAX_THRUST   = 880; 
  constexpr int16_t POTI_MIN_THRUST   = 114;
  constexpr int16_t POTI_MAX_RUDDER   = 896; 
  constexpr int16_t POTI_MIN_RUDDER   = 106;
  constexpr int16_t POTI_MAX_ELEVATOR = 885; 
  constexpr int16_t POTI_MIN_ELEVATOR = 127;
  constexpr int16_t POTI_MAX_AILERON  = 920; 
  constexpr int16_t POTI_MIN_AILERON  = 125;

  // Deadzone um die Mittelstellung der Joysticks, in der der Wert auf 0 gesetzt wird (ADC-Wert)
  constexpr int16_t DEADBAND_THRUST   = 50;
  constexpr int16_t DEADBAND_RUDDER   = 25;
  constexpr int16_t DEADBAND_ELEVATOR = 25;
  constexpr int16_t DEADBAND_AILERON  = 25;
  constexpr bool    INVERSE_THRUST    = false;
  constexpr bool    INVERSE_RUDDER    = true;
  constexpr bool    INVERSE_ELEVATOR  = false;
  constexpr bool    INVERSE_AILERON   = true;
}


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

  int16_t  getValue()           const { return value;          };
  int16_t  getValueRaw()        const { return valueRaw;       };
  int16_t  getValueNormalized() const { return valueNormalized;};
  int16_t  getTrimmValueRaw()   const { return trimm;          };
  int16_t  getControlLimit()    const { return controlLimit;   };

  void     setDeadBand(const int16_t centerDeadBandADC);
  void     setDirection(const bool inverseDirection);

  void     setExpoCurve(const int16_t expoRatePermille);
  void     setDualRate(const int16_t lowerRatePermille, const int16_t upperRatePermille);
  void     setXYCurve(const int16_t xCurvePercent[], const int16_t yCurvePercent[], const uint8_t numPoints);

  void     resetExpoCurve();
  void     resetXYCurve();
  void     resetDualRate();
  
private:
  static constexpr int16_t controlLimit = 10000; // Obergrenze für den normalisierten Joystickwert
  static constexpr int16_t inputRange   = 1000;  // Obergrenze für den ADC-Wert des Joysticks (10 Bit ADC), inklusive Totzone an den Rändern (0..1000, 0..100.0%)
  static constexpr uint8_t sizeXYcurve  = 31;    // Maximale Anzahl an Punkten für die alternative Kennlinie des Joysticks (x-y-Kurve)

  JoystickType joystickType;

  void applyExpoCurve();
  void applyDualRate();
  void applyXYCurve();

  // Meine Empfehlung:
  // - ADC-Werte, Grenzen und Differenzen: int16_t
  // - Pins und kleine Zustände: uint8_t
  // - Mittelwertsumme: int32_t
  // - Zeitwerte von millis(): uint32_t
  // - Zähler nach benötigtem Wertebereich
  // Typkorrektheit und nachvollziehbare Arithmetik sind hier wichtiger als die minimale Ersparnis einzelner CPU-Takte.
  // Die Verwendung von int16_t ist für diese Aufgabe auf einem 8-Bit-Arduino völlig angemessen.
  uint8_t  pin_joystick         = 0;        // ADC-Input-Pin für den Joystick-Poti
  uint8_t  pin_trimm            = 0;        // ADC-Input-Pin für den Trimmer-Poti
  int16_t  valueRaw             = 0;        // Rohwert des Joysticks (ADC-Wert)
  int16_t  valueNormalized      = 0;        // Normalisierter Wert des Joysticks bevor weiter Expo-, xy-Kennlinien- oder Dual-Rate-Manipulationen erfolgen
  int16_t  value                = 0;        // Normalisierter Wert des Joysticks (-controlLimit..+controlLimit)
  int16_t  trimm                = 0;        // Rohwert des Trimmers (ADC-Wert) -> Dieser wird erstmal nicht normalisiert, so ganz weiß ich noch nicht, wie der weiter verarbeitet werden soll.
  int16_t  minPoti              = 0;        // Minimalwert des Joystick-Potis (ADC-Wert)
  int16_t  maxPoti              = 0;        // Maximalwert des Joystick-Potis (ADC-Wert)
  int16_t  centerPos            = 0;        // Mittelwert des Joystick-Potis (ADC-Wert) -> Wird beim Setup aus der Ruhelage des Joysticks ermittelt (außer beim Thrust-Joystick, der hat keine Mittelstellung)
  int16_t  deadBand             = 0;        // +/-Deadzone um die Mittelstellung, in dem der Joystickwert auf 0 gesetzt wird (ADC-Wert), um kleine Abweichungen in der Ruhelage zu ignorieren. 
  bool     thrust_config        = false;    // Zur Unterscheidung, ob es sich um den Thrust-Joystick handelt, der keine Mittelstellung hat und daher nur positive Werte liefert.
  
  // Member-Variablen für die weitere Manipulation der Joystickwerte, z.B. Dual-Rate und Expo-Kurve
  int16_t  upperRateLimit       = 1000;     // Permilleteil (direkte Verwendung des ADC-Werts) des zu verwenden Joystickwegs für die obere Hälfte des Wertebereichs (0..+controlLimit)
  int16_t  lowerRateLimit       = 1000;     // Permilleteil (direkte Verwendung des ADC-Werts) des zu verwenden Joystickwegs für die untere Hälfte des Wertebereichs (-controlLimit..0)
  float    expoPower            = 1.0F;     // Expo-Funktion, wid aus dem Permillezwert expoRatePermille (0..1000‰) berechnet und auf 1.0..4.0 gemappt
  uint8_t  xyCurvePoints        = 0;        // Anzahl der Punkte in der x-y-Kurve, die zur alternativen Kennlinie verwendet werden sollen.
  int16_t  xCurve[sizeXYcurve];             // Array für x-y-Kurve zur alternativen Kennlinie, falls die Exponentialfunktion nicht ausreicht.
  int16_t  yCurve[sizeXYcurve];             // Array für x-y-Kurve zur alternativen Kennlinie, falls die Exponentialfunktion nicht ausreicht.
  
  bool     useExpoCurve         = false;    // Flag, ob die Expo-Funktion angewendet werden soll (x-y-Kurve wird deaktiviert, Logik erfolgt im Setzen von Expo)
  bool     useXYCurve           = false;    // Flag, ob die x-y-Kurve angewendet werden soll (Expo-Funktion wird deaktiviert, Logik erfolgt im Setzen der xy-Kurve)
  bool     useDualRate          = false;    // Flag, ob DualRate angewendet werden soll.
  bool     inverseDirection     = false;    // Zur Umkehrung der Richtung des Joysticks (unabhängig zunächst vom Servo).

};



#endif

