// Joysticks.cpp
#include "Joysticks.h"

Joystick::Joystick(const JoystickType type) : joystickType(type)
{
  switch (joystickType)
  {
  case JoystickType::THRUST:
    pin_joystick     = JoystickConfig::THRUST;
    pin_trimm        = JoystickConfig::TRIMM_THRUST;
    minPoti          = JoystickConfig::POTI_MIN_THRUST;
    maxPoti          = JoystickConfig::POTI_MAX_THRUST;
    deadBand         = JoystickConfig::DEADBAND_THRUST;
    inverseDirection = JoystickConfig::INVERSE_THRUST;
    thrust_config    = true;
    break;

  case JoystickType::RUDDER:
    pin_joystick     = JoystickConfig::RUDDER;
    pin_trimm        = JoystickConfig::TRIMM_RUDDER;
    minPoti          = JoystickConfig::POTI_MIN_RUDDER;
    maxPoti          = JoystickConfig::POTI_MAX_RUDDER;
    deadBand         = JoystickConfig::DEADBAND_RUDDER;
    inverseDirection = JoystickConfig::INVERSE_RUDDER;
    thrust_config    = false;
    break;

  case JoystickType::ELEVATOR:
    pin_joystick     = JoystickConfig::ELEVATOR;
    pin_trimm        = JoystickConfig::TRIMM_ELEVATOR;
    minPoti          = JoystickConfig::POTI_MIN_ELEVATOR;
    maxPoti          = JoystickConfig::POTI_MAX_ELEVATOR;
    deadBand         = JoystickConfig::DEADBAND_ELEVATOR;
    inverseDirection = JoystickConfig::INVERSE_ELEVATOR;
    thrust_config    = false;
    break;

  case JoystickType::AILERON:
    pin_joystick     = JoystickConfig::AILERON;
    pin_trimm        = JoystickConfig::TRIMM_AILERON;
    minPoti          = JoystickConfig::POTI_MIN_AILERON;
    maxPoti          = JoystickConfig::POTI_MAX_AILERON;
    deadBand         = JoystickConfig::DEADBAND_AILERON;
    inverseDirection = JoystickConfig::INVERSE_AILERON;
    thrust_config    = false;
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
  valueRaw = analogRead(pin_joystick);
  trimm    = analogRead(pin_trimm) - 512; // Trimmwert auf die aktuelle Nulllage beziehen -> 0 = keine Trimmung, -512..+511 = maximale Trimmung

  // Clampen des Rohwertes auf die Poti-Grenzen, um die Wertebereiche der Joysticks zu begrenzen
  valueRaw = constrain(valueRaw, minPoti, maxPoti);

  // Aufteilen des Wertebereichs in zwei Hälften, um die Werte zentriert um centerPos auf -controlLimit..+controlLimit zu mappen
  if      (centerPos + deadBand >= maxPoti)  valueNormalized = 0;
  else if (valueRaw > centerPos + deadBand)  valueNormalized = map(valueRaw, centerPos + deadBand, maxPoti, 0, controlLimit);
  else if (valueRaw > centerPos - deadBand)  valueNormalized = 0;
  else if (thrust_config)                    valueNormalized = 0;
  else if (centerPos - deadBand <= minPoti)  valueNormalized = 0;
  else                                       valueNormalized = map(valueRaw, minPoti, centerPos - deadBand, -controlLimit, 0);

  if (inverseDirection) value = -valueNormalized; // Richtung umkehren, falls der Servo in die entgegengesetzte Richtung angesteuert werden soll
  else                  value =  valueNormalized;

  applyExpoCurve(); // Nichtlineare Kennlinie anwenden, falls eingestellt
  applyXYCurve();   // Alternative Kennlinie über x-y-Kurve anwenden, falls eingestellt
  applyDualRate();  // Dual-Rate anwenden, falls eingestellt

  return value;
}


void Joystick::setDeadBand(const int16_t centerDeadBand)
{
  deadBand = constrain(centerDeadBand, 0, inputRange);
}


void Joystick::setDirection(const bool inverseServoDirection)
{ 
  inverseDirection = inverseServoDirection; 
}


// -------------------------------------------------------------------------------------------------------------------
void Joystick::setDualRate(const int16_t lowerRate, const int16_t upperRate)
{
  // Verschiebung und Clampen auf 0..1000 (0..100.0%), dadurch ergibt sich eine kleine Totzone an den Rändern und
  // ein verständlicher Skalieungsbereich.
  useDualRate    = true;
  upperRateLimit = constrain(upperRate-12, 0, inputRange);
  lowerRateLimit = constrain(lowerRate-12, 0, inputRange);
}
// -------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------
void Joystick::resetDualRate()
{
  useDualRate = false;
}
// -------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------
void Joystick::applyDualRate()
{
  if (useDualRate)
  {
    if (value > 0) value = (static_cast<int32_t>(value) * upperRateLimit) / inputRange;
    else           value = (static_cast<int32_t>(value) * lowerRateLimit) / inputRange;
  }
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Joystick::setExpoCurve(const int16_t expoRate)
{
  useExpoCurve = true;  
  useXYCurve   = false; 

  // Berechnung der Potenzfunktion entsprechend:
  //   f: x |-> x^p  x = 0..1, p = 1.0..4.0
  // x wird normiert entsprechend dem aktuell eingestellten maximalen Wert,
  // die Potenzialfunktion ausgeführt und anschließend entnormiert durch
  // Rückskalierung auf die eingestellten Limits.
  // (Betragsmäßig, für negative Auslenkungen egalisieren sich die Vorzeichen,
  // daher nur Unterscheidung nötig für Intervallgrenzen.)

  // ExpoRate wird direkt aus dem ADC im Wert von 0..1023 übernommen, direkt auf dieser Basis wird weiter gerechnet.
  // Verschiebung und Clampen auf 0..1000 (0..100.0%), dadurch ergibt sich eine kleine Totzone an den Rändern und
  // ein verständlicher Skalieungsbereich.
  int16_t expoPowerRaw = constrain(expoRate-12, 0, inputRange); // 0..1000, 0..100.0% (kleine Totzone an den Rändern)
  expoPower = map(expoPowerRaw, 0, inputRange, 100, 400) / 100.0F; // Potenzwert für die Exponentialfunktion berechnen (1.0..4.0) 
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Joystick::resetExpoCurve()
{
  useExpoCurve = false;
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Joystick::applyExpoCurve()
{
  // Ein Exponent von 1.0F wirkt sich nicht auf den Wert aus.
  if (useExpoCurve && (expoPower > 1.0F))
  {
    // ??? Brauchen wir eine untere Grenze für expoPower, bei der die Berechnung abgekürzt wird? 
    // ??? -> expoPower = 1.0F ändert nichts am Wert und bis zu welchem Wet von expoPower ergibt sich eine Relevanz.
    // ??? Dann kann die Berechnung ggf. hier abgekürzt werden, um Rechenzeit zu sparen.  
    float x = static_cast<float>(abs(value)) / static_cast<float>(controlLimit); // Normierung auf den Bereich 0..1

    // Berechnung der Potenzfunktion entsprechend:
    //   f: x |-> x^p  x = 0..1
    // x wird normiert entsprechend dem aktuell eingestellten maximalen Wert, die Potenzialfunktion
    // ausgeführt undanschließend entnormiert durch  a
    // Rückskalierung auf die eingestellten Limits.
    // (Betragsmäßig, für negative Auslenkungen egalisieren sich die Vorzeichen,
    // daher nur Unterscheidung nötig für Intervallgrenzen.)
    x = pow(x, expoPower);

    if (value < 0) value = -(int16_t)(x * controlLimit); // Entnormierung und ggf. Vorzeichen korrigieren
    else           value =  (int16_t)(x * controlLimit); // Entnormierung
  }
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Joystick::setXYCurve(const int16_t xCurve[], const int16_t yCurve[], const uint8_t numPoints)
{
  useExpoCurve = false;
  useXYCurve   = true;

  xyCurvePoints = min(numPoints, sizeXYcurve);
  if ((xyCurvePoints < 2) || (xCurve == nullptr) || (yCurve == nullptr))
  {
    // Falls die X-Werte nicht korrekt sind, wird die xy-Curve deaktiviert
    xyCurvePoints = 0;
    useXYCurve    = false;
    return;
  }

  for (uint8_t i = 0; i < xyCurvePoints; i++)
  {
    this->xCurve[i] = constrain(xCurve[i], -100, 100);
    this->yCurve[i] = constrain(yCurve[i], -100, 100);

    // Die X-Werte müssen streng aufsteigend sein.
    if ((i > 0) && (this->xCurve[i] <= this->xCurve[i - 1]))
    {
      // Falls die X-Werte nicht korrekt sind, wird die xy-Curve deaktiviert
      xyCurvePoints = 0;
      useXYCurve    = false; 
      return;
    }
  }
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Joystick::resetXYCurve()
{
  useXYCurve = false;
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Joystick::applyXYCurve()
{
  // Funktion nicht ausführen, falls die Exponentialfunktion verwendet wird oder die x-y-Kurve nicht korrekt definiert ist.
  if ((useXYCurve) && (xyCurvePoints > 1))
  {
    const int32_t input = value;

    // Ersten Kurvenpunkt in den normierten Wertebereich umrechnen.
    int32_t x0 = (static_cast<int32_t>(xCurve[0]) * controlLimit) / 100;
    int32_t y0 = (static_cast<int32_t>(yCurve[0]) * controlLimit) / 100;
    // Letzten Kurvenpunkt in den normierten Wertebereich umrechnen.
    int32_t x1 = (static_cast<int32_t>(xCurve[xyCurvePoints-1]) * controlLimit) / 100;
    int32_t y1 = (static_cast<int32_t>(yCurve[xyCurvePoints-1]) * controlLimit) / 100;
    // Werte links vom definierten Bereich auf den ersten Punkt begrenzen.
    if      (input <= x0)  value = y0;
    else if (input >= x1)  value = y1;
    else
    {
      // Passendes Kurvensegment suchen.
      for (uint8_t i = 1; i < xyCurvePoints; i++)
      {
        x1 = (static_cast<int32_t>(xCurve[i]) * controlLimit) / 100;
        y1 = (static_cast<int32_t>(yCurve[i]) * controlLimit) / 100;
    
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
}
// -------------------------------------------------------------------------------------------------------------------
