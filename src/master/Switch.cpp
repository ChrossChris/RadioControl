#include "Switch.h"

Switch::Switch(uint8_t pin0)
  : switchType(SwitchType::STANDARD),
    pin0(pin0)    
{}

  
Switch::Switch(uint8_t pin0, uint8_t pin1)
  : switchType(SwitchType::STANDARD_WITH_CENTER_POSITION),
    pin0(pin0),
    pin1(pin1)
{}


Switch::Switch(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3)
  : switchType(SwitchType::ROTARY_FOUR_POSITION),
    pin0(pin0),
    pin1(pin1),
    pin2(pin2),
    pin3(pin3)
{}

Switch::Switch(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4)
  : switchType(SwitchType::ROTARY_FIVE_POSITION),
    pin0(pin0),
    pin1(pin1),
    pin2(pin2),
    pin3(pin3),
    pin4(pin4)
{}


void Switch::setup()
{
  switch (switchType)
  {
    case SwitchType::ROTARY_FIVE_POSITION:
      pinMode(pin4, INPUT);
      // fall through
    case SwitchType::ROTARY_FOUR_POSITION:
      pinMode(pin3, INPUT);
      pinMode(pin2, INPUT);
      // fall through
    case SwitchType::STANDARD_WITH_CENTER_POSITION:
      pinMode(pin1, INPUT);
      // fall through
    case SwitchType::STANDARD:
      pinMode(pin0, INPUT);
      break;
  }
}


void Switch::update()
{
  switch (switchType)
  {
    case SwitchType::STANDARD:
      value = digitalRead(pin0);
      break;
    case SwitchType::STANDARD_WITH_CENTER_POSITION:
      if      (digitalRead(pin0) == HIGH)  value = 0;
      else if (digitalRead(pin1) == HIGH)  value = 2;
      else                              value = 1;
      break;
    case SwitchType::ROTARY_FOUR_POSITION:
      if      (digitalRead(pin0) == HIGH)  value = 0;
      else if (digitalRead(pin1) == HIGH)  value = 1;
      else if (digitalRead(pin2) == HIGH)  value = 2;
      else if (digitalRead(pin3) == HIGH)  value = 3;
      break;
    case SwitchType::ROTARY_FIVE_POSITION:
      if      (digitalRead(pin0) == HIGH)  value = 0;
      else if (digitalRead(pin1) == HIGH)  value = 1;
      else if (digitalRead(pin2) == HIGH)  value = 2;
      else if (digitalRead(pin3) == HIGH)  value = 3;
      else if (digitalRead(pin4) == HIGH)  value = 4;
      break;
    default:
      value = 0;
  } 

}

