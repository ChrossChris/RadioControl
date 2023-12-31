#ifndef DEFINITIONS
#define DEFINITIONS

#define SCREEN_JOYSTICK_VALUES    0
#define SCREEN_JOYSTICK_TRIM      1
#define SCREEN_POTI_CENTER        2
#define SCREEN_POTI_LEFT_RIGHT    3
#define SCREEN_SWITCHES           4

struct GeneralInfo
{
  int16_t dummy1 = 0;
  int16_t dummy2 = 0;
  int16_t dummy3 = 0;
  
  int8_t  screen = SCREEN_JOYSTICK_VALUES;
};


struct ScreenJoystickValues
{
  int16_t thrustValue   = 0;
  int16_t rudderValue   = 0;
  int16_t elevatorValue = 0;
  int16_t aileronValue  = 0;
};

struct ScreenJoystickTrim
{
  int16_t thrustTrim   = 0;
  int16_t rudderTrim   = 0;
  int16_t elevatorTrim = 0;
  int16_t aileronTrim  = 0;
};

struct ScreenPotiCenter
{
  int16_t potiMain        = 0;
  int16_t potiCenterLeft  = 0;
  int16_t potiCenterRight = 0;
};

struct ScreenPotiLeftRight
{
  int16_t potiLeft1      = 0;
  int16_t potiLeft2      = 0;
  int16_t potiRight1     = 0;
  int16_t potiRight2Cont = 0;
};


#endif