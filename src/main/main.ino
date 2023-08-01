#include <util/atomic.h>

#define POTI_CENTER               512

// Serielle Schnittstelle
#define BAUD_RATE                 57600
#define SERIAL_START              255
#define SERIAL_END                128
#define UPDATE_PERIOD             100

// Analoge Ports
#define JOYSTICK1                 A0
#define TRIMMER1                  A1
#define JOYSTICK2                 A2
#define TRIMMER2                  A3
#define JOYSTICK3                 A4
#define TRIMMER3                  A5
#define JOYSTICK4                 A6
#define TRIMMER4                  A7

#define POTI_MAIN                 A8
#define POTI_CENTER_LEFT          A9
#define POTI_CENTER_RIGHT         A10
#define POTI_LEFT1                A11
#define POTI_LEFT2                A12
#define POTI_RIGHT1               A13
#define POTI_RIGHT2_CONT          A14

// Digitale Ports
#define SWITCH_LEFT1              27
#define SWITCH_LEFT2              29
#define BUTTON_RIGHT              25
#define SWITCH_RIGHT1             23
#define SWITCH_RIGHT2_3PST_POS1   21
#define SWITCH_RIGHT2_3PST_POS2   20
#define SWITCH_RIGHT_ROTARY_POS1  17
#define SWITCH_RIGHT_ROTARY_POS2  15
#define SWITCH_RIGHT_ROTARY_POS3  13
#define SWITCH_RIGHT_ROTARY_POS4  11

#define SWITCH_CENTER_LEFT        32
#define SWITCH_CENTER_RIGHT       33
#define SWITCH_MAIN1              34
#define SWITCH_MAIN3              35
#define SWITCH_MAIN2_3PST_POS1    36
#define SWITCH_MAIN2_3PST_POS2    37
#define SWITCH_MAIN4_3PST_POS1    38
#define SWITCH_MAIN4_3PST_POS2    39

#define TOGGLE_BUTTON1_LEFT       41
#define TOGGLE_BUTTON1_RIGHT      43
#define TOGGLE_BUTTON2_DOWN       45
#define TOGGLE_BUTTON2_UP         47

// Poti-Grenzen
#define POTI_JOYSTICK1_MAX        898
#define POTI_JOYSTICK1_MIN        114
#define POTI_JOYSTICK2_MAX        902
#define POTI_JOYSTICK2_MIN        96
#define POTI_JOYSTICK3_MAX        894
#define POTI_JOYSTICK3_MIN        123
#define POTI_JOYSTICK4_MAX        926
#define POTI_JOYSTICK4_MIN        115

#define POTI_TRIMMER1_MAX         667
#define POTI_TRIMMER1_MIN         351
#define POTI_TRIMMER2_MAX         706
#define POTI_TRIMMER2_MIN         396
#define POTI_TRIMMER3_MAX         593
#define POTI_TRIMMER3_MIN         288
#define POTI_TRIMMER4_MAX         709
#define POTI_TRIMMER4_MIN         388

// CPPM packet definitions
#define CPPM_PORT           3      // Output-Pin for the CPPM signal
#define FRAME_LENGTH        23000  // Total length of the CPPM packet in micro seconds (not sharp defined but necessary low level must be guaranteed for frame sync)
#define PULSE_LENGTH        250    // Pulse length in micro seconds for each PPM signal (CPPM channel) (200µs..500µs) (420µs measured in a Graupner FM6014)
#define PRESCALER           CS11   // Prescaler 8 -> Timer increments by 0.5µs steps
#define MICRO_SECONDS       2      // Translates micro seconds into timer counter (internal tick is 0.5µs by using prescaler 8 (bit CS11) -> factor 2 is needed 
#define PPM_CENTER_VALUE    1500   // Center position at 1.5ms
#define PPM_LEVEL           500    // May vary between 400..600
#define PPM_CHANNELS        8      // Number of encoded servos signals in one CPPM packet

// Ausgangssignale an den Sender: Aufruf in der ISR
// Container für PPM-Signale für jeden einzelnen Servo
int  ppm[PPM_CHANNELS]    = {PPM_CENTER_VALUE};
int  ppmISR[PPM_CHANNELS] = {PPM_CENTER_VALUE};


#define INPUT_PORT_FIRST          0
#define INPUT_PORT_LAST           TOGGLE_BUTTON2_DOWN


struct Joystick
{
  int value = 0;
  int trimm = 0;
};


