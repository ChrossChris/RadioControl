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
void Potentiometer::setup()                                   { pinMode(pin, INPUT); }
void Potentiometer::setDirection(const bool inverseDirection) { this->inverseDirection = inverseDirection; }
// -------------------------------------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------------------------------------
void Potentiometer::update()
{
  const int16_t rawValue    = analogRead(pin);

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
      if (inverseDirection) value = map(maxRawValue-rawValue, 0, maxRawValue, 0, CONTROL_LIMIT);
      else                  value = map(            rawValue, 0, maxRawValue, 0, CONTROL_LIMIT);
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

