#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include "RcBaseInput.h"

/// Legt fest, wie der 10-Bit-ADC-Wert eines Potentiometers aufbereitet wird.
enum class PotentiometerType : uint8_t
{
  RAW,                     ///< 0..1023; keine Normierung und keine Richtungsumkehr.
  NORMAL,                  ///< 0..CONTROL_LIMIT; unipolar und normiert.
  SYMMETRIC,               ///< -CONTROL_LIMIT..+CONTROL_LIMIT mit fester Mitte.
  SYMMETRIC_WITH_DEADBAND, ///< Wie SYMMETRIC, mit Totzone um die Mittelstellung.
  CONTINUOUS               ///< Derzeit identisch zu NORMAL; fuer eine Erweiterung vorgesehen.
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

  /// Liefert den Betrag des gemeinsamen normierten Steuerbereichs.
  /// Ein ausdruecklich unnormierter Eingabemodus darf davon abweichen.
  /// @return Positive Grenze des normierten Bereichs (definiert in RcBaseInput).
  int16_t getControlLimit() const { return CONTROL_LIMIT; }

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

  uint8_t           pin;                      ///< Analoger Eingangspin.
  PotentiometerType potiType;                 ///< Gewaehlte Art der Werteaufbereitung.
  bool              inverseDirection = false; ///< Aktuelle Auswerterichtung.
};


#endif
