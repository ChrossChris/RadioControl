#ifndef DEFINITIONS
#define DEFINITIONS
  

// Steuerungseinstellungen
#define LIMIT_FLAPS         100   // Max. Wert 255
#define LIMIT_TRIMM         250   // Max. Wert 255
#define LIMIT_QS_MISCHER    150   // Max. Wert 255
#define LIMIT_QF_MISCHER    100   // Max. Wert 255
#define LIMIT_FH_MISCHER    100   // Max. Wert 255
#define LIMIT_MH_MISCHER    150   // Max. Wert 255

// Indizes für die einzelnen Ruder und Array-Zugriffe
#define HOEHE               0
#define SEITE               1
#define QUER                2
#define MOTOR               3
#define MIN                 0
#define MAX                 1
#define MIN_POTI            2
#define MAX_POTI            3
#define CHANNELS            4

// Serielle Schnittstelle






#ifndef MODEL_MULTIPLEX_HERON
#define MODEL_MULTIPLEX_HERON

#define MULTIPLEX_HERON                       0
#define MULTIPLEX_HERON_HOEHE_MIN            -60
#define MULTIPLEX_HERON_HOEHE_MAX             60
#define MULTIPLEX_HERON_SEITE_MIN            -100
#define MULTIPLEX_HERON_SEITE_MAX             100
#define MULTIPLEX_HERON_QUER_MIN             -50
#define MULTIPLEX_HERON_QUER_MAX              85
#define MULTIPLEX_HERON_MOTOR_MIN             0
#define MULTIPLEX_HERON_MOTOR_MAX             70
#define MULTIPLEX_HERON_FLIP_HOEHE            1
#define MULTIPLEX_HERON_FLIP_SEITE            1
#define MULTIPLEX_HERON_FLIP_QUER_LINKS      -1
#define MULTIPLEX_HERON_FLIP_QUER_RECHTS     -1
#define MULTIPLEX_HERON_FLIP_FLAP_LINKS       1
#define MULTIPLEX_HERON_FLIP_FLAP_RECHTS      1
#define MULTIPLEX_HERON_BUTTERFLY             100
#define MULTIPLEX_HERON_EXPO_QUER             512
#define MULTIPLEX_HERON_EXPO_HOEHE            512
#define MULTIPLEX_HERON_EXPO_SEITE            128
#define MULTIPLEX_HERON_QUER_SEITE_MISCHER    25
#define MULTIPLEX_HERON_QUER_FLAPS_MISCHER    50
#define MULTIPLEX_HERON_FLAP_HOEHE_MISCHER    0
#define MULTIPLEX_HERON_MOTR_HOEHE_MISCHER    6
#define MULTIPLEX_HERON_MOTR_HOEHE_TOTZONE    500

#endif


















#define SCREEN_JOYSTICK_VALUES    0
#define SCREEN_JOYSTICK_TRIM      1
#define SCREEN_POTI_CENTER        2
#define SCREEN_POTI_LEFT_RIGHT    3
#define SCREEN_SWITCHES           4

struct GeneralInfo
{
  int8_t    screen  = SCREEN_JOYSTICK_VALUES;
  uint16_t  taskCtr = 0;
  uint8_t   tone    = 0;
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