#include "RcHardware.h"

#include "Potentiometer.h"
#include "Servo.h"
#include "Switch.h"

namespace RcHardware
{
namespace
{
  Joystick thrust  (JoystickType::THRUST);
  Joystick rudder  (JoystickType::RUDDER);
  Joystick elevator(JoystickType::ELEVATOR);
  Joystick aileron (JoystickType::AILERON);

  Joystick* joysticks[]
  {
    &thrust,
    &rudder, 
    &elevator, 
    &aileron
  };
  constexpr uint8_t joystickCount = sizeof(joysticks) / sizeof(joysticks[0]);

  Potentiometer potiMain           (PotentiometerPorts::MAIN,         PotentiometerType::CONTINUOUS);
  Potentiometer expoAileron        (PotentiometerPorts::LEFT1,        PotentiometerType::NORMAL);
  Potentiometer expoElevator       (PotentiometerPorts::LEFT2,        PotentiometerType::NORMAL);
  Potentiometer flightModusGain    (PotentiometerPorts::RIGHT1,       PotentiometerType::SYMMETRIC_WITH_DEADBAND);
  Potentiometer potiCenterLeft     (PotentiometerPorts::CENTER_LEFT,  PotentiometerType::NORMAL);
  Potentiometer potiCenterRight    (PotentiometerPorts::CENTER_RIGHT, PotentiometerType::NORMAL);
  Potentiometer potiRightContinuous(PotentiometerPorts::RIGHT2_CONT,  PotentiometerType::CONTINUOUS);

  Potentiometer* potentiometers[]
  {
    &potiMain,
    &expoAileron,
    &expoElevator,
    &flightModusGain,
    &potiCenterLeft,
    &potiCenterRight,
    &potiRightContinuous
  };
  constexpr uint8_t potentiometerCount = sizeof(potentiometers) / sizeof(potentiometers[0]);

  Switch flightModusSelector(SwitchPorts::RIGHT2_3PST_POS1, SwitchPorts::RIGHT2_3PST_POS2);
  Switch switchLeft1          (SwitchPorts::LEFT1);
  Switch switchLeft2          (SwitchPorts::LEFT2);
  Switch switchRight          (SwitchPorts::RIGHT1);
  Switch switchCenterLeft     (SwitchPorts::CENTER_LEFT);
  Switch switchCenterRight    (SwitchPorts::CENTER_RIGHT);
  Switch switchMain1          (SwitchPorts::MAIN1);
  Switch switchMain2_3pos     (SwitchPorts::MAIN2_3PST_POS1, SwitchPorts::MAIN2_3PST_POS2);
  Switch switchMain3          (SwitchPorts::MAIN3);
  Switch switchMain4_3pos     (SwitchPorts::MAIN4_3PST_POS1, SwitchPorts::MAIN4_3PST_POS2);
  Switch switch4Pos           (SwitchPorts::RIGHT_ROTARY_POS1,
                               SwitchPorts::RIGHT_ROTARY_POS2,
                               SwitchPorts::RIGHT_ROTARY_POS3,
                               SwitchPorts::RIGHT_ROTARY_POS4);

  Switch* switches[]
  {
    &flightModusSelector,
    &switchLeft1,
    &switchLeft2,
    &switchRight,
    &switchCenterLeft,
    &switchCenterRight,
    &switchMain1,
    &switchMain2_3pos,
    &switchMain3,
    &switchMain4_3pos,
    &switch4Pos
  };
  constexpr uint8_t switchCount = sizeof(switches) / sizeof(switches[0]);

  Servo servoThrust      (ServoConfig::PPM_MOTOR,       thrust.getControlLimit(),  100);
  Servo servoRudder      (ServoConfig::PPM_SEITE,       rudder.getControlLimit(),  100);
  Servo servoElevator    (ServoConfig::PPM_HOEHE,       elevator.getControlLimit(), 100);
  Servo servoAileronLeft (ServoConfig::PPM_QUER_LINKS,  aileron.getControlLimit(), 100);
  Servo servoAileronRight(ServoConfig::PPM_QUER_RECHTS, aileron.getControlLimit(), 100);
  Servo servoFlapLeft    (ServoConfig::PPM_FLAP_LINKS,  aileron.getControlLimit(), 100);
  Servo servoFlapRight   (ServoConfig::PPM_FLAP_RECHTS, aileron.getControlLimit(), 100);

