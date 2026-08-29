#ifndef MIXER_H
#define MIXER_H

#include <Arduino.h>
#include "RcBaseInput.h"
#include "servo.h"


// enum MixMode : uint8_t
// {
//   MIX_ADD = 0,
//   MIX_REPLACE
// };

/// Legt beim Verbinden fest, ob eine vorhandene Mixerkonfiguration erhalten
/// bleibt oder vor der Aktivierung auf Standardwerte gesetzt wird.
enum class MixerConfigMode : uint8_t
{
  KEEP, ///< Behaelt die Konfiguration und skaliert sie auf die neue Quelle.
  RESET ///< Setzt die Konfiguration vor der Aktivierung auf Standardwerte.
};


/// Legt beim Duplizieren eines Mixers fest, wie die Verstaerkungen fuer die
/// positive und negative Eingangsseite uebernommen werden.
enum class MixerGainMode : uint8_t
{
  KEEP, ///< Uebernimmt beide Verstaerkungen unveraendert.
  SWAP  ///< Vertauscht gainPositive und gainNegative in der Kopie.
};


/// Vom konkreten Steuerbereich unabhaengige, prozentuale Mixerkonfiguration.
/// Offset und Totzonengrenzen werden beim Verbinden oder Aendern auf das
/// controlLimit der angeschlossenen Quelle umgerechnet.
struct MixerConfiguration
{
  /// Maximale Anzahl sichtbarer Zeichen im Mixernamen, ohne Nullterminator.
  static constexpr uint8_t maxNameLength = 20;

  char name[maxNameLength + 1] = ""; ///< Nullterminierter Anzeigename.

  /// Setzt den Namen, begrenzt ihn auf maxNameLength Zeichen und stellt die
  /// Nullterminierung sicher. nullptr setzt den Namen auf einen leeren String.
  /// @param newName Nullterminierter Name oder nullptr.
  void setName(const char* newName);

  /// Verstärkungen und Offset in Prozent, jeweils -100 bis +100.
  int8_t gainPositive         = 100; ///< Verstaerkung positiver Werte, -100..+100 Prozent.
  int8_t gainNegative         = 100; ///< Verstaerkung negativer Werte, -100..+100 Prozent.
  int8_t offsetPercent        = 0;   ///< Offset relativ zum Steuerbereich, -100..+100 Prozent.

  /// Totzonengrenzen in Prozent: unten -100 bis 0, oben 0 bis +100.
  int8_t deadBandLowerPercent = 0; ///< Untere Totzonengrenze, -100..0 Prozent.
  int8_t deadBandUpperPercent = 0; ///< Obere Totzonengrenze, 0..+100 Prozent.
};


