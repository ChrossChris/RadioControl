#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include "RcBaseInput.h"


namespace PotentiometerPorts
{
  // Analoge ADC-Kanäle für die Potentioemter
  constexpr uint8_t MAIN         = A8;
  constexpr uint8_t CENTER_LEFT  = A9;
  constexpr uint8_t CENTER_RIGHT = A10;
  constexpr uint8_t LEFT1        = A11;
  constexpr uint8_t LEFT2        = A12;
  constexpr uint8_t RIGHT1       = A13;
  constexpr uint8_t RIGHT2_CONT  = A14;
}


/// Legt fest, wie der 10-Bit-ADC-Wert eines Potentiometers aufbereitet wird.
enum class PotentiometerType : uint8_t
{
  RAW,                     ///< 0..1023; keine Normierung und keine Richtungsumkehr.
  NORMAL,                  ///< 0..CONTROL_LIMIT; unipolar und normiert.
  SYMMETRIC,               ///< -CONTROL_LIMIT..+CONTROL_LIMIT mit fester Mitte.
  SYMMETRIC_WITH_DEADBAND, ///< Wie SYMMETRIC, mit Totzone um die Mittelstellung.
  CONTINUOUS               ///< Relative Sektorauswertung eines frei drehbaren Potentiometers.
                           ///< Wertebereich -CONTROL_LIMIT..+CONTROL_LIMIT; bei Überschreitung
                           ///< wird der gespeicherte Wert auf 0 zurückgesetzt.
};


/// Liest ein analoges Potentiometer ein und speichert den entsprechend dem
/// gewaehlten PotentiometerType aufbereiteten Wert.
///
/// setup() initialisiert den Eingangspin, update() fuehrt genau eine ADC-Messung
/// aus und getValue() liefert den gespeicherten Wert ohne erneute Messung.
/// Vor dem ersten update() liefert getValue() den initialen Wert 0.
class Potentiometer : public RcBaseInput
{
public:
  /// Erzeugt einen Potentiometereingang.
  /// @param pin Analoger Arduino-Eingang, beispielsweise A0.
  /// @param type Art der Aufbereitung und damit Wertebereich von getValue().
  ///             Standard ist NORMAL mit 0..CONTROL_LIMIT.
  /// @param inverseDirection Kehrt die Richtung in den normierten Modi um.
  ///                         Im Modus RAW wird dieser Parameter ignoriert.
  explicit Potentiometer(uint8_t           pin,
                         PotentiometerType type = PotentiometerType::NORMAL,
                         bool              inverseDirection = false);

  /// Konfiguriert den analogen Pin als Eingang.
  /// Muss einmal in Arduino-setup() vor dem ersten update() aufgerufen werden.
  void setup() override;

  /// Liest das Potentiometer einmal ueber analogRead() ein und speichert den
  /// gemaess PotentiometerType aufbereiteten Wert.
  /// Kein Rueckgabewert: Das Ergebnis wird mit getValue() abgefragt.
  void update() override;

  /// Aendert die Auswerterichtung der normierten Potentiometermodi.
  /// Der gespeicherte Wert wird nicht rueckwirkend geaendert; die neue Richtung
  /// wirkt ab dem naechsten update(). Im Modus RAW hat sie keine Wirkung.
  /// @param inverseDirection true fuer invertierte, false fuer normale Richtung.
  void setDirection(bool inverseDirection);
  
private:
  /// Groesster Messwert des verwendeten 10-Bit-ADC.
  static constexpr int16_t maxRawValue = 1023;

  /// Halbe Breite der Totzone in ADC-Schritten fuer
  /// SYMMETRIC_WITH_DEADBAND.
  static constexpr int16_t deadBand = 15;

  /// Anzahl der Sektoren für das kontinuierliche Potentiometer. 
  /// Sinnvollerweise sollten nur Werte von 2er-Potenzen und mindestens
  /// vier Sektoren verwendet werden, da sonst keine sinnvolle Aufteilung
  /// und Erkennung der Sektoren möglich ist. Erlaubte Bereiche von 4..128.
  /// Allerdings gibt es für 128 Sektoren keine Hysterese mehr.
  static constexpr uint8_t sectorCount = 16;
  static_assert(sectorCount >=   4, "At least four sectors are required");
  static_assert(sectorCount <= 128, "Maximum 128 sectors are allowed");
  static_assert((maxRawValue + 1) % sectorCount == 0, "sectorCount must divide the ADC range exactly");

  /// Breite eines Sektors in ADC-Schritten.
  static constexpr int16_t sectorWidth = (maxRawValue + 1) / sectorCount;

  /// Hysterese von ungefähr 10 % der Sektorbreite auf jeder Seite einer
  /// Sektorgrenze. Messwerte in diesem Bereich verändern den zuletzt
  /// akzeptierten Sektor nicht.
  static constexpr int16_t sectorHysteresis = sectorWidth / 10;
  
  uint8_t           pin;                          ///< Analoger Eingangspin.
  PotentiometerType potiType;                     ///< Gewaehlte Art der Werteaufbereitung.
  bool              inverseDirection = false;     ///< Aktuelle Auswerterichtung.
  int16_t           oldSector = 0;                ///< Zwischenspeichern des alten Sektors (nur bei CONTINUOUS benötigt.)


};


#endif
