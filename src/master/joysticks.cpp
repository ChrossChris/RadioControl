// Joysticks.cpp
#include "Joysticks.h"

Joystick::Joystick(const JoystickType type, const int16_t centerDeadZone, const int16_t maxValueNormalized, const bool inverseServoDirection)
    : joystickType(type),
      deadZone(centerDeadZone),
      controlLimit(maxValueNormalized),
      inverseDirection(inverseServoDirection)
  {
    switch (joystickType)
    {
      case JoystickType::THRUST:
        pin_joystick  = JOYSTICK_THRUST;
        pin_trimm     = TRIMM_THRUST;
        minPoti       = POTI_MIN_THRUST;
        maxPoti       = POTI_MAX_THRUST;
        thrust_config = true;
        break;

      case JoystickType::RUDDER:
        pin_joystick  = JOYSTICK_RUDDER;
        pin_trimm     = TRIMM_RUDDER;
        minPoti       = POTI_MIN_RUDDER;
        maxPoti       = POTI_MAX_RUDDER;
        thrust_config = false;
        break;

      case JoystickType::ELEVATOR:
        pin_joystick  = JOYSTICK_ELEVATOR;
        pin_trimm     = TRIMM_ELEVATOR;
        minPoti       = POTI_MIN_ELEVATOR;
        maxPoti       = POTI_MAX_ELEVATOR;
        thrust_config = false;
        break;

      case JoystickType::AILERON:
        pin_joystick  = JOYSTICK_AILERON;
        pin_trimm     = TRIMM_AILERON;
        minPoti       = POTI_MIN_AILERON;
        maxPoti       = POTI_MAX_AILERON;
        thrust_config = false;
        break;
    }
  }


  void Joystick::setup()
  {
    pinMode(pin_joystick, INPUT);
    pinMode(pin_trimm,    INPUT);

    // Nulllage ermitteln beim Einschalten der Fernsteuerung (Mittelwert aus 10 Messungen)
    if (thrust_config)  centerPos = minPoti;  // Thrust-Joystick hat keine Mittelstellung, daher Nulllage auf den minimalen Poti-Wert setzen
    else
    {
      const int amount = 10;
      for (int i = 0; i < amount;   i++) 
      { 
        centerPos += analogRead(pin_joystick);   // Obacht, dass die maximale Typgröße von int16_t nicht überschritten wird (ca. 500*10 Messungen = 5000 < 32767)
        delay(20);  // Einschwingzeit des Sample-and-Hold-Kondensators und kurze Pause zwischen den Messungen
      }
      centerPos /= amount;
    }
  }


  int16_t Joystick::update()
  {
    rawValue = analogRead(pin_joystick);
    trimm    = analogRead(pin_trimm) - 512;  // Trimmwert auf die aktuelle Nulllage beziehen -> 0 = keine Trimmung, -512..+512 = maximale Trimmung

    // Clampen des Rohwertes auf die Poti-Grenzen, um die Wertebereiche der Joysticks zu begrenzen
    rawValue = constrain(rawValue, minPoti, maxPoti);
    
    // Aufteilen des Wertebereichs in zwei Hälften, um die Werte zentriert um centerPos auf -controlLimit..+controlLimit zu mappen
    if      (centerPos + deadZone > maxPoti)  value = 0;
    else if (rawValue > centerPos + deadZone) value = map(rawValue,  centerPos+deadZone, maxPoti,  0, controlLimit);
    else if (rawValue > centerPos - deadZone) value = 0;
    else if (thrust_config)                   value = 0;
    else if (centerPos - deadZone < minPoti)  value = 0;
    else                                      value = map(rawValue,  minPoti, centerPos-deadZone, -controlLimit, 0);
        
    if (inverseDirection) value = -value;  // Richtung umkehren, falls der Servo in die entgegengesetzte Richtung angesteuert werden soll
    return value;
  }
  

  void Joystick::setDeadZone(const int16_t centerDeadZone)
  {
    deadZone = centerDeadZone;
  }

  void Joystick::setDirection(const bool inverseServoDirection)
  {
    inverseDirection = inverseServoDirection;
}
