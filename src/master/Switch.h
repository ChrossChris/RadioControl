#ifndef SWITCH_H
#define SWITCH_H

#include "RcBaseInput.h"


/// Erfasst diskrete Schalterstellungen ueber digitale Arduino-Eingaenge.
///
/// Der Schaltertyp wird eindeutig durch die Anzahl der an den Konstruktor
/// uebergebenen Pins festgelegt:
/// - ein Pin: Zweistellungsschalter, Wertebereich 0..1;
/// - zwei Pins: Dreistellungsschalter mit Mittelstellung, Wertebereich 0..2;
/// - vier Pins: Vierstellungs-Drehschalter, Wertebereich 0..3;
/// - fuenf Pins: Fuenfstellungs-Drehschalter, Wertebereich 0..4.
///
/// Die Klasse verwendet bewusst keine Normierung auf CONTROL_LIMIT. getValue()
/// liefert direkt die zuletzt erkannte, bei 0 beginnende Positionsnummer.
/// setup() konfiguriert alle benoetigten Pins bewusst als INPUT. Alle
/// Schalterleitungen sind hardwareseitig ueber externe Pull-down-Widerstaende
/// abgesichert: Ein offener beziehungsweise inaktiver Kontakt ergibt LOW, ein
/// geschlossener beziehungsweise aktiver Kontakt legt HIGH am Eingang an.
///
/// update() besitzt keine zeitbasierte Entprellung. Bei einem Drehschalter ohne
/// aktiven Eingang bleibt die zuletzt gueltige Position erhalten. Sind mehrere
/// Drehschaltereingaenge gleichzeitig HIGH, hat der Pin mit der niedrigsten
/// Positionsnummer Vorrang. Vor dem ersten update() liefert getValue() den in
/// RcBaseInput definierten Initialwert 0.
class Switch : public RcBaseInput
{
public:
  /// Erzeugt einen Zweistellungsschalter mit einem digitalen Eingang.
  /// @param pin0 Eingangspin mit externem Pull-down-Widerstand; LOW ergibt
  ///             Position 0, HIGH ergibt Position 1.
  explicit Switch(uint8_t pin0);  

  /// Erzeugt einen Dreistellungsschalter mit zwei digitalen Eingaengen und
  /// externen Pull-down-Widerstaenden.
  /// @param pin0 HIGH ergibt Position 0.
  /// @param pin1 HIGH ergibt Position 2.
  /// Sind beide Eingaenge LOW, wird die Mittelstellung 1 geliefert.
  /// Durch die Mechanik und Verschaltung des verwendeten Schalters wird 
  /// ausgeschlossen, dass beide Eingaenge gleichzeitig HIGH sein können.
  explicit Switch(uint8_t pin0, uint8_t pin1);

  /// Erzeugt einen Vierstellungs-Drehschalter mit externen Pull-down-Widerstaenden.
  /// Genau einer der vier Eingaenge sollte HIGH sein; pin0..pin3 entsprechen
  /// den Positionen 0..3.
  /// @param pin0 Eingang fuer Position 0.
  /// @param pin1 Eingang fuer Position 1.
  /// @param pin2 Eingang fuer Position 2.
  /// @param pin3 Eingang fuer Position 3.
  explicit Switch(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3);

  /// Erzeugt einen Fuenfstellungs-Drehschalter mit externen Pull-down-Widerstaenden.
  /// Genau einer der fuenf Eingaenge sollte HIGH sein; pin0..pin4 entsprechen
  /// den Positionen 0..4.
  /// @param pin0 Eingang fuer Position 0.
  /// @param pin1 Eingang fuer Position 1.
  /// @param pin2 Eingang fuer Position 2.
  /// @param pin3 Eingang fuer Position 3.
  /// @param pin4 Eingang fuer Position 4.
  explicit Switch(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
  
  /// Konfiguriert alle fuer den Schaltertyp benoetigten Pins als INPUT. Die
  /// externen Pull-down-Widerstaende stellen bei inaktiven Kontakten LOW sicher;
  /// INPUT_PULLUP wird deshalb nicht verwendet.
  /// Muss einmal in Arduino-setup() vor dem ersten update() aufgerufen werden.
  void setup() override;

  /// Liest die digitalen Eingaenge und speichert die erkannte Positionsnummer
  /// im gemeinsamen Eingangswert von RcBaseInput.
  /// Kein Rueckgabewert: Die Position wird anschliessend mit getValue()
  /// abgefragt.
  void update() override;

private:
  /// Interner Schaltertyp; wird automatisch durch den verwendeten Konstruktor
  /// bestimmt und kann vom Aufrufer nicht inkonsistent gesetzt werden.
  enum class SwitchType : uint8_t
  {
    STANDARD,                     ///< Zweistellungsschalter mit einem Pin.
    STANDARD_WITH_CENTER_POSITION, ///< Dreistellungsschalter mit zwei Pins.
    ROTARY_FOUR_POSITION,          ///< Vierstellungs-Drehschalter mit vier Pins.
    ROTARY_FIVE_POSITION           ///< Fuenfstellungs-Drehschalter mit fuenf Pins.
  };

  SwitchType switchType = SwitchType::STANDARD; ///< Durch den Konstruktor festgelegter Typ.
  uint8_t    pin0       = 0;                    ///< Eingang fuer Position 0.
  uint8_t    pin1       = 0;                    ///< Eingang fuer Position 1 bzw. 2 beim Mittenschalter.
  uint8_t    pin2       = 0;                    ///< Eingang fuer Position 2 bei Drehschaltern.
  uint8_t    pin3       = 0;                    ///< Eingang fuer Position 3 bei Drehschaltern.
  uint8_t    pin4       = 0;                    ///< Eingang fuer Position 4 beim Fuenfstellungsschalter.
};

#endif
