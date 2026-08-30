#ifndef MIXER_H
#define MIXER_H

#include <Arduino.h>
#include "RcBaseInput.h"
#include "Servo.h"


enum class MixerType : uint8_t
{
  THRUST,
  RUDDER,
  ELEVATOR,
  AILERON_LEFT,
  AILERON_RIGHT,
  AILERON_FLAP_LEFT,
  AILERON_FLAP_RIGHT,
  FLIGHTMODE_AILERON_LEFT,
  FLIGHTMODE_AILERON_RIGHT,
  FLIGHTMODE_FLAP_LEFT,
  FLIGHTMODE_FLAP_RIGHT,
  AILERON_RUDDER,
  THRUST_ELEVATOR,
  UNDEFINED
};

/// Legt beim Duplizieren eines Mixers fest, wie die Verstaerkungen fuer die
/// positive und negative Eingangsseite uebernommen werden.
enum class MixerGainMode : uint8_t
{
  KEEP, ///< Uebernimmt beide Verstaerkungen unveraendert.
  SWAP  ///< Vertauscht gainPositive und gainNegative in der Kopie.
};


/// Vom konkreten Steuerbereich unabhaengige, prozentuale Mixerkonfiguration.
/// Offset und Totzonengrenzen werden bei der Konstruktion oder Änderung auf
/// das controlLimit der fest zugeordneten Quelle umgerechnet.
struct MixerConfiguration
{
   /// Verstärkungen und Offset in Prozent, jeweils -100 bis +100.
  int8_t gainPositive  = 100; ///< Verstaerkung positiver Werte, -100..+100 Prozent.
  int8_t gainNegative  = 100; ///< Verstaerkung negativer Werte, -100..+100 Prozent.
  int8_t offsetPercent = 0;   ///< Offset relativ zum Steuerbereich, -100..+100 Prozent.

  /// Totzonengrenzen in Prozent: unten -100 bis 0, oben 0 bis +100.
  int8_t deadBandLowerPercent = 0; ///< Untere Totzonengrenze, -100..0 Prozent.
  int8_t deadBandUpperPercent = 0; ///< Obere Totzonengrenze, 0..+100 Prozent.
};


/// Verknuepft eine kontinuierliche Steuerquelle mit einem Servo und bereitet
/// den Quellenwert gemaess MixerConfiguration fuer dieses Servo auf.
///
/// Jeder Mixer wird bei seiner Konstruktion fest mit einer Quelle und einem
/// Servo verknüpft. Beide Zuordnungen können später nicht geändert werden.
/// Nur ein aktivierter Mixer nimmt an der Berechnung teil; deactivateMixer()
/// und activateMixer() ändern ausschließlich diesen Aktivierungszustand.
///
/// runMixer() verarbeitet den zuletzt gespeicherten Quellenwert in dieser
/// Reihenfolge:
/// - Werte innerhalb der einschliesslichen Totzonengrenzen werden 0;
/// - ausserhalb der Totzone wird die vorzeichenabhaengige Verstaerkung angewandt;
/// - ein Offset verschiebt und skaliert den verbleibenden Teilbereich;
/// - das Ergebnis wird auf -controlLimit..+controlLimit begrenzt und mit
///   Servo::addActuation() zum Servoausgang addiert.
///
/// Der Mixer besitzt Quelle und Servo nicht. Beide Objekte muessen daher
/// waehrend der gesamten Verbindung gueltig bleiben.
class Mixer
{
public:
  /// Ein Mixer ohne Quelle und Servo ist nicht zulässig. Normales Kopieren
  /// wird ebenfalls verhindert, weil es unbemerkt dasselbe Servo verwenden
  /// würde. Zum gezielten Duplizieren dient der Konstruktor mit newServo.
  Mixer() = delete;
  Mixer(const Mixer&) = delete;
  Mixer& operator=(const Mixer&) = delete;

  /// Erzeugt einen Mixer aus einer zusammengefassten Konfiguration. Alle Werte
  /// werden vor der Verwendung auf ihre zulaessigen Bereiche begrenzt.
  /// @param type Unveränderliche fachliche Aufgabe des Mixers.
  /// @param source Zu lesende Steuerquelle. Sie muss den Mixer überleben.
  /// @param servo Zielservo. Es muss den Mixer überleben.
  Mixer(MixerType                  type,
        const RcBaseInput&         source,
        Servo&                     servo);

