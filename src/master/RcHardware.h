#ifndef RC_HARDWARE_H
#define RC_HARDWARE_H

#include "Joystick.h"
#include "Mixer.h"

/// Verwaltet die konkrete Eingangs-, Servo- und Mixerbelegung der Fernsteuerung.
/// Die Hardwareobjekte werden statisch angelegt und bleiben waehrend der
/// gesamten Programmlaufzeit gueltig.
namespace RcHardware
{
  /// Initialisiert alle Joysticks, Potentiometer und Schalter und wendet die
  /// anfängliche Mixerkonfiguration an.
  void setup();

  /// Liest alle Eingabeelemente genau einmal ein.
  void updateInputs();

  /// Setzt die Servosummen zurueck und fuehrt anschliessend alle Mixer aus.
  void runMixers();

  /// Sucht einen Mixer anhand seiner fachlichen Aufgabe.
  /// @return Zeiger auf den Mixer oder nullptr, wenn der Typ nicht vorhanden ist.
  Mixer* getMixer(MixerType type);

  /// Sucht einen Joystick anhand seines Kanaltyps, beispielsweise fuer
  /// Diagnoseausgaben.
  /// @return Zeiger auf den Joystick oder nullptr, wenn der Typ nicht vorhanden ist.
  Joystick* getJoystick(JoystickType type);
}

#endif
