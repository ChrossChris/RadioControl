#include <util/atomic.h>
#include "Joystick.h"
#include "Potentiometer.h"
#include "Switch.h"
#include "Servo.h"
#include "Mixer.h"


Joystick  thrust   (JoystickType::THRUST);
Joystick  rudder   (JoystickType::RUDDER);
Joystick  elevator (JoystickType::ELEVATOR);
Joystick  aileron  (JoystickType::AILERON);
Joystick* joysticks[] {&thrust, &rudder, &elevator, &aileron};
constexpr uint8_t joystickCount = sizeof(joysticks) / sizeof(joysticks[0]);;

Potentiometer  potiMain(PotentiometerPorts::MAIN, PotentiometerType::CONTINUOUS);
Potentiometer  expoAileron(PotentiometerPorts::LEFT1, PotentiometerType::NORMAL);
Potentiometer  expoElevator(PotentiometerPorts::LEFT2, PotentiometerType::NORMAL);
Potentiometer  flightModusGain(PotentiometerPorts::RIGHT1, PotentiometerType::SYMMETRIC_WITH_DEADBAND);
Potentiometer  potiCenterLeft(PotentiometerPorts::CENTER_LEFT, PotentiometerType::NORMAL);
Potentiometer  potiCenterRight(PotentiometerPorts::CENTER_RIGHT, PotentiometerType::NORMAL);
Potentiometer  potiRightContinuous(PotentiometerPorts::RIGHT2_CONT, PotentiometerType::CONTINUOUS);
Potentiometer* potentiometers[] {&potiMain, &expoAileron, &expoElevator, &flightModusGain, &potiCenterLeft, &potiCenterRight, &potiRightContinuous};
constexpr uint8_t potentiometerCount = sizeof(potentiometers) / sizeof(potentiometers[0]);

Switch  flightModusSelector(SwitchPorts::RIGHT2_3PST_POS1, SwitchPorts::RIGHT2_3PST_POS2);
Switch  switchLeft1(SwitchPorts::LEFT1);
Switch  switchLeft2(SwitchPorts::LEFT2);
Switch  switchRight(SwitchPorts::RIGHT1);
Switch  switchCenterLeft(SwitchPorts::CENTER_LEFT);
Switch  switchCenterRight(SwitchPorts::CENTER_RIGHT);
Switch  switchMain1(SwitchPorts::MAIN1);
Switch  switchMain2_3pos(SwitchPorts::MAIN2_3PST_POS1, SwitchPorts::MAIN2_3PST_POS2);
Switch  switchMain3(SwitchPorts::MAIN3);
Switch  switchMain4_3pos(SwitchPorts::MAIN4_3PST_POS1, SwitchPorts::MAIN4_3PST_POS2);
Switch  switch4Pos(SwitchPorts::RIGHT_ROTARY_POS1, SwitchPorts::RIGHT_ROTARY_POS2, SwitchPorts::RIGHT_ROTARY_POS3, SwitchPorts::RIGHT_ROTARY_POS4);
Switch* switches[] {&flightModusSelector, &switchLeft1, &switchLeft2, &switchRight, &switchCenterLeft, &switchCenterRight, &switchMain1, &switchMain2_3pos, &switchMain3, &switchMain4_3pos, &switch4Pos};
constexpr uint8_t switchCount = sizeof(switches) / sizeof(switches[0]);

Servo  servoThrust(ServoConfig::PPM_MOTOR, thrust.getControlLimit(), 100);
Servo  servoRudder(ServoConfig::PPM_SEITE, rudder.getControlLimit(), 100);
Servo  servoElevator(ServoConfig::PPM_HOEHE, elevator.getControlLimit(), 100);
Servo  servoAileronLeft(ServoConfig::PPM_QUER_LINKS, aileron.getControlLimit(), 100);
Servo  servoAileronRight(ServoConfig::PPM_QUER_RECHTS, aileron.getControlLimit(), 100);
Servo  servoFlapLeft(ServoConfig::PPM_FLAP_LINKS, aileron.getControlLimit(), 100);
Servo  servoFlapRight(ServoConfig::PPM_FLAP_RECHTS, aileron.getControlLimit(), 100);
Servo* servos[] {&servoThrust, &servoRudder, &servoElevator, &servoAileronLeft, &servoAileronRight, &servoFlapLeft, &servoFlapRight};
constexpr uint8_t servoCount = sizeof(servos) / sizeof(servos[0]);

