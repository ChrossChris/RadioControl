#include "Servo.h"

  
Servo::Servo(uint8_t channel, int16_t maxControlValue, uint8_t maxTrimmLevelPercent)
: ppmChannel(channel),
  actuationValue(0)
{
  controlLimit = max(1,maxControlValue);
  maxTrimmLevelPercent = min(100, maxTrimmLevelPercent);            // Trimmung kann maximal um die Strecke ServoConfig::LEVEL verschoben werden
  trimmLimit  = (ServoConfig::LEVEL * maxTrimmLevelPercent) / 100;  // und muss daher auf 100% begrenzt werden.
}


void Servo::setInverseDirection()
{
  inverseDirection = true;
}


void Servo::setForwardDirection()
{
  inverseDirection = false;
}


void Servo::setTrimm(int16_t trimmPercent)
{
  int32_t trimm = constrain(trimmPercent, -100, 100);
  trimmValue   = trimmLimit * trimm / 100;
}


void Servo::addActuation(int16_t addValue)
{
  actuationValue += addValue;
}


void Servo::clearServo()
{
  actuationValue = 0;
}

uint8_t Servo::getPpmChannel() const
{
  return ppmChannel;
}


PpmValue Servo::getPpmSignal() const
{
  PpmValue ppm = 0;

  int16_t actuationConstrained = constrain(actuationValue, -controlLimit, controlLimit);
  if (inverseDirection) actuationConstrained = -actuationConstrained;

  if (actuationConstrained < 0) ppm = map(actuationConstrained, -controlLimit,  0,  ServoConfig::CENTER_VALUE - ServoConfig::LEVEL, ServoConfig::CENTER_VALUE + trimmValue);
  else                          ppm = map(actuationConstrained,  0,  controlLimit,  ServoConfig::CENTER_VALUE + trimmValue,         ServoConfig::CENTER_VALUE + ServoConfig::LEVEL);
  
    
  // Übernahme des gewünschten Servostellung und Absicherung der maximalen Pegelgrenzen
  // (Entsprechend Logik im Code können die Grenzen nicht überschritten werden, aber sicher ist
  // sicher, falls ggf. noch Änderungen im Code erfolgen.)
  ppm = constrain(ppm,ServoConfig::CENTER_VALUE - ServoConfig::LEVEL, ServoConfig::CENTER_VALUE + ServoConfig::LEVEL);

  return ppm;
}



//   // -----------------------------------------------------------------------------------------
// // Mischt Ruderausschläge aufeinander, bspw. Querruder auf Seitenruder oder Flaps auf Höhenruder
// // In der Realisierung verschiebt der Mixanteil die Nullage des Servos, sodass die Joystickauslenkung
// // in beide Richtungen einen unterschiedlich langen Stellweg am Servo zur Folge haben. Ansonsten würde
// // bspw. bereits der Anschlag bei halber Joystickauslenkung erreicht werden. Somit ergibt sich ein 
// // besseres und nachvollziehbareres Steuergefühl.
// // targetChannel:     Zielkanal, dem zugemischt werden soll
// // mixIn:             Eingangswert, der den Mischer steuert (in maximal Ruderausschlag CONTROL_LIMIT)
// // gain:              Verstärkungswert des Mischeranteil in Prozent: ruderkanal += gain [%] * source
// // inputDeadzone:     Totzone im Eingangskanal, erst bei Überschreiten dieses Werts wird der Mischer aktiv
// // inputMaximu:       Maximalwert des Eingangskanals (entsprechend der eingestellten Begrenzung nicht bei CONTROL_LIMIT)
// int mixer(const byte targetChannel, const int mixIn, const int gain, const int inputDeadzone, const int inputMaximum)
// {
//   int mix = 0;
//   if (abs(mixIn) > inputDeadzone)
//   {
//     const int sign = mixIn > 0 ? 1 : -1;
//     mix = (long) gain * CONTROL_LIMIT * sign*(abs(mixIn)-inputDeadzone) / (100L*(inputMaximum - inputDeadzone));  // Skalierung entsprechend Prozentanteil von gain
//     mix = constrain(mix, joystickLimits[targetChannel][MIN], joystickLimits[targetChannel][MAX]);                 // Mixanteil darf die maximalen Ruderausschläge nicht überschreiten
//   }
  
//   if  (joysticks[targetChannel] < 0)  mix = map(joysticks[targetChannel], joystickLimits[targetChannel][MIN], 0, joystickLimits[targetChannel][MIN], mix);  // Mixer verschiebt die Nulllage, somit verbleiben 100%
//   else                                mix = map(joysticks[targetChannel], 0, joystickLimits[targetChannel][MAX], mix, joystickLimits[targetChannel][MAX]);  // Auslenkung für die Joysticks in beide Richtungen (mit jeweils 
//                                                                                                                                                             // unterschiedlich lang resultierendem Stellweg am Servo)
//   return mix;
// }
