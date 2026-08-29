#ifndef RC_BASE_INPUT_H
#define RC_BASE_INPUT_H

#include <Arduino.h>


/// Abstrakte Basisschnittstelle fuer alle physikalischen Bedieneingaenge der
/// Fernsteuerung, beispielsweise Joysticks, Potentiometer und Schalter.
///
/// Die Erfassung ist bewusst zweistufig aufgebaut: update() liest die Hardware
/// und speichert den aufbereiteten Wert; getValue() liefert anschliessend diesen
/// Wert ohne einen erneuten Hardwarezugriff. So koennen mehrere Verbraucher im
/// selben Programmdurchlauf mit exakt demselben Eingangswert arbeiten.
///
/// Der konkrete Wertebereich von getValue() wird von der abgeleiteten Klasse
/// beziehungsweise ihrem Betriebsmodus festgelegt. Normalisierte Eingaben
/// verwenden ueblicherweise -CONTROL_LIMIT..+CONTROL_LIMIT oder
/// 0..CONTROL_LIMIT.
class RcBaseInput
{
public:
  /// Initialisiert die zum Eingang gehoerende Hardware.
  /// Muss einmal in Arduino-setup() vor dem ersten update() aufgerufen werden.
  virtual void setup() = 0;

  /// Liest den aktuellen Hardwarezustand ein, bereitet ihn entsprechend der
  /// konkreten Eingangsklasse auf und speichert das Ergebnis intern.
  /// Kein Rueckgabewert: Das Ergebnis wird mit getValue() abgefragt.
  virtual void update() = 0;

  /// Liefert den bei update() zuletzt gespeicherten Eingangswert.
  /// Die Funktion fuehrt weder einen Hardwarezugriff noch eine Neuberechnung aus.
  /// @return Letzter gespeicherter Eingangswert; der konkrete Wertebereich ist
  ///         von der abgeleiteten Klasse beziehungsweise ihrem Modus abhaengig.
  int16_t getValue() const { return value; }

  /// Ermoeglicht die korrekte Zerstoerung abgeleiteter Objekte ueber einen
  /// Zeiger auf RcBaseInput.
  virtual ~RcBaseInput() = default;

protected:
  /// Gemeinsame positive Grenze fuer normalisierte Eingangswerte.
  static constexpr int16_t CONTROL_LIMIT = 10000;

  /// Verhindert die direkte Erzeugung der abstrakten Basisklasse und erlaubt
  /// die Konstruktion durch abgeleitete Eingangsklassen.
  RcBaseInput() = default;
  
  /// Zuletzt von update() gespeicherter Wert. Befüllung muss in den abge-
  /// leiteten Klassen erfolgen, aber alle verwenden diese Member-Variable
  /// als internen Speicher.
  int16_t value = 0; 

};

#endif
