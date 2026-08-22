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
  /// Erzeugt einen Joystickkanal und ordnet ihm anhand von 'type' die in
  /// 'JoystickConfig' hinterlegten Pins, Kalibriergrenzen, Totzone und Richtung zu.
  /// @param type Kanaltyp: THRUST, RUDDER, ELEVATOR oder AILERON.
  explicit Joystick(const JoystickType type);

  /// Initialisiert die Eingänge und bestimmt bei zentrierenden Joysticks die
  /// aktuelle Mittelstellung aus zehn ADC-Messungen. Bei THRUST wird die
  /// konfigurierte untere Potigrenze als Nullstellung verwendet.
  /// Muss einmal in 'setup()' aufgerufen werden, bevor 'update()' verwendet wird.
  void setup();

  /// Liest Joystick und Trimmer ein, begrenzt und normalisiert den Joystickwert
  /// und wendet implizt Richtung, Expo- oder XY-Kurve sowie Dual Rate an.
  /// Diese Funktion muss aufgerufen werden, wenn die aktuelle Joystick-Position 
  /// ausgewertet werden soll.
  /// @return Bearbeiteter Joystickwert im Bereich -controlLimit..+controlLimit
  ///         (aktuell -10000..+10000). THRUST liefert ohne Richtungsumkehr 0..+controlLimit.
  int16_t update();

  /// Liefert den zuletzt von 'update()' berechneten Endwert nach allen Kennlinien.
  /// Wertebereich: -controlLimit..+controlLimit (aktuell -10000..+10000).
  /// Der Wert entspricht dem letzten Wert nach Aufruf von 'update()', somit muss der Wert
  /// nicht außerhalb von Joystick gepuffert werden.
  /// @return Letzter bearbeiteter Joystickwert im Bereich -10000..+10000.
  int16_t getValue() const { return value; };

  /// Liefert den zuletzt eingelesenen und auf die kalibrierten Potigrenzen
  /// begrenzten ADC-Wert. Der genaue Bereich hängt vom Joysticktyp ab.
  /// @return Letzter begrenzter ADC-Wert zwischen der konfigurierten minimalen
  ///         und maximalen Potigrenze des jeweiligen Kanals.
  int16_t getValueRaw() const { return valueRaw; };

  /// Liefert den auf -controlLimit..+controlLimit normalisierten Joystickwert
  /// vor Richtungsumkehr, Expo-/XY-Kurve und Dual Rate.
  /// @return Letzter normalisierter Wert im Bereich -10000..+10000.
  ///         THRUST liegt ohne Richtungsumkehr im Bereich 0..+10000.
  int16_t getValueNormalized() const { return valueNormalized;};

  /// Liefert den rohen Trimmerwert bezogen auf die ADC-Mitte.
  /// Wertebereich bei einem 10-Bit-ADC: -512..+511.
  /// @return Letzter Trimmerwert im Bereich -512..+511.
  int16_t getTrimmValueRaw() const { return trimm; };

  /// Liefert die betragsmäßige Grenze des normalisierten Joystickwertes.
  /// @return Konstante Obergrenze des normalisierten Bereichs; aktuell 10000.
  int16_t getControlLimit() const { return controlLimit; };

  /// Setzt die Totzone beiderseits der Mittelstellung in ADC-Schritten.
  /// @param centerDeadBandADC Gewünschte Totzone; intern auf 0..1000 begrenzt.
  ///        Bei THRUST wirkt sie als untere Totzone ab der Minimalstellung.
  void setDeadBand(const int16_t centerDeadBandADC);

  /// Schaltet die Vorzeichenumkehr des normalisierten Joystickwertes ein oder aus.
  /// Die Umkehrung erfolgt vor Expo-/XY-Kurve und Dual Rate.
  /// @param inverseDirection 'true' kehrt die Richtung um, 'false' verwendet
  ///        die durch die Potikalibrierung vorgegebene Richtung.
  void setDirection(const bool inverseDirection);

  /// Aktiviert die Expo-Kennlinie und deaktiviert gleichzeitig die XY-Kurve.
  /// @param expoRatePermille ADC-Einstellwert 0..1023. Nach einer Randtotzone
  ///        wird er intern auf 0..1000 begrenzt und auf den Exponenten 1.0..4.0
  ///        abgebildet. 0-12 entspricht 0 %, 1012..1023 entspricht 100 % Ausschlag.
  void setExpoCurve(const int16_t expoRatePermille);

  /// Aktiviert eine getrennte Begrenzung des negativen und positiven Bereichs.
  /// @param lowerRatePermille Einstellwert für negative Ausgangswerte, erwartet
  ///        0..1023; intern nach Randtotzone auf 0..1000 begrenzt.
  /// @param upperRatePermille Einstellwert für positive Ausgangswerte, erwartet
  ///        0..1023; intern nach Randtotzone auf 0..1000 begrenzt.
  /// 0-12 entspricht 0 %, 1012..1023 entspricht 100 % Ausschlag.
  void setDualRate(const int16_t lowerRatePermille, const int16_t upperRatePermille);

  /// Aktiviert eine stückweise linear interpolierte XY-Kennlinie und deaktiviert Expo.
  /// @param xCurvePercent X-Stützpunkte in Prozent; jeder Wert wird auf
  ///        -100..+100 begrenzt. Nach der Begrenzung müssen sie streng aufsteigen.
  /// @param yCurvePercent Ausgangswerte der Stützpunkte in Prozent; jeder Wert
  ///        wird auf -100..+100 begrenzt.
  /// @param numPoints Anzahl gültiger Einträge in beiden Arrays. Erlaubt sind
  ///        2..sizeXYcurve (aktuell 31); größere Werte werden auf 31 begrenzt.
  ///        Bei weniger als zwei oder ungültigen X-Punkten wird die Kurve deaktiviert.
  void setXYCurve(const int16_t xCurvePercent[], const int16_t yCurvePercent[], const uint8_t numPoints);

  /// Deaktiviert die Expo-Kennlinie. Der zuletzt gesetzte Exponent bleibt gespeichert.
  void resetExpoCurve();

  /// Deaktiviert die XY-Kennlinie. Stützpunkte und Punktzahl bleiben gespeichert.
  void resetXYCurve();

  /// Deaktiviert Dual Rate. Die zuletzt gesetzten Grenzwerte bleiben gespeichert.
  void resetDualRate();
  