const MixerConfiguration defaultConfig;
Mixer rcMixerSetup[]
{
  {MixerType::THRUST,                   thrust,          servoThrust      },
  {MixerType::RUDDER,                   rudder,          servoRudder      },
  {MixerType::ELEVATOR,                 elevator,        servoElevator    },
  {MixerType::AILERON_LEFT,             aileron,         servoAileronLeft },
  {MixerType::AILERON_RIGHT,            aileron,         servoAileronRight},
  {MixerType::AILERON_FLAP_LEFT,        aileron,         servoFlapLeft    },
  {MixerType::AILERON_FLAP_RIGHT,       aileron,         servoFlapRight   },
  {MixerType::FLIGHTMODE_AILERON_LEFT,  flightModusGain, servoAileronLeft },
  {MixerType::FLIGHTMODE_AILERON_RIGHT, flightModusGain, servoAileronRight},
  {MixerType::FLIGHTMODE_FLAP_LEFT,     flightModusGain, servoFlapLeft    },
  {MixerType::FLIGHTMODE_FLAP_RIGHT,    flightModusGain, servoFlapRight   },
  {MixerType::AILERON_RUDDER,           aileron,         servoRudder      },
  {MixerType::THRUST_ELEVATOR,          thrust,          servoElevator    }
};
constexpr uint8_t rcMixerCount = sizeof(rcMixerSetup) / sizeof(rcMixerSetup[0]);


Mixer* getMixer(const MixerType type)
{
  for (uint8_t idx = 0; idx < rcMixerCount; ++idx) 
  {
    if (rcMixerSetup[idx].getType() == type)  return &rcMixerSetup[idx];
  }
  
  return nullptr;
}


void setup()
{
  for (uint8_t idx = 0; idx < joystickCount;      idx++) joysticks[idx]->setup();
  for (uint8_t idx = 0; idx < potentiometerCount; idx++) potentiometers[idx]->setup();
  for (uint8_t idx = 0; idx < switchCount;        idx++) switches[idx]->setup();
  
  Mixer* rudderMixer = getMixer(MixerType::RUDDER);
  if (rudderMixer != nullptr) rudderMixer->setGain(80, 80);

  Mixer* elevatorMixer = getMixer(MixerType::ELEVATOR);
  if (elevatorMixer != nullptr) elevatorMixer->setGain(80, 60);

  initDebugMonitor();
}

void loop()
{
  for (uint8_t idx = 0; idx < joystickCount;      ++idx) joysticks[idx]->update();
  for (uint8_t idx = 0; idx < potentiometerCount; ++idx) potentiometers[idx]->update();
  for (uint8_t idx = 0; idx < switchCount;        ++idx) switches[idx]->update();
  for (uint8_t idx = 0; idx < servoCount;         ++idx) servos[idx]->clearServo();
  for (uint8_t idx = 0; idx < rcMixerCount;       ++idx) rcMixerSetup[idx].runMixer();
}
// -----------------------------------------------------------------------------------------------------


// // -----------------------------------------------------------------------------------------------------
// // Serial
// void updateDisplay()
// { 
//   static unsigned long lastUpdateTime = 0;

//   // Aktualisierung des Displays nur alle UPDATE_PERIOD, ansonsten
//   // funktioniert die Kommunikation über die Schnittstelle nicht

//   const unsigned long now = millis();
//   const bool updateTimeExceeded = (now - lastUpdateTime) >= UPDATE_PERIOD;
//   const bool updateRequested    = (Serial1.available() > 1) && (Serial1.read() == SERIAL_START) && (Serial1.read() == SERIAL_END);

//   if (!updateRequested && !updateTimeExceeded) return;

//   // Senden des Start-Datenblocks
//   Serial1.write(SERIAL_START);
//   Serial1.write(SERIAL_START);

//   // Screen-Information -> Daraus bestimmt sich automatisch die Länge des Datenstroms
//   static GeneralInfo generalInfo;

//   generalInfo.screen  = screen;
//   generalInfo.taskCtr = taskCtr;
//   generalInfo.tone    = buttonRight;
//   Serial1.write((const byte*) &generalInfo, sizeof(GeneralInfo));

