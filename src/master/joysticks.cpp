// Joysticks.cpp
#include "Joysticks.h"

Joystick::Joystick(const JoystickType type) : joystickType(type)
{
  switch (joystickType)
  {
  case JoystickType::THRUST:
    pin_joystick  = JOYSTICK_THRUST;
    pin_trimm     = TRIMM_THRUST;
    minPoti       = POTI_MIN_THRUST;
    maxPoti       = POTI_MAX_THRUST;
    thrust_config = true;
    deadBand      = DEADBAND_THRUST;
    break;

  case JoystickType::RUDDER:
    pin_joystick  = JOYSTICK_RUDDER;
    pin_trimm     = TRIMM_RUDDER;
    minPoti       = POTI_MIN_RUDDER;
    maxPoti       = POTI_MAX_RUDDER;
    thrust_config = false;
    deadBand      = DEADBAND_RUDDER;
    break;

  case JoystickType::ELEVATOR:
    pin_joystick  = JOYSTICK_ELEVATOR;
    pin_trimm     = TRIMM_ELEVATOR;
    minPoti       = POTI_MIN_ELEVATOR;
    maxPoti       = POTI_MAX_ELEVATOR;
    thrust_config = false;
    deadBand      = DEADBAND_ELEVATOR;
    break;

  case JoystickType::AILERON:
    pin_joystick  = JOYSTICK_AILERON;
    pin_trimm     = TRIMM_AILERON;
    minPoti       = POTI_MIN_AILERON;
    maxPoti       = POTI_MAX_AILERON;
    thrust_config = false;
    deadBand      = DEADBAND_AILERON;
    break;
  }
}


void Joystick::setup()
{
  pinMode(pin_joystick, INPUT);
  pinMode(pin_trimm, INPUT);

  // Nulllage ermitteln beim Einschalten der Fernsteuerung (Mittelwert aus 10 Messungen)
  if (thrust_config) centerPos = minPoti; // Thrust-Joystick hat keine Mittelstellung, daher Nulllage auf den minimalen Poti-Wert setzen
  else
  {
    centerPos = 0;
    const int amount = 10;
    for (int i = 0; i < amount; i++)
    {
      centerPos += analogRead(pin_joystick); // Obacht, dass die maximale Typgröße von int16_t nicht überschritten wird (ca. 500*10 Messungen = 5000 < 32767)
      delay(20);                             // Einschwingzeit des Sample-and-Hold-Kondensators und kurze Pause zwischen den Messungen
    }
    centerPos /= amount;
  }
}


int16_t Joystick::update()
{
  rawValue = analogRead(pin_joystick);
  trimm    = analogRead(pin_trimm) - 512; // Trimmwert auf die aktuelle Nulllage beziehen -> 0 = keine Trimmung, -512..+512 = maximale Trimmung

  // Clampen des Rohwertes auf die Poti-Grenzen, um die Wertebereiche der Joysticks zu begrenzen
  rawValue = constrain(rawValue, minPoti, maxPoti);

  // Aufteilen des Wertebereichs in zwei Hälften, um die Werte zentriert um centerPos auf -controlLimit..+controlLimit zu mappen
  if      (centerPos + deadBand >= maxPoti)  value = 0;
  else if (rawValue > centerPos + deadBand)  value = map(rawValue, centerPos + deadBand, maxPoti, 0, CONTROL_LIMIT);
  else if (rawValue > centerPos - deadBand)  value = 0;
  else if (thrust_config)                    value = 0;
  else if (centerPos - deadBand <= minPoti)  value = 0;
  else                                       value = map(rawValue, minPoti, centerPos - deadBand, -CONTROL_LIMIT, 0);

  // if (inverseDirection) value = -value; // Richtung umkehren, falls der Servo in die entgegengesetzte Richtung angesteuert werden soll

  if (useExpoCurve) applyExpoCurve(); // Nichtlineare Kennlinie anwenden, falls eingestellt
  else              applyXYCurve();   // Alternative Kennlinie über x-y-Kurve anwenden, falls eingestellt

  applyDualRate(); // Dual-Rate anwenden, falls eingestellt

  return value;
}


void Joystick::setDeadBand(const int16_t centerDeadBand)
{
  deadBand = abs(centerDeadBand);
}


void Joystick::setDirection(const bool inverseServoDirection)
{ 
  inverseDirection = inverseServoDirection; 
}


void Joystick::setDualRate(const int16_t upperRatePercent, const int16_t lowerRatePercent)
{
  this->upperRatePercent = constrain(upperRatePercent, 0, 100);
  this->lowerRatePercent = constrain(lowerRatePercent, 0, 100);
}


