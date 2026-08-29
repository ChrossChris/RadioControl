#ifndef MIXER_H
#define MIXER_H

#include <Arduino.h>
#include "ControlSource.h"
#include "servo.h"


// enum MixMode : uint8_t
// {
//   MIX_ADD = 0,
//   MIX_REPLACE
// };

enum class MixerConfigMode : uint8_t
{
  KEEP,
  RESET
};


enum class MixerGainMode : uint8_t
{
  KEEP,
  SWAP
};


struct MixerConfiguration
{
  /// Maximale Anzahl sichtbarer Zeichen im Mixernamen, ohne Nullterminator.
  static constexpr uint8_t maxNameLength = 20;

  char name[maxNameLength + 1] = "";

  /// Setzt den Namen, begrenzt ihn auf maxNameLength Zeichen und stellt die
  /// Nullterminierung sicher. nullptr setzt den Namen auf einen leeren String.
  void setName(const char* newName);

  /// Verstärkungen und Offset in Prozent, jeweils -100 bis +100.
  int8_t gainPositive         = 100;
  int8_t gainNegative         = 100;
  int8_t offsetPercent        = 0;

  /// Totzonengrenzen in Prozent: unten -100 bis 0, oben 0 bis +100.
  int8_t deadBandLowerPercent = 0;
  int8_t deadBandUpperPercent = 0;
};


class Mixer
{
public:
  /// Erzeugt einen unverbundenen, deaktivierten Mixer mit Standardkonfiguration.
  Mixer();

  /// Erzeugt einen konfigurierten Mixer, verbindet ihn und aktiviert ihn bei
  /// gültigen Quell- und Servo-Zeigern. Bei ungültigen Zeigern bleibt ein
  /// vollständig zurückgesetzter Mixer zurück.
  explicit Mixer(const ControlSource* source,
                       Servo*    servo,
                 const int8_t    gainPositive,
                 const int8_t    gainNegative,
                 const uint8_t   deadBandPercent = 0,
                 const int8_t    offsetPercent   = 0);

  /// Erzeugt einen Mixer aus einer zusammengefassten Konfiguration.
  Mixer(const ControlSource* source,
        Servo* servo,
        const MixerConfiguration& configuration);

  /// Dupliziert Verbindung und Konfiguration eines Mixers für ein anderes
  /// Servo. Optional werden die Gains für positive und negative Werte
  /// explizit vertauscht. Der Aktivierungszustand wird übernommen.
  Mixer(const Mixer& source,
        Servo* newServo,
        MixerGainMode gainMode = MixerGainMode::KEEP);


  /// Ein normales Kopieren würde unbemerkt denselben Servo-Zeiger verwenden,
  /// daher werden diese unterbunden.
  Mixer(const Mixer&) = delete;
  Mixer& operator=(const Mixer&) = delete;


  /// Verbindet und aktiviert den Mixer. Standardmäßig wird die bisherige
  /// Konfiguration zurückgesetzt; mit KEEP bleibt sie erhalten und wird auf das
  /// neue controlLimit skaliert. Bei ungültigen Zeigern bleibt der bisherige
  /// Zustand unverändert und die Funktion gibt false zurück.
  bool connectMixer(const ControlSource* source,
                    Servo* servo,
                    MixerConfigMode configMode = MixerConfigMode::RESET);


  /// Trennt den Mixer und setzt zusätzlich seine Konfiguration auf Standardwerte.
  /// Der Mixer wird gelöscht.
  void resetMixer();


  /// Deaktiviert die Berechnung, behält aber Verbindung und Konfiguration.
  void deactivateMixer();


  /// Aktiviert einen gültig verbundenen Mixer und aktualisiert alle skalierten
  /// Grenzwerte. Gibt false zurück, wenn Quelle oder Servo nicht verbunden sind.
  bool activateMixer();


  void setGain(int8_t gainPositive, int8_t gainNegative);
  void setOffset(int8_t offsetPercent);


  /// Übernimmt eine vollständige Konfiguration und begrenzt alle Werte auf
  /// ihre zulässigen Bereiche.
  void setConfiguration(const MixerConfiguration& configuration);


  /// Gibt die aktuell gespeicherte, bereits begrenzte Konfiguration zurück.
  const MixerConfiguration& getConfiguration() const;
  

  void setDeadBandLimits(const int8_t lowerLimitPercent, const int8_t upperLimitPercent);

  
  /// Setzt eine möglichst symmetrische Totzone mit der angegebenen Gesamtbreite.
  /// Bei ungeraden Prozentwerten liegt ein Prozentpunkt mehr auf der Oberseite.
  void setDeadBand(const uint8_t deadBandPercent);

  void setName(const char* name);
  const char* getName() const;


  void runMixer();

private:
  void resetConfiguration();
  void updateScaledConfiguration();

  const ControlSource* source = nullptr;
  Servo* servo             = nullptr;

  bool enabled = false;

  MixerConfiguration configuration;

  int16_t controlLimit       = 0;
  int16_t deadBandLowerLimit = 0;
  int16_t deadBandUpperLimit = 0;
  int16_t offset             = 0;

};

#endif