  Servo* servos[]
  {
    &servoThrust,
    &servoRudder,
    &servoElevator,
    &servoAileronLeft,
    &servoAileronRight,
    &servoFlapLeft,
    &servoFlapRight
  };
  constexpr uint8_t servoCount = sizeof(servos) / sizeof(servos[0]);

  Mixer rcMixerSetup[]
  {
    {MixerType::THRUST,                   thrust,          servoThrust       },
    {MixerType::RUDDER,                   rudder,          servoRudder       },
    {MixerType::ELEVATOR,                 elevator,        servoElevator     },
    {MixerType::AILERON_LEFT,             aileron,         servoAileronLeft  },
    {MixerType::AILERON_RIGHT,            aileron,         servoAileronRight },
    {MixerType::AILERON_FLAP_LEFT,        aileron,         servoFlapLeft     },
    {MixerType::AILERON_FLAP_RIGHT,       aileron,         servoFlapRight    },
    {MixerType::FLIGHTMODE_AILERON_LEFT,  flightModusGain, servoAileronLeft  },
    {MixerType::FLIGHTMODE_AILERON_RIGHT, flightModusGain, servoAileronRight },
    {MixerType::FLIGHTMODE_FLAP_LEFT,     flightModusGain, servoFlapLeft     },
    {MixerType::FLIGHTMODE_FLAP_RIGHT,    flightModusGain, servoFlapRight    },
    {MixerType::AILERON_RUDDER,           aileron,         servoRudder       },
    {MixerType::THRUST_ELEVATOR,          thrust,          servoElevator     }
  };
  constexpr uint8_t mixerCount = sizeof(rcMixerSetup) / sizeof(rcMixerSetup[0]);
}


Mixer* getMixer(const MixerType type)
{
  for (uint8_t idx = 0; idx < mixerCount; ++idx)
  {
    if (rcMixerSetup[idx].getType() == type) return &rcMixerSetup[idx];
  }

  return nullptr;
}


Joystick* getJoystick(const JoystickType type)
{
  switch (type)
  {
    case JoystickType::THRUST:   return &thrust;
    case JoystickType::RUDDER:   return &rudder;
    case JoystickType::ELEVATOR: return &elevator;
    case JoystickType::AILERON:  return &aileron;
  }

  return nullptr;
}


void setup()
{
  for (uint8_t idx = 0; idx < joystickCount;      ++idx)      joysticks[idx]->setup();
  for (uint8_t idx = 0; idx < potentiometerCount; ++idx) potentiometers[idx]->setup();
  for (uint8_t idx = 0; idx < switchCount;        ++idx)       switches[idx]->setup();

  Mixer* rudderMixer = getMixer(MixerType::RUDDER);
  if (rudderMixer != nullptr) rudderMixer->setGain(80, 80);

  Mixer* elevatorMixer = getMixer(MixerType::ELEVATOR);
  if (elevatorMixer != nullptr) elevatorMixer->setGain(80, 60);
}


void updateInputs()
{
  for (uint8_t idx = 0; idx < joystickCount;      ++idx) joysticks[idx]->update();
  for (uint8_t idx = 0; idx < potentiometerCount; ++idx) potentiometers[idx]->update();
  for (uint8_t idx = 0; idx < switchCount;        ++idx) switches[idx]->update();
}


void runMixers()
{
  for (uint8_t idx = 0; idx < servoCount; ++idx) servos[idx]->clearServo();
  for (uint8_t idx = 0; idx < mixerCount; ++idx) rcMixerSetup[idx].runMixer();
}
}