private:
  static constexpr int16_t controlLimit = 10000; // Obergrenze für den normalisierten Joystickwert
  static constexpr int16_t inputRange   = 1000;  // Obergrenze für den ADC-Wert des Joysticks (10 Bit ADC), inklusive Totzone an den Rändern (0..1000, 0..100.0%)
  static constexpr uint8_t sizeXYcurve  = 31;    // Maximale Anzahl an Punkten für die alternative Kennlinie des Joysticks (x-y-Kurve)

  JoystickType joystickType;

  /// Wendet die aktivierte Expo-Kennlinie auf 'value' an.
  /// Der Betrag wird auf 0.0..1.0 normiert, mit 'expoPower' (1.0..4.0)
  /// potenziert und anschließend wieder auf 0..controlLimit skaliert.
  /// Bei deaktiviertem Expo oder einem Exponenten <= 1.0 bleibt 'value' unverändert.
  /// Kein Rückgabewert: Das Ergebnis wird direkt in 'value' gespeichert.
  void applyExpoCurve();

  /// Skaliert 'value' bei aktiviertem Dual Rate getrennt nach Vorzeichen.
  /// Negative Werte verwenden 'lowerRateLimit', positive Werte 'upperRateLimit'.
  /// Die Faktoren 0..inputRange entsprechen einer Skalierung von 0..100 %.
  /// Eine int32_t-Zwischenrechnung schützt die Multiplikation vor Überlauf.
  /// Kein Rückgabewert: Das Ergebnis wird direkt in 'value' gespeichert.
  void applyDualRate();

  /// Wendet bei aktivierter und gültiger XY-Kurve eine stückweise lineare
  /// Interpolation auf 'value' an. Werte außerhalb des definierten X-Bereichs
  /// werden auf den ersten beziehungsweise letzten Y-Stützpunkt begrenzt.
  /// Voraussetzung sind mindestens zwei streng aufsteigende X-Stützpunkte.
  /// Kein Rückgabewert: Das Ergebnis wird direkt in 'value' gespeichert.
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