void setup()
{
  Serial.begin(57600);
  Serial1.begin(BAUD_RATE);
  for (int8_t i_port = INPUT_PORT_FIRST; i_port <= INPUT_PORT_LAST; i_port++) pinMode(i_port, INPUT);
  initCPPM();
}


void loop()
{
  Joystick thrust, rudder, elevator, aileron;
  thrust.value    = analogRead(JOYSTICK1);
  thrust.trimm    = analogRead(TRIMMER1);
  rudder.value    = analogRead(JOYSTICK2);
  rudder.trimm    = analogRead(TRIMMER2);
  elevator.value  = analogRead(JOYSTICK3);
  elevator.trimm  = analogRead(TRIMMER3);
  aileron.value   = analogRead(JOYSTICK4);
  aileron.trimm   = analogRead(TRIMMER4);
  
  int potiMain        = analogRead(POTI_MAIN);
  int potiCenterLeft  = analogRead(POTI_CENTER_LEFT);
  int potiCenterRight = analogRead(POTI_CENTER_RIGHT);
  int potiLeft1       = analogRead(POTI_LEFT1);
  int potiLeft2       = analogRead(POTI_LEFT2);
  int potiRight1      = analogRead(POTI_RIGHT1);
  int potiRight2Cont  = analogRead(POTI_RIGHT2_CONT);


  int8_t switchLeft[2] = {0};
  switchLeft[0] = digitalRead(SWITCH_LEFT1);
  switchLeft[1] = digitalRead(SWITCH_LEFT2);

  int8_t switchRight[3] = {0};
  switchRight[0] = digitalRead(SWITCH_RIGHT1);
  if      (digitalRead(SWITCH_RIGHT2_3PST_POS1) == 1)  switchRight[1] =  0;
  else if (digitalRead(SWITCH_RIGHT2_3PST_POS2) == 1)  switchRight[1] =  2;
  else                                                 switchRight[1] =  1;

  int8_t buttonRight = digitalRead(BUTTON_RIGHT);
  static int8_t switchRightRotary = 0;
  if      (digitalRead(SWITCH_RIGHT_ROTARY_POS1) == 1)  switchRightRotary =  0;
  else if (digitalRead(SWITCH_RIGHT_ROTARY_POS2) == 1)  switchRightRotary =  1;
  else if (digitalRead(SWITCH_RIGHT_ROTARY_POS3) == 1)  switchRightRotary =  2;
  else if (digitalRead(SWITCH_RIGHT_ROTARY_POS4) == 1)  switchRightRotary =  3;
  
  int8_t switchCenter[6] = {0};
  switchCenter[0] = digitalRead(SWITCH_CENTER_LEFT);
  switchCenter[1] = digitalRead(SWITCH_CENTER_RIGHT);
  switchCenter[2] = digitalRead(SWITCH_MAIN1);
  switchCenter[4] = digitalRead(SWITCH_MAIN3);

  if      (digitalRead(SWITCH_MAIN2_3PST_POS1) == 1)  switchCenter[3] = 0;
  else if (digitalRead(SWITCH_MAIN2_3PST_POS2) == 1)  switchCenter[3] = 2;
  else                                                switchCenter[3] = 1;

  if      (digitalRead(SWITCH_MAIN4_3PST_POS1) == 1)  switchCenter[5] = 0;
  else if (digitalRead(SWITCH_MAIN4_3PST_POS2) == 1)  switchCenter[5] = 2;
  else                                                switchCenter[5] = 1;
  
  int8_t toggleButton[2] = {0};
  if      (digitalRead(TOGGLE_BUTTON1_LEFT)  == 1)    toggleButton[0] = -1;  // -> left
  else if (digitalRead(TOGGLE_BUTTON1_RIGHT) == 1)    toggleButton[0] =  1;  // -> right
  else                                                toggleButton[0] =  0;

  if      (digitalRead(TOGGLE_BUTTON2_DOWN) == 1)     toggleButton[1] = -1; // -> down
  else if (digitalRead(TOGGLE_BUTTON2_UP)   == 1)     toggleButton[1] =  1; // -> up
  else                                                toggleButton[1] =  0;


  ppm[0] = map(thrust.value,    POTI_JOYSTICK1_MIN, POTI_JOYSTICK1_MAX, PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[1] = map(rudder.value,    POTI_JOYSTICK2_MIN, POTI_JOYSTICK2_MAX, PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[2] = map(elevator.value,  POTI_JOYSTICK3_MIN, POTI_JOYSTICK3_MAX, PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[3] = map(aileron.value,   POTI_JOYSTICK4_MIN, POTI_JOYSTICK4_MAX, PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[4] = map(thrust.trimm,    POTI_TRIMMER1_MIN,  POTI_TRIMMER1_MAX,  PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[5] = map(rudder.trimm,    POTI_TRIMMER2_MIN,  POTI_TRIMMER2_MAX,  PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[6] = map(elevator.trimm,  POTI_TRIMMER3_MIN,  POTI_TRIMMER3_MAX,  PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);
  ppm[7] = map(aileron.trimm,   POTI_TRIMMER4_MIN,  POTI_TRIMMER4_MAX,  PPM_CENTER_VALUE-PPM_LEVEL, PPM_CENTER_VALUE+PPM_LEVEL);

  // Update of PPM buffer for the ISR
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    memcpy((void *)ppmISR, (void *)ppm, sizeof(int)*PPM_CHANNELS);
  }
 
  
  //  char out[700];
  //  sprintf(out,"J1[%4d|%4d] J2[%4d|%4d] J3[%4d|%4d] J4[%4d|%4d]  ||  PPM[%4d %4d %4d %4d | %4d %4d %4d %4d]  ||  PM[%4d] P_CL[%4d] P_CR[%4d]  ||  PL1[%4d] PL2[%4d] PR1[%4d] PR2c[%4d]  ||  SW_CL[%1d] SW_CR[%1d] SW1[%1d] SW2[%1d] SW3[%2d] SW4[%2d]  ||  TB1[%2d] TB2[%2d]  ||  SWL1[%1d] SWL2[%1d] || SWR1[%1d] SWR2[%1d] SWRrot[%1d] BT [%1d]\n",
  //    thrust.value, thrust.trimm, rudder.value, rudder.trimm, elevator.value, elevator.trimm, aileron.value, aileron.trimm,
  //    ppm[0], ppm[1], ppm[2], ppm[3],ppm[4], ppm[5], ppm[6], ppm[7],
  //    potiMain, potiCenterLeft, potiCenterRight, potiLeft1, potiLeft2, potiRight1, potiRight2Cont,
  //    switchCenter[0], switchCenter[1], switchCenter[2], switchCenter[3], switchCenter[4], switchCenter[5],
  //    toggleButton[0], toggleButton[1],
  //    switchLeft[0], switchLeft[1],
  //    switchRight[0], switchRight[1], switchRightRotary, buttonRight);
  //  Serial.print(out);

  updateDisplay();
}



// -----------------------------------------------------------------------------------------------------
// Set up the timing for the CPPM transmission
void initCPPM()
{
  pinMode(CPPM_PORT, OUTPUT);
  digitalWrite(CPPM_PORT, LOW);

  // Set up timer 1 for ISR
  // Further details: https://wolles-elektronikkiste.de/timer-und-pwm-teil-2-16-bit-timer1
  cli();
  TCNT1   = 0;                    // Reset counter of timer 1
  TCCR1A  = 0;                    // Reset TCCR1 byte A
  TCCR1B  = 0;                    // Reset TCCR1 byte B
  bitSet(TCCR1B, WGM12);          // CTC Mode (Clear Timer on Compare Match)
  bitSet(TCCR1B, PRESCALER);      // Timer resolution
  bitSet(TIMSK1, OCIE1A);         // Output compare interrupt enable -> ISR on timer1 compare match
  OCR1A   = 1000 * MICRO_SECONDS; // First call of corresponding ISR in 1ms
  sei();
}
// -----------------------------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------------------------
// Serial
void updateDisplay()
{ 
  // Nächste Aktualisierungszeitpunkt
  static unsigned long updateTime = 0;

  // Aktualisierung des Displays nur alle UPDATE_PERIOD, ansonsten
  // funktioniert die Kommunikation über die Schnittstelle nicht
  if (millis() > updateTime)
  {
    byte i = 0;
    byte  values[8];
    values[i++] = highByte((unsigned int)  ppm[0]);
    values[i++] = lowByte( (unsigned int)  ppm[0]);
    values[i++] = highByte((unsigned int)  ppm[1]);
    values[i++] = lowByte( (unsigned int)  ppm[1]);
    values[i++] = highByte((unsigned int)  ppm[2]);
    values[i++] = lowByte( (unsigned int)  ppm[2]);
    values[i++] = highByte((unsigned int)  ppm[3]);
    values[i++] = lowByte( (unsigned int)  ppm[3]);

    // Senden des Datenblocks
    Serial1.write(SERIAL_START);
    Serial1.write(SERIAL_START);
    Serial1.write(i + 2); 
    for (int idx = 0; idx < i; idx++)  Serial1.write(values[idx]);
    Serial1.write(SERIAL_END);
    Serial1.write(SERIAL_END);

    // Nächste Aktualisierung des Displays bestimmen
    updateTime = millis() + UPDATE_PERIOD;
  }
}
  // -----------------------------------------------------------------------------------------------------



// ----------------------------------------------------------------------------------------------------------------------------
// Interrupt service routine: Connected to compare interrupt on timer 1 (TIMER1_COMPA_vect)
// ----------------------------------------------------------------------------------------------------------------------------
// Generates the CPPM signal on the output following the standard definition as follows:
// Pulses P0..Pn+1 are of defined length between 200..500µs, the servo signal on each channel is encoded by the duration between
// two following pulse (including the duration of the pulse).
// For n channels, n+1 pulses are needed to close the complete packet.
//           
//          +--+            +--+                +--+                     +--+        +--+                        +--+
//          |  |       .    |  |        .       |  |                     |  |        |  |                        |  |
//          |  |       .    |  |        .       |  |                     |  |        |  |                        |  |
//          |  |       .    |  |        .       |  |                     |  |        |  |                        |  |
// ---------+  +-------+----+  +--------+-------+  +----    ......   ----+  +--------+  +------------------------+  +------
//          |P0|       ^    |P1|        ^       |P2|                     |Pn|        |Pn+1|                      |P0|
//       (200...500µs)      |         1.5ms     |                        |           |  |                        |
//          |<--- Ch. 0 --->|<--------->|       |                        |           |  |                        |<- Channel 0 ...
//          |   (1..2ms)    |        (center)   |                        |           |  |                        |
//          |               |<----- Ch. 1 ----->|                        |           |  |                        |
//          |                                   |<---- Ch. 2 ...         |           |  |                        |
//          |                                                            |<- Ch. N ->|  |                        |
//          |                                                                           |<----- frame sync ----->|
//          |                                                                        ( frame_length - sum(channel_i) )
//          |<----------------------------------------- frame length ------------------------------------------->|
//                                                       (20..23ms)
ISR(TIMER1_COMPA_vect)
{  
  static byte         channel       = 0;      // current channel of CPPM packet to be transmitted in this ISR call
  static unsigned int totalTimeCPPM = 0;      // total time of the CPPM packet in microseconds
  static boolean      pulse         = true;   // indicates HIGH or LOW level for the output

  // Set pulse for current channel
  if (pulse) 
  { 
    digitalWrite(CPPM_PORT, HIGH);            // high level on ppm port -> start pulse
    OCR1A = PULSE_LENGTH * MICRO_SECONDS;     // set timer to length of the pulse 
    pulse = false;                            // prepare next interrupt to reset the ppm port to low level
  } 
  
  // Otherwise pause/low level to encode the servo position
  else
  { 
    digitalWrite(CPPM_PORT, LOW);   // low level on ppm port -> reset the pulse and start the pause modulation to encode the servo position
    pulse = true;                   // prepare next interrupt to set the ppm port to high level for the next pulse

    // Prepare timer for next interrupt, distinguish next channel or final pulse to close the CPPM packet
    // Close CPPM packet with final pulse
    if (channel >= PPM_CHANNELS)
    {
      // set remaining pause time to fill the full CPPM frame, final pulse n+1 must not be forgotten
      OCR1A          = (FRAME_LENGTH - (totalTimeCPPM + PULSE_LENGTH)) * MICRO_SECONDS; 
      channel        = 0; // reset to channel 0 (CPPM sequence will be transmitted with next timer interrupt starting with channel 0)
      totalTimeCPPM  = 0; // reset totalTimeCPPM for transmission of next CPPM packet
    }

    // otherwise timer for next pulse for channel "channel"
    else
    {
      OCR1A          = (ppmISR[channel] - PULSE_LENGTH) * MICRO_SECONDS; // encode the servo position by the duration of the pause + pulse length
      totalTimeCPPM += ppmISR[channel];   // add the duration of current CPPM channel (1..2ms) to the already consumed total time of the CPPM packet up to the current channel
      channel++;                          // prepare for the next channel to be sent
    }         
  }

}
// ----------------------------------------------------------------------------------------------------------------------------
