#include <util/atomic.h>
#include "Joystick.h"
#include "Potentiometer.h"
#include "Switch.h"
#include "Servo.h"
#include "Mixer.h"


// Serielle Schnittstelle
#define BAUD_RATE                 57600
#define SERIAL_START              255
#define SERIAL_END                128
#define UPDATE_PERIOD             250  // Fallback; normale Updates werden vom Display angefordert


Joystick thrust     (JoystickType::THRUST);
Joystick rudder     (JoystickType::RUDDER);
Joystick elevator   (JoystickType::ELEVATOR);
Joystick aileron    (JoystickType::AILERON);

Potentiometer flap(A8, PotentiometerRange::UNIPOLAR);
ThreePositionSwitch flightMode(32, 33);
Potentiometer butterflyPosition;

constexpr uint32_t BUTTERFLY_DEPLOY_TIME_MS = 3000;
constexpr uint32_t BUTTERFLY_RETRACT_TIME_MS = 2000;
constexpr int16_t BUTTERFLY_POTI_DEADBAND = 50;

Servo    servoThrust(ServoConfig::PPM_MOTOR, thrust.getControlLimit(), 100);
Servo    servoRudder(ServoConfig::PPM_SEITE, rudder.getControlLimit(), 100);;
Servo    servoElevator(ServoConfig::PPM_HOEHE, elevator.getControlLimit(), 100);;
Servo    servoAileronLeft(ServoConfig::PPM_QUER_LINKS, aileron.getControlLimit(), 100);;
Servo    servoAileronRight(ServoConfig::PPM_QUER_RECHTS, aileron.getControlLimit(), 100);;
Servo    servoFlapLeft(ServoConfig::PPM_FLAP_LINKS, aileron.getControlLimit(), 100);;
Servo    servoFlapRight(ServoConfig::PPM_FLAP_RECHTS, aileron.getControlLimit(), 100);;

#define DUAL_RATE_LEFT            A11
#define DUAL_RATE_RIGHT           A12
#define EXPO                      A13
#define SWITCH_XY_CURVE           20
#define SWITCH_EXPO               21
#define SWITCH_DUALRATE_LEFT      27
#define SWITCH_DUALRATE_RIGHT     29


int16_t expo                = 0;
int16_t dualRateLeft        = 0;
int16_t dualRateRight       = 0;
int8_t  switchExpo          = 0;
int8_t  switchXYCurve       = 0;
int8_t  switchDualRateLeft  = 0;
int8_t  switchDualRateRight = 0;

Mixer rc_setup[16];
uint8_t rcMixerCount  = 0;

uint16_t  taskCtr = 0;

void setup()
{
  pinMode(SWITCH_EXPO, INPUT);
  pinMode(SWITCH_XY_CURVE, INPUT);
  pinMode(SWITCH_DUALRATE_LEFT, INPUT);
  pinMode(SWITCH_DUALRATE_RIGHT, INPUT);

  pinMode(EXPO, INPUT);
  pinMode(DUAL_RATE_LEFT, INPUT);
  pinMode(DUAL_RATE_RIGHT, INPUT);
  
  thrust.setup();
  rudder.setup();
  elevator.setup();
  aileron.setup();
  flap.setup();
  flightMode.setup();

  MixerConfiguration defaultConfig;
  defaultConfig.setName("RC Motor");
  rc_setup[0].connectMixer(&thrust,   &servoThrust);
  rc_setup[0].setConfiguration(defaultConfig);

  defaultConfig.setName("RC Seite");
  rc_setup[1].connectMixer(&rudder,   &servoRudder);
  rc_setup[1].setConfiguration(defaultConfig);

  defaultConfig.setName("RC Höhe");
  rc_setup[2].connectMixer(&elevator, &servoElevator);
  rc_setup[2].setConfiguration(defaultConfig);

  defaultConfig.setName("RC QuerLinks");
  rc_setup[3].connectMixer(&aileron,  &servoAileronLeft);
  rc_setup[3].setConfiguration(defaultConfig);

  defaultConfig.setName("RC QuerRechts");
  rc_setup[4].connectMixer(&aileron,  &servoAileronRight);
  rc_setup[4].setConfiguration(defaultConfig);

  defaultConfig.setName("RC WölbLinks");
  rc_setup[5].connectMixer(&aileron,  &servoFlapLeft);
  rc_setup[5].setConfiguration(defaultConfig);

  defaultConfig.setName("RC WölbRechts");
  rc_setup[6].connectMixer(&aileron,  &servoFlapRight);
  rc_setup[6].setConfiguration(defaultConfig);

  defaultConfig.setName("Mixer Motor->Höhe");
  rc_setup[7].connectMixer(&thrust,   &servoElevator);
  rc_setup[7].setConfiguration(defaultConfig);

  defaultConfig.setName("Mixer Quer->Seite");
  rc_setup[8].connectMixer(&aileron,  &servoRudder);
  rc_setup[8].setConfiguration(defaultConfig);

  defaultConfig.setName("Mixer Butterfly");
  rc_setup[9].connectMixer(&butterflyPosition, &servoAileronLeft);
  rc_setup[9].setConfiguration(defaultConfig);
  rc_setup[10].connectMixer(&butterflyPosition, &servoAileronRight);
  rc_setup[10].setConfiguration(defaultConfig);
  rc_setup[11].connectMixer(&butterflyPosition, &servoFlapLeft);
  rc_setup[11].setConfiguration(defaultConfig);
  rc_setup[12].connectMixer(&butterflyPosition, &servoFlapRight);
  rc_setup[12].setConfiguration(defaultConfig);

  rcMixerCount = 13;

  initDebugMonitor();
}