  /// Erzeugt einen Mixer aus einer zusammengefassten Konfiguration. Alle Werte
  /// werden vor der Verwendung auf ihre zulaessigen Bereiche begrenzt.
  /// @param type Unveränderliche fachliche Aufgabe des Mixers.
  /// @param source Zu lesende Steuerquelle. Sie muss den Mixer überleben.
  /// @param servo Zielservo. Es muss den Mixer überleben.
  /// @param configuration Zu uebernehmende Mixerparameter.
  Mixer(MixerType                  type,
        const RcBaseInput&         source,
        Servo&                     servo,
        const MixerConfiguration&  configuration);

  /// Deaktiviert die Berechnung. Verbindung und Konfiguration bleiben erhalten,
  /// sodass der Mixer anschließend erneut aktiviert werden kann.
  void deactivateMixer();

  /// Aktiviert den Mixer wieder. Quelle, Servo und skalierte Konfiguration
  /// bleiben auch im deaktivierten Zustand unverändert erhalten.
  void activateMixer();

  /// Übernimmt eine vollständige Konfiguration und begrenzt alle Werte auf
  /// ihre zulässigen Bereiche.
  /// @param configuration Zu übernehmende Konfiguration.
  void setConfiguration(const MixerConfiguration& configuration);

  /// Gibt die aktuell gespeicherte, bereits begrenzte Konfiguration zurück.
  /// @return Konstante Referenz auf die interne Konfiguration; sie bleibt nur
  ///         solange gueltig wie das Mixerobjekt.
  const MixerConfiguration& getConfiguration() const;

  /// Liefert die bei der Konstruktion festgelegte Aufgabe des Mixers.
  /// @return Unveränderlicher Typ des Mixers.
  MixerType getType() const;

  /// Setzt die getrennten Verstaerkungen beider Eingangsseiten.
  /// @param gainPositive Verstaerkung positiver Werte; -100..+100 Prozent.
  /// @param gainNegative Verstaerkung negativer Werte; -100..+100 Prozent.
  void setGain(int8_t gainPositive, int8_t gainNegative);

  /// Setzt den Offset relativ zum Steuerbereich.
  /// @param offsetPercent Offset; begrenzt auf -100..+100 Prozent.
  void setOffset(int8_t offsetPercent);

  /// Setzt eine asymmetrische Totzone relativ zum Steuerbereich.
  /// @param lowerLimitPercent Untere Grenze; begrenzt auf -100..0 Prozent.
  /// @param upperLimitPercent Obere Grenze; begrenzt auf 0..+100 Prozent.
  void setDeadBandLimits(const int8_t lowerLimitPercent, const int8_t upperLimitPercent);

  /// Setzt eine möglichst symmetrische Totzone mit der angegebenen Gesamtbreite.
  /// Bei ungeraden Prozentwerten liegt ein Prozentpunkt mehr auf der Oberseite.
  /// @param deadBandPercent Gesamtbreite; begrenzt auf 0..100 Prozent.
  void setDeadBand(const uint8_t deadBandPercent);

  /// Berechnet den aktuellen Mixerbeitrag und addiert ihn zum verbundenen
  /// Servo. Bei deaktiviertem Mixer erfolgt keine Aenderung.
  void runMixer();

private:
  /// Rechnet Offset und Totzonengrenzen aus Prozent in den aktuellen
  /// Steuerbereich um und rundet ganzzahlig zum naechsten Wert.
  void updateConfiguration();

  const MixerType type;                  ///< Unveränderliche fachliche Aufgabe des Mixers.
  MixerConfiguration configuration;      ///< Prozentuale und veränderbare Konfiguration.

  const RcBaseInput* source  = nullptr;  ///< Feste, nicht besitzende Zuordnung zur Steuerquelle.
  Servo* servo               = nullptr;  ///< Feste, nicht besitzende Zuordnung zum Zielservo.

  int16_t controlLimit       = 0;        ///< Betrag des Wertebereichs der Quelle.
  int16_t deadBandLowerLimit = 0;        ///< Skalierte untere Totzonengrenze.
  int16_t deadBandUpperLimit = 0;        ///< Skalierte obere Totzonengrenze.
  int16_t offset             = 0;        ///< Auf controlLimit skalierter Offset.
  bool    enabled            = false;    ///< true, wenn runMixer() einen Beitrag berechnen darf.

};

#endif
