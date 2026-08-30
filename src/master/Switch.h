#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>

namespace SwitchPorts
{
  // Digitale Ports für die angeschlossenen Schalter
  constexpr uint8_t LEFT1              = 27;
  constexpr uint8_t LEFT2              = 29;
  constexpr uint8_t RIGHT1             = 23;
  constexpr uint8_t RIGHT2_3PST_POS1   = 21;
  constexpr uint8_t RIGHT2_3PST_POS2   = 20;
  constexpr uint8_t RIGHT_ROTARY_POS1  = 17;
  constexpr uint8_t RIGHT_ROTARY_POS2  = 15;
  constexpr uint8_t RIGHT_ROTARY_POS3  = 13;
  constexpr uint8_t RIGHT_ROTARY_POS4  = 11;
  constexpr uint8_t CENTER_LEFT        = 32;
  constexpr uint8_t CENTER_RIGHT       = 33;
  constexpr uint8_t MAIN1              = 34;
  constexpr uint8_t MAIN3              = 35;
  constexpr uint8_t MAIN2_3PST_POS1    = 36;
  constexpr uint8_t MAIN2_3PST_POS2    = 37;
  constexpr uint8_t MAIN4_3PST_POS1    = 38;
  constexpr uint8_t MAIN4_3PST_POS2    = 39;
}



/// Erfasst diskrete Schalterstellungen ueber digitale Arduino-Eingaenge.
///
/// Der Schaltertyp wird eindeutig durch die Anzahl der an den Konstruktor
/// uebergebenen Pins festgelegt:
/// - ein Pin: Zweistellungsschalter, Wertebereich 0..1;
/// - zwei Pins: Dreistellungsschalter mit Mittelstellung, Wertebereich 0..2;
/// - vier Pins: Vierstellungs-Drehschalter, Wertebereich 0..3;
/// - fuenf Pins: Fuenfstellungs-Drehschalter, Wertebereich 0..4.
///
/// Die Klasse verwendet keine Normierung. getState() liefert direkt die zuletzt
/// erkannte, bei 0 beginnende Positionsnummer.
/// setup() konfiguriert alle benoetigten Pins bewusst als INPUT. Alle
/// Schalterleitungen sind hardwareseitig ueber externe Pull-down-Widerstaende
/// abgesichert: Ein offener beziehungsweise inaktiver Kontakt ergibt LOW, ein
/// geschlossener beziehungsweise aktiver Kontakt legt HIGH am Eingang an.
///
/// update() besitzt keine zeitbasierte Entprellung. Bei einem Drehschalter ohne
/// aktiven Eingang bleibt die zuletzt gueltige Position erhalten. Sind mehrere
/// Drehschaltereingaenge gleichzeitig HIGH, hat der Pin mit der niedrigsten
/// Positionsnummer Vorrang. Vor dem ersten update() liefert getState() den
/// Initialwert 0.
class Switch
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
  void setup();

  /// Liest die digitalen Eingaenge und speichert die erkannte Positionsnummer
  /// in der internen Membervariablen state.
  /// Kein Rueckgabewert: Die Position wird anschliessend mit getState() abgefragt.
  void update();

  /// Liefert die beim letzten update() erkannte Schalterposition ohne erneuten
  /// Zugriff auf die digitalen Eingaenge.
  /// @return Zweistellungsschalter: 0..1; Dreistellungsschalter: 0..2;
  ///         Vierstellungs-Drehschalter: 0..3; Fuenfstellungs-Drehschalter: 0..4.
  uint8_t getState() const;

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
  uint8_t    state      = 0;                    ///< Zuletzt erkannte, bei 0 beginnende Positionsnummer.
  uint8_t    pin0       = 0;                    ///< Eingang fuer Position 0.
  uint8_t    pin1       = 0;                    ///< Eingang fuer Position 1 bzw. 2 beim Mittenschalter.
  uint8_t    pin2       = 0;                    ///< Eingang fuer Position 2 bei Drehschaltern.
  uint8_t    pin3       = 0;                    ///< Eingang fuer Position 3 bei Drehschaltern.
  uint8_t    pin4       = 0;                    ///< Eingang fuer Position 4 beim Fuenfstellungsschalter.
};

#endif