void Joystick::setExpoCurve(const int16_t expoRatePercent)
{
  useExpoCurve          = true; // x-y-Kurve deaktivieren, stattdessen die Exponentialfunktion verwenden (standardmäßig)
  this->expoRatePercent = constrain(expoRatePercent, 0, 100);
}


void Joystick::setXYCurve(const int16_t xCurve[], const int16_t yCurve[], const uint8_t numPoints)
{
  useExpoCurve = false; // Exponentialfunktion deaktivieren, stattdessen die x-y-Kurve verwenden

  xyCurvePoints = constrain(numPoints, 0, MAX_XY_POINTS);
  if (xyCurvePoints < 2)
  {
    useExpoCurve = true;
    return;
  }

  for (uint8_t i = 0; i < xyCurvePoints; i++)
  {
    x[i] = constrain(xCurve[i], -100, 100);
    y[i] = constrain(yCurve[i], -100, 100);

    // Die X-Werte müssen streng aufsteigend sein.
    if ((i > 0) && (x[i] <= x[i - 1]))
    {
      xyCurvePoints = 0;
      useExpoCurve  = true; // Falls die X-Werte nicht korrekt sind, wird die Exponentialfunktion als Fallback verwendet
      return;
    }
  }
}


void Joystick::applyDualRate()
{
  if (value > 0) value = (static_cast<int32_t>(value) * upperRatePercent) / 100;
  else value = (static_cast<int32_t>(value) * lowerRatePercent) / 100;
}


void Joystick::applyExpoCurve()
{
  if (!useExpoCurve) return; // Falls die x-y-Kurve verwendet wird, wird die Exponentialfunktion nicht angewendet.

  // // Falls expoRatePercent zu klein (quasi abgeschaltet), kann die Berechnung abgekürzt und der Joystickwert
  // // bleibt unverändert.
  // if (expoRatePercent < 10) return;

  const float p = map(expoRatePercent, 0, 100, 100, 400) / 100.0F;
  float x       = static_cast<float>(abs(value)) / static_cast<float>(controlLimit); // Normierung auf den Bereich 0..1

  // Berechnung der Potenzfunktion entsprechend:
  //   f: x |-> x^p  x = 0..1
  // x wird normiert entsprechend dem aktuell eingestellten maximalen Wert,
  // die Potenzialfunktion ausgeführt und anschließend entnormiert durch
  // Rückskalierung auf die eingestellten Limits.
  // (Betragsmäßig, für negative Auslenkungen egalisieren sich die Vorzeichen,
  // daher nur Unterscheidung nötig für Intervallgrenzen.)
  x = pow(x, p);

  if (value < 0) value = -(int16_t)(x * controlLimit); // Entnormierung und ggf. Vorzeichen korrigieren
  else           value =  (int16_t)(x * controlLimit); // Entnormierung
}


void Joystick::applyXYCurve()
{
  // Funktion nicht ausführen, falls die Exponentialfunktion verwendet wird oder die x-y-Kurve nicht korrekt definiert ist.
  if ((useExpoCurve) || (xyCurvePoints < 2)) return;
  

  const int32_t input = value;

  // Ersten Kurvenpunkt in den normierten Wertebereich umrechnen.
  int32_t x0 = (static_cast<int32_t>(x[0]) * controlLimit) / 100;
  int32_t y0 = (static_cast<int32_t>(y[0]) * controlLimit) / 100;
  // Letzten Kurvenpunkt in den normierten Wertebereich umrechnen.
  int32_t x1 = (static_cast<int32_t>(x[xyCurvePoints-1]) * controlLimit) / 100;
  int32_t y1 = (static_cast<int32_t>(y[xyCurvePoints-1]) * controlLimit) / 100;
  // Werte links vom definierten Bereich auf den ersten Punkt begrenzen.
  if      (input <= x0)  value = y0;
  else if (input >= x1)  value = y1;
  else
  {
    // Passendes Kurvensegment suchen.
    for (uint8_t i = 1; i < xyCurvePoints; i++)
    {
      x1 = (static_cast<int32_t>(x[i]) * controlLimit) / 100;
      y1 = (static_cast<int32_t>(y[i]) * controlLimit) / 100;
   
      if (input <= x1)
      {
        // Identische X-Punkte sind ausgeschlossen, da bereits in setXYCurve() überprüft wurde, dass die X-Werte streng aufsteigend sein müssen.
        // Lineare Interpolation:
        //
        //             input - x0
        // y = y0 + ---------------- * (y1 - y0)
        //               x1 - x0
        //
        const int32_t interpolated = y0 + ((input - x0) * (y1 - y0)) / (x1 - x0);
        value = constrain(interpolated, -controlLimit, controlLimit);
        break;
      }

      // Update der vorherigen Punkte für die nächste Iteration, damit nicht zweimal die gleiche Rechnung durchgeführt wird.
      x0 = x1;
      y0 = y1;
      
    }
  }

}