/// Verknuepft eine kontinuierliche Steuerquelle mit einem Servo und bereitet
/// den Quellenwert gemaess MixerConfiguration fuer dieses Servo auf.
///
/// Ein Mixer kann unverbunden erzeugt oder unmittelbar mit Quelle und Servo
/// verbunden werden. Nur ein aktivierter Mixer nimmt an der Berechnung teil.
/// deactivateMixer() behaelt Verbindung und Konfiguration; resetMixer() trennt
/// beide Objekte und stellt die Standardkonfiguration wieder her.
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
  /// Erzeugt einen unverbundenen, deaktivierten Mixer mit Standardkonfiguration.
  Mixer();

  /// Erzeugt einen konfigurierten Mixer, verbindet ihn und aktiviert ihn bei
  /// gültigen Quell- und Servo-Zeigern. Bei ungültigen Zeigern bleibt ein
  /// vollständig zurückgesetzter Mixer zurück.
  /// @param source Zu lesende Steuerquelle; darf nicht nullptr sein.
  /// @param servo Zielservo; darf nicht nullptr sein.
  /// @param gainPositive Verstaerkung positiver Werte, -100..+100 Prozent.
  /// @param gainNegative Verstaerkung negativer Werte, -100..+100 Prozent.
  /// @param deadBandPercent Gesamtbreite der Totzone, 0..100 Prozent.
  /// @param offsetPercent Offset relativ zum Steuerbereich, -100..+100 Prozent.
  explicit Mixer(const RcBaseInput* source,
                       Servo*       servo,
                 const int8_t       gainPositive,
                 const int8_t       gainNegative,
                 const uint8_t      deadBandPercent = 0,
                 const int8_t       offsetPercent   = 0);

  /// Erzeugt einen Mixer aus einer zusammengefassten Konfiguration. Alle Werte
  /// werden vor der Verwendung auf ihre zulaessigen Bereiche begrenzt.
  /// @param source Zu lesende Steuerquelle; darf nicht nullptr sein.
  /// @param servo Zielservo; darf nicht nullptr sein.
  /// @param configuration Zu uebernehmende Mixerparameter.
  Mixer(const RcBaseInput* source,
        Servo* servo,
        const MixerConfiguration& configuration);

  /// Dupliziert Verbindung und Konfiguration eines Mixers für ein anderes
  /// Servo. Optional werden die Gains für positive und negative Werte
  /// explizit vertauscht. Der Aktivierungszustand wird übernommen.
  /// @param source Zu duplizierender Mixer.
  /// @param newServo Zielservo der Kopie; darf nicht nullptr sein.
  /// @param gainMode KEEP behaelt, SWAP vertauscht beide Verstaerkungen.
  Mixer(const Mixer& source,
        Servo* newServo,
        MixerGainMode gainMode = MixerGainMode::KEEP);


  /// Ein normales Kopieren würde unbemerkt denselben Servo-Zeiger verwenden,
  /// daher werden diese unterbunden.
  Mixer(const Mixer&) = delete;
  Mixer& operator=(const Mixer&) = delete;


  /// Verbindet und aktiviert den Mixer. Standardmäßig wird die bisherige
  /// Konfiguration zurückgesetzt; mit KEEP bleibt sie erhalten und wird auf das
  /// neue controlLimit skaliert. Bei ungültigen Zeigern wird der Mixer
  /// deaktiviert; vorhandene Verbindung und Konfiguration bleiben gespeichert.
  /// @param source Neue Steuerquelle; nullptr ist ungueltig.
  /// @param servo Neues Zielservo; nullptr ist ungueltig.
  /// @param configMode RESET setzt die Konfiguration zurueck, KEEP behaelt sie.
  /// @return true bei erfolgreicher Verbindung und Aktivierung, sonst false.
  bool connectMixer(const RcBaseInput* source,
                    Servo* servo,
                    MixerConfigMode configMode = MixerConfigMode::RESET);


  /// Trennt und deaktiviert den Mixer und setzt seine Konfiguration auf
  /// Standardwerte. Das Mixerobjekt selbst bleibt bestehen und kann erneut
  /// verbunden werden.
  void resetMixer();


  /// Deaktiviert die Berechnung, behält aber Verbindung und Konfiguration. Der
  /// Mixer kann anschliessend mit activateMixer() erneut aktiviert werden.
  void deactivateMixer();


  /// Aktiviert einen gültig verbundenen Mixer und aktualisiert alle skalierten
  /// Grenzwerte. Gibt false zurück, wenn Quelle oder Servo nicht verbunden sind.
  /// @return true bei erfolgreicher Aktivierung, sonst false.
  bool activateMixer();


  /// Setzt die getrennten Verstaerkungen beider Eingangsseiten.
  /// @param gainPositive Verstaerkung positiver Werte; -100..+100 Prozent.
  /// @param gainNegative Verstaerkung negativer Werte; -100..+100 Prozent.
  void setGain(int8_t gainPositive, int8_t gainNegative);

  /// Setzt den Offset relativ zum Steuerbereich.
  /// @param offsetPercent Offset; begrenzt auf -100..+100 Prozent.
  void setOffset(int8_t offsetPercent);


  /// Übernimmt eine vollständige Konfiguration und begrenzt alle Werte auf
  /// ihre zulässigen Bereiche.
  /// @param configuration Zu uebernehmende Konfiguration. Der Name wird kopiert.
  void setConfiguration(const MixerConfiguration& configuration);


  /// Gibt die aktuell gespeicherte, bereits begrenzte Konfiguration zurück.
  /// @return Konstante Referenz auf die interne Konfiguration; sie bleibt nur
  ///         solange gueltig wie das Mixerobjekt.
  const MixerConfiguration& getConfiguration() const;
  

  /// Setzt eine asymmetrische Totzone relativ zum Steuerbereich.
  /// @param lowerLimitPercent Untere Grenze; begrenzt auf -100..0 Prozent.
  /// @param upperLimitPercent Obere Grenze; begrenzt auf 0..+100 Prozent.
  void setDeadBandLimits(const int8_t lowerLimitPercent, const int8_t upperLimitPercent);

  
  /// Setzt eine möglichst symmetrische Totzone mit der angegebenen Gesamtbreite.
  /// Bei ungeraden Prozentwerten liegt ein Prozentpunkt mehr auf der Oberseite.
  /// @param deadBandPercent Gesamtbreite; begrenzt auf 0..100 Prozent.
  void setDeadBand(const uint8_t deadBandPercent);

  /// Setzt den Anzeigenamen. Zu lange Namen werden gekuerzt; nullptr leert ihn.
  /// @param name Nullterminierter Name oder nullptr.
  void setName(const char* name);

  /// @return Zeiger auf den internen, nullterminierten Namenspuffer. Der Zeiger
  ///         darf nicht freigegeben oder zum Veraendern des Namens benutzt werden.
  const char* getName() const;


  /// Berechnet den aktuellen Mixerbeitrag und addiert ihn zum verbundenen
  /// Servo. Bei deaktiviertem Mixer erfolgt keine Aenderung.
  void runMixer();

private:
  /// Stellt die Standardkonfiguration wieder her und aktualisiert die daraus
  /// abgeleiteten skalierten Werte.
  void resetConfiguration();

  /// Rechnet Offset und Totzonengrenzen aus Prozent in den aktuellen
  /// Steuerbereich um und rundet ganzzahlig zum naechsten Wert.
  void updateScaledConfiguration();

  const RcBaseInput* source = nullptr; ///< Nicht besitzender Zeiger auf die Steuerquelle.
  Servo* servo              = nullptr; ///< Nicht besitzender Zeiger auf das Zielservo.

  bool enabled = false; ///< true, wenn runMixer() einen Beitrag berechnen darf.

  MixerConfiguration configuration; ///< Prozentuale und lesbare Konfiguration.

  int16_t controlLimit       = 0; ///< Betrag des Wertebereichs der Quelle.
  int16_t deadBandLowerLimit = 0; ///< Skalierte untere Totzonengrenze.
  int16_t deadBandUpperLimit = 0; ///< Skalierte obere Totzonengrenze.
  int16_t offset             = 0; ///< Auf controlLimit skalierter Offset.

};

#endif
