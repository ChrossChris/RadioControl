#include <util/atomic.h>
#include "joysticks.h"



// Serielle Schnittstelle
#define BAUD_RATE                 57600
#define SERIAL_START              255
#define SERIAL_END                128
#define UPDATE_PERIOD             250  // Fallback; normale Updates werden vom Display angefordert


Joystick thrust   (JoystickType::THRUST,   50,  1000, false);
Joystick rudder   (JoystickType::RUDDER,   25,  1000, false);
Joystick elevator (JoystickType::ELEVATOR, 25,  1000, false);
Joystick aileron  (JoystickType::AILERON,  25,  1000, false);

uint16_t  taskCtr = 0;


void setup()
{
  
  thrust.setup();
  rudder.setup();
  elevator.setup();
  aileron.setup();

  initDebugMonitor();
}

void loop()
{
  taskCtr++;
  thrust.update();
  rudder.update();  
  elevator.update();
  aileron.update();

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

