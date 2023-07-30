// This is part of the PDQ re-mixed version of Adafruit's GFX library
// and associated chipset drivers.
// Here is the libraries original copyright notice:

/***************************************************
  This is an example sketch for the Adafruit 2.2" SPI display.
  This library works with the Adafruit 2.2" TFT Breakout w/SD card
  ----> http://www.adafruit.com/products/1480
 
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

// Serielle Schnittstelle
#define BAUD_RATE                 57600
#define SERIAL_START              255
#define SERIAL_END                128
#define UPDATE_PERIOD             100


// LCD PIN  Uno (328) Leo (32u4)  644/1284
// -------  --------- ---------- --------
// 1  VCC   3.3V/5V  3.3V/5V  3.3V/5V // +3.3V or 5V with on-board regulator
// 2  GND     GND      GND      GND
// 3* CS      10       10      4  // Could be any GPIO pin, but then need to make sure SS isn't a LOW input (or slave SPI mode)
// 4* RESET   0/8/RESET  0/8/RESET  0/RESET // This relies on soft-reset. You can also use Arduino reset pin (if correct voltage).
// 5* DC/RS   9        9       3  // Could be any GPIO pin
// 6* SDI/MOSI  11      ICSP4      5  // HW SPI pin (can't change)
// 7* SCK     13      ICSP3      7  // HW SPI pin (can't change) NOTE: On Uno this causes on-board LED to flicker during SPI use
// 8* LED  3.3V/5V   3.3V/5V  3.3V/5V // LCD screen blanked when LOW (could use GPIO for PWM dimming)
// 9  SDO/MISO      -       -    -  // (not used if present, LCD code is currently "write only")
//
//  * = Typically only 3.3V safe logic-line (unless board has level converter [ala Adafruit]). Be careful with 5V!



#define ILI9341_SAVE_SPI_SETTINGS 0   // <= 0/1 with 1 to save/restore AVR SPI control and statusregisters (required when other SPI devices are in use with other settings)
#define ILI9341_CS_PIN            9   // 5   // <= /CS pin (chip-select, LOW to get attention of ILI9341, HIGH and it ignores SPI bus)
#define ILI9341_DC_PIN           10   // 6   // <= DC pin (1=data or 0=command indicator line) also called RS
#define ILI9341_RST_PIN           8   //4   // <= RST pin (optional)
#define LED_ON                    7


#define BUZZER                    3
#define UBATTREF                  7


#include <SPI.h>				            // must include this here (or else IDE can't find it)
#include <PDQ_GFX.h>				        // PDQ: Core graphics library
#include <PDQ_ILI9341.h>			      // PDQ: Hardware-specific driver library

// Fonts
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>	
#include <Fonts/FreeSerifItalic12pt7b.h>

// NOTE: Changes to test with Adafruit libraries (comment out PDQ lines above and un-comment the AF: ones below)
// AF: #include <Adafruit_GFX.h>          // AF: Core graphics library
// AF: #include <Adafruit_ILI9341.h>      // AF: Hardware-specific library
// AF: Adafruit_ILI9341 tft(10,  9, 8);   // AF: create LCD object (HW SPI, CS=pin 10, D/C=pin 8, reset=9)


PDQ_ILI9341 tft;      // PDQ: create LCD object (using pins in "PDQ_ILI9341_config.h")

// These are used to get information about static SRAM and flash memory sizes
extern "C" char __data_start[];		  // start of SRAM data
extern "C" char _end[];			        // end of SRAM data (used to check amount of SRAM this program's variables use)
extern "C" char __data_load_end[];	// end of FLASH (used to check amount of Flash this program's code and data uses)


// -----------------------------------------------------------------------------------------------
// Tonausgabe
struct ToneSequence
{
  ToneSequence(int _freq, int _duration, int _pause)
  : frequency(_freq)
  , duration(_duration)
  , pause(_pause)
  {};
 
  int frequency = 0;
  int duration  = 0;
  int pause     = 0;
};
typedef ToneSequence TS;


void setup()
{
 // put your setup code here, to run once:
  pinMode(LED_ON, OUTPUT);
  digitalWrite(LED_ON, HIGH);

#if defined(ILI9341_RST_PIN)	// reset like Adafruit does
	FastPin<ILI9341_RST_PIN>::setOutput();
	FastPin<ILI9341_RST_PIN>::hi();
	FastPin<ILI9341_RST_PIN>::lo();
	delay(1);
	FastPin<ILI9341_RST_PIN>::hi();
#endif

	tft.begin();			// initialize LCD
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 35);
  tft.setTextColor(65535);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);
  tft.setTextWrap(false);
  tft.println("Start...");
  
  Serial.begin(BAUD_RATE);
  pinMode(BUZZER, OUTPUT);

}


int thrust   = 0;
int rudder   = 0;
int elevator = 0;
int aileron  = 0;
int uBattRef = 0;
  

void loop(void)
{

// ----------------------------------------------------------------------------------
  // Datenstrom entsprechend Protokoll über die serielle Schnittstelle einlesen
  byte rxMsg[8] = {0};
  byte amount    = 0;

  byte rxByte = 0;
  while (1)
  {
    if (Serial.available() > 2)
    {
      rxByte = Serial.read();
      if ((rxByte == SERIAL_START) && (Serial.peek() == SERIAL_START)) 
      {
        Serial.read(); // Zweite Start-Byte verwerfen
        amount = Serial.read();
        break;
      }
    }
  }
  Serial.readBytes(rxMsg, amount);
  uBattRef = analogRead(UBATTREF);


  // ----------------------------------------------------------------------------------
  // Empfangene Daten verarbeiten und interpretieren
  thrust   = (unsigned int)  word(rxMsg[0],rxMsg[1]);  
  rudder   = (unsigned int)  word(rxMsg[2],rxMsg[2]);  
  elevator = (unsigned int)  word(rxMsg[4],rxMsg[5]);  
  aileron  = (unsigned int)  word(rxMsg[6],rxMsg[7]);  
  
  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(0, 35);
  tft.setTextColor(65535);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setFont(&FreeSerif12pt7b);

  tft.print("Thrust: ");
  tft.println(thrust);
  tft.print("Rudder: ");
  tft.println(rudder);
  tft.print("Elevator: ");
  tft.println(elevator);
  tft.print("Aileron: ");
  tft.println(aileron);
  tft.print("Batterie: ");
  tft.println(uBattRef);

  if (thrust < 1500) noTone(BUZZER);
  else tone(BUZZER, elevator);
}