void loop()
{

  switchExpo    = digitalRead(SWITCH_EXPO);
  switchXYCurve = digitalRead(SWITCH_XY_CURVE);
  
  taskCtr++;
  thrust.update();
  rudder.update();  
  elevator.update();
  aileron.update();
  flap.update();
  flightMode.update();

  const bool butterflyActive =
    flightMode.getValue() == flightMode.getControlLimit();

  if (butterflyActive)
  {
    const int32_t targetDifference =
      static_cast<int32_t>(flap.getValue()) - butterflyPosition.getTarget();

    if (abs(targetDifference) >= BUTTERFLY_POTI_DEADBAND)
      butterflyPosition.setTarget(flap.getValue(), BUTTERFLY_DEPLOY_TIME_MS);
  }
  else
  {
    butterflyPosition.setTarget(0, BUTTERFLY_RETRACT_TIME_MS);
  }

  butterflyPosition.update();

  expo = analogRead(EXPO);
  if (switchExpo)
  { 
    thrust.setExpoCurve(expo);
    rudder.setExpoCurve(expo);
    elevator.setExpoCurve(expo);
    aileron.setExpoCurve(expo);
  }
  else if (switchXYCurve)
  {
    const int16_t x[] = {-100, 0,  10,  20, 40, 90, 100};
    const int16_t y[] = {   0, 0,   0,  10, 30, 70, 100};
    const uint8_t n   = 7;
    thrust.setXYCurve(x, y, n);
    rudder.setXYCurve(x, y, n);
    elevator.setXYCurve(x, y, n);
    aileron.setXYCurve(x, y, n);
  }
  else
  {
    thrust.resetExpoCurve();
    rudder.resetExpoCurve();
    elevator.resetExpoCurve();
    aileron.resetExpoCurve();

    thrust.resetXYCurve();
    rudder.resetXYCurve();
    elevator.resetXYCurve();
    aileron.resetXYCurve();
  }

  dualRateLeft        = analogRead(DUAL_RATE_LEFT);
  dualRateRight       = analogRead(DUAL_RATE_RIGHT);
  switchDualRateLeft  = digitalRead(SWITCH_DUALRATE_LEFT);
  switchDualRateRight = digitalRead(SWITCH_DUALRATE_RIGHT);
  if (switchDualRateLeft && switchDualRateRight)
  {
    thrust.setDualRate(dualRateLeft, dualRateRight);
    rudder.setDualRate(dualRateLeft, dualRateRight);
    elevator.setDualRate(dualRateLeft, dualRateRight);
    aileron.setDualRate(dualRateLeft, dualRateRight);
  }
  else if (switchDualRateLeft)
  {
    thrust.setDualRate(dualRateLeft, 1023);
    rudder.setDualRate(dualRateLeft, 1023);
    elevator.setDualRate(dualRateLeft, 1023);
    aileron.setDualRate(dualRateLeft, 1023);
  }
  else if (switchDualRateRight)
  {
    thrust.setDualRate(1023, dualRateRight);
    rudder.setDualRate(1023, dualRateRight);
    elevator.setDualRate(1023, dualRateRight);
    aileron.setDualRate(1023, dualRateRight);
  }
  else
  {
    thrust.resetDualRate();
    rudder.resetDualRate();
    elevator.resetDualRate();
    aileron.resetDualRate();
  }

  servoThrust.clearServo();
  servoRudder.clearServo();
  servoElevator.clearServo();
  servoAileronLeft.clearServo();
  servoAileronRight.clearServo();
  servoFlapLeft.clearServo();
  servoFlapRight.clearServo();

  for (uint8_t mixerIndex = 0; mixerIndex < rcMixerCount; ++mixerIndex)
  {
    rc_setup[mixerIndex].runMixer();
  }

  updateDebugMonitor();

  delay(100);

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