//   if (generalInfo.screen == SCREEN_JOYSTICK_VALUES)
//   {
//     static ScreenJoystickValues status_screen;
//     status_screen.thrustValue   = thrust.value;
//     status_screen.rudderValue   = rudder.value;
//     status_screen.elevatorValue = elevator.value;
//     status_screen.aileronValue  = aileron.value;
//     Serial1.write((const byte*) &status_screen, sizeof(ScreenJoystickValues));
//   }
  
//   else if (generalInfo.screen == SCREEN_JOYSTICK_TRIM)
//   {
//     static ScreenJoystickTrim status_screen;
//     status_screen.thrustTrim   = thrust.trimm;
//     status_screen.rudderTrim   = rudder.trimm;
//     status_screen.elevatorTrim = elevator.trimm;
//     status_screen.aileronTrim  = aileron.trimm;
//     Serial1.write((const byte*) &status_screen, sizeof(ScreenJoystickTrim));
//   }

//   else if (generalInfo.screen == SCREEN_POTI_CENTER)
//   {
//     static ScreenPotiCenter status_screen;
//     status_screen.potiMain        = potiMain;
//     status_screen.potiCenterLeft  = potiCenterLeft;
//     status_screen.potiCenterRight = potiCenterRight;
//     Serial1.write((const byte*) &status_screen, sizeof(ScreenPotiCenter));
//   }

//   else if (generalInfo.screen == SCREEN_POTI_LEFT_RIGHT)
//   {
//     static ScreenPotiLeftRight status_screen;
//     status_screen.potiLeft1       = potiLeft1;
//     status_screen.potiLeft2       = potiLeft2;
//     status_screen.potiRight1      = potiRight1;
//     status_screen.potiRight2Cont  = potiRight2Cont;
//     Serial1.write((const byte*) &status_screen, sizeof(ScreenPotiLeftRight));
//   }

//   else if (generalInfo.screen == SCREEN_SWITCHES)
//   {
//     uint8_t values[4] = {0};
//     uint8_t buffer = 0;

//     buffer = 0;
//     bitWrite(buffer, 0, bitRead(switchLeft[0],0));
//     bitWrite(buffer, 1, bitRead(switchLeft[1],0));
//     bitWrite(buffer, 2, bitRead(switchRight[0],0));
//     bitWrite(buffer, 3, bitRead(switchRight[1],0));
//     bitWrite(buffer, 4, bitRead(switchRight[1],1));
//     bitWrite(buffer, 5, bitRead(switchRightRotary,0));
//     bitWrite(buffer, 6, bitRead(switchRightRotary,1));
//     bitWrite(buffer, 7, bitRead(buttonRight,0));
//     values[0] = buffer;
    
//     buffer = 0;
//     bitWrite(buffer, 0, bitRead(switchCenter[0],0));
//     bitWrite(buffer, 1, bitRead(switchCenter[0],1));
//     bitWrite(buffer, 2, bitRead(switchCenter[1],0));
//     bitWrite(buffer, 3, bitRead(switchCenter[1],1));
//     bitWrite(buffer, 4, bitRead(switchCenter[2],0));
//     bitWrite(buffer, 5, bitRead(switchCenter[2],1));
//     bitWrite(buffer, 6, bitRead(switchCenter[3],0));
//     bitWrite(buffer, 7, bitRead(switchCenter[3],1));
//     values[1] = buffer;

//     buffer = 0;
//     bitWrite(buffer, 0, bitRead(switchCenter[4],0));
//     bitWrite(buffer, 1, bitRead(switchCenter[4],1));
//     bitWrite(buffer, 2, bitRead(switchCenter[5],0));
//     bitWrite(buffer, 3, bitRead(switchCenter[5],1));
//     values[2] = buffer;

//     buffer = 0;
//     bitWrite(buffer, 0, bitRead(toggleButton[0]+3,0));
//     bitWrite(buffer, 1, bitRead(toggleButton[0]+3,1));
//     bitWrite(buffer, 2, bitRead(toggleButton[0]+3,2));
//     bitWrite(buffer, 3, bitRead(toggleButton[1]+3,0));
//     bitWrite(buffer, 4, bitRead(toggleButton[1]+3,1));
//     bitWrite(buffer, 5, bitRead(toggleButton[1]+3,2));
//     values[3] = buffer;

//     Serial1.write((const byte*) values, 4);
//   }

//   // Senden des Abschluss-Datenblocks
//   Serial1.write(SERIAL_END);
//   Serial1.write(SERIAL_END);


//   lastUpdateTime = now;
// }
// // -----------------------------------------------------------------------------------------------------

