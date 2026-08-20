#include <Arduino.h>

// ============================================================
// Allgemeine Konstanten
// ============================================================

static const uint8_t MAX_CHANNELS = 8;
static const uint8_t MAX_MIXERS   = 8;

// Interner normierter Signalbereich
static const int16_t SIGNAL_MIN = -1000;
static const int16_t SIGNAL_MAX =  1000;

// ============================================================
// Mixer-Modus
// ============================================================

enum MixerMode : uint8_t
{
    MIX_ADD,       // Ziel += Mixerbeitrag
    MIX_REPLACE    // Ziel = Mixerbeitrag
};

// ============================================================
// Welche Hälfte der Quelle soll verwendet werden?
// ============================================================

enum MixerSide : uint8_t
{
    MIX_BOTH,
    MIX_POSITIVE_ONLY,
    MIX_NEGATIVE_ONLY
};

// ============================================================
// Mixer-Konfiguration
// ============================================================

struct MixerConfig
{
    bool enabled;

    uint8_t source;     // Quellkanal / logische Funktion
    uint8_t target;     // Zielkanal / Zielbus

    // getrennte Gewichtung für positive und negative Hälfte
    int8_t weightPositive;
    int8_t weightNegative;

    // konstanter Zuschlag in Prozent
    int8_t offset;

    // Totzone der Quelle in Prozent
    uint8_t deadband;

    MixerSide side;
    MixerMode mode;

    // optional:
    // Runtime-Korrektur, z.B. von einem Poti
    int8_t runtimeOffset;

    // Begrenzung der Runtime-Korrektur
    int8_t runtimeMin;
    int8_t runtimeMax;

    // optionaler Schalter
    // 255 = immer aktiv
    uint8_t activationSwitch;
};

// ============================================================
// Laufzeitdaten
// ============================================================

struct MixerRuntime
{
    int16_t lastContribution;
};

// ============================================================
// Hilfsfunktionen
// ============================================================

int16_t clampSignal(int32_t value)
{
    if (value < SIGNAL_MIN)
        return SIGNAL_MIN;

    if (value > SIGNAL_MAX)
        return SIGNAL_MAX;

    return (int16_t)value;
}

int8_t clampPercent(int16_t value)
{
    if (value < -100)
        return -100;

    if (value > 100)
        return 100;

    return (int8_t)value;
}

// ============================================================
// Aktivierungsprüfung
// ============================================================

bool isMixerActive(
    const MixerConfig &mix,
    const bool switchStates[],
    uint8_t switchCount)
{
    if (!mix.enabled)
        return false;

    // 255 bedeutet: immer aktiv
    if (mix.activationSwitch == 255)
        return true;

    if (mix.activationSwitch >= switchCount)
        return false;

    return switchStates[mix.activationSwitch];
}

// ============================================================
// Totzone
// ============================================================

int16_t applyDeadband(
    int16_t input,
    uint8_t deadbandPercent)
{
    int16_t threshold =
        ((int32_t)deadbandPercent * SIGNAL_MAX) / 100;

    if (abs(input) <= threshold)
        return 0;

    return input;
}

// ============================================================
// Seitenfilter
// ============================================================

int16_t applySideFilter(
    int16_t input,
    MixerSide side)
{
    switch (side)
    {
        case MIX_POSITIVE_ONLY:
            return input > 0 ? input : 0;

        case MIX_NEGATIVE_ONLY:
            return input < 0 ? input : 0;

        case MIX_BOTH:
        default:
            return input;
    }
}

// ============================================================
// Mixer-Gewichtung
// ============================================================

int16_t applyMixerWeight(
    int16_t input,
    const MixerConfig &mix)
{
    int16_t weight;

    if (input >= 0)
        weight = mix.weightPositive;
    else
        weight = mix.weightNegative;

    // Runtime-Korrektur auf Gewicht addieren
    int16_t runtime =
        constrain(
            mix.runtimeOffset,
            mix.runtimeMin,
            mix.runtimeMax
        );

    weight += runtime;

    weight = clampPercent(weight);

    int32_t result =
        ((int32_t)input * weight) / 100;

    return clampSignal(result);
}

// ============================================================
// Offset
// ============================================================

int16_t applyMixerOffset(
    int16_t value,
    int8_t offsetPercent)
{
    int32_t offset =
        ((int32_t)offsetPercent * SIGNAL_MAX) / 100;

    return clampSignal(
        (int32_t)value + offset
    );
}

// ============================================================
// Einzelnen Mixer berechnen
// ============================================================

int16_t calculateMixerContribution(
    const MixerConfig &mix,
    int16_t sourceValue)
{
    // 1. Totzone
    int16_t value =
        applyDeadband(
            sourceValue,
            mix.deadband
        );

    // 2. Positive / negative Hälfte filtern
    value =
        applySideFilter(
            value,
            mix.side
        );

    // 3. Gewichtung
    value =
        applyMixerWeight(
            value,
            mix
        );

    // 4. Offset
    value =
        applyMixerOffset(
            value,
            mix.offset
        );

    return value;
}

// ============================================================
// Mixer auf Ziel anwenden
// ============================================================

void applyMixer(
    const MixerConfig &mix,
    MixerRuntime &runtime,
    const int16_t sources[],
    int16_t targets[],
    const bool switchStates[],
    uint8_t switchCount)
{
    if (!isMixerActive(
            mix,
            switchStates,
            switchCount))
    {
        runtime.lastContribution = 0;
        return;
    }

    if (mix.source >= MAX_CHANNELS)
        return;

    if (mix.target >= MAX_CHANNELS)
        return;

    int16_t sourceValue =
        sources[mix.source];

    int16_t contribution =
        calculateMixerContribution(
            mix,
            sourceValue
        );

    runtime.lastContribution =
        contribution;

    switch (mix.mode)
    {
        case MIX_REPLACE:

            targets[mix.target] =
                contribution;

            break;

        case MIX_ADD:
        default:

            targets[mix.target] =
                clampSignal(
                    (int32_t)targets[mix.target]
                    + contribution
                );

            break;
    }
}

// ============================================================
// Alle Mixer abarbeiten
// ============================================================

void processMixers(
    MixerConfig mixers[],
    MixerRuntime mixerRuntime[],
    uint8_t mixerCount,
    const int16_t sources[],
    int16_t targets[],
    const bool switchStates[],
    uint8_t switchCount)
{
    for (uint8_t i = 0; i < mixerCount; ++i)
    {
        applyMixer(
            mixers[i],
            mixerRuntime[i],
            sources,
            targets,
            switchStates,
            switchCount
        );
    }
}