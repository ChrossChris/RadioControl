#include "Potentiometer.h"

// -------------------------------------------------------------------------------------------------------------------
Potentiometer::Potentiometer(const uint8_t           pin,
                             const PotentiometerType type,
                             const bool              inverseDirection)
  : pin(pin),
    potiType(type),
    inverseDirection(inverseDirection)
{
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Potentiometer::setup() 
{ 
  pinMode(pin, INPUT); 
  if (potiType == PotentiometerType::CONTINUOUS) oldSector = analogRead(pin) / sectorWidth;  
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Potentiometer::setDirection(const bool inverseDirection)
{
  this->inverseDirection = inverseDirection;
}
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Potentiometer::update()
{
  const int16_t rawValue = analogRead(pin);

  switch (potiType)
  {
    case PotentiometerType::RAW:
    {
      value = rawValue;
      break;
    }
    case PotentiometerType::NORMAL:
    {
      if (inverseDirection) value = map(maxRawValue-rawValue, 0, maxRawValue, 0, CONTROL_LIMIT);
      else                  value = map(            rawValue, 0, maxRawValue, 0, CONTROL_LIMIT);
      break;
    }
    case PotentiometerType::SYMMETRIC:
    {
      const int16_t centerValue = (maxRawValue+1) / 2;
      if      (rawValue > centerValue) value = map(rawValue, centerValue, maxRawValue,              0, CONTROL_LIMIT);
      else if (rawValue < centerValue) value = map(rawValue,           0, centerValue, -CONTROL_LIMIT,             0); 
      else                             value = 0;
      if (inverseDirection) value = -value;
      break;
    }
    case PotentiometerType::SYMMETRIC_WITH_DEADBAND:
    {
      const int16_t upperLimit  = ((maxRawValue+1) / 2) + deadBand;
      const int16_t lowerLimit  = ((maxRawValue+1) / 2) - deadBand;
      if      (rawValue > upperLimit) value = map(rawValue, upperLimit, maxRawValue, 0, CONTROL_LIMIT);
      else if (rawValue < lowerLimit) value = map(rawValue, 0, lowerLimit, -CONTROL_LIMIT, 0); 
      else                            value = 0;
      if (inverseDirection) value = -value;
      break;
    }
    case PotentiometerType::CONTINUOUS:
    {
      const int16_t positionInSector  = rawValue % sectorWidth;
      if (  (positionInSector  <   sectorHysteresis) 
         || (positionInSector  >= (sectorWidth - sectorHysteresis)) ) break;

      const int16_t sector = rawValue / sectorWidth;
      int16_t delta = sector - oldSector;

      // Korrigiert den Umlaufpunkt zwischen dem letzten und ersten Sektor.
      // Durch halfSectorCount wird die richtige Zählweise geprüft, falls
      // bei der Abtastung ein oder mehrere Sektoren übersprungen wurde.
      const int16_t halfSectorCount = sectorCount / 2;
      if      (delta >  halfSectorCount)  delta -= sectorCount;
      else if (delta < -halfSectorCount)  delta += sectorCount;

      if   (inverseDirection) value -= delta;
      else                    value += delta;

      oldSector = sector;

      // Wird CONTROL_LIMIT überschritten, beginnt die relative Zählung wieder
      // bei 0. Nach ungefähr CONTROL_LIMIT / sectorCount vollständigen Umdrehungen
      // in derselben Richtung wird die Grenze überschritten. Bspw. bei 16 Sektoren
      // pro Umdrehung tritt dies erst nach 10000 / 16 = 625 vollständigen Umdrehungen
      // in derselben Richtung auf. Es ist also ausreichend Puffer vorhanden.
      if ((value > CONTROL_LIMIT) || (value < -CONTROL_LIMIT)) value = 0;

      break;
    }
    default:
    {
      value = 0;
      break;
    }
  }
}
// -------------------------------------------------------------------------------------------------------------------

