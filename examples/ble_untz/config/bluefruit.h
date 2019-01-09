// COMMON SETTINGS
// ----------------------------------------------------------------------------------------------
// These settings are used in both SW UART, HW UART and SPI mode
// ----------------------------------------------------------------------------------------------
#define BUFSIZE                        128   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   false // If set to 'true' enables debug output


// ----------------------------------------------------------------------------------------------
// The following macros declare the pins that will be used for 'SW' serial.
// You should use this option if you are connecting the UART Friend to an UNO
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_SWUART_RXD_PIN       9    // Required for software serial!
#define BLUEFRUIT_SWUART_TXD_PIN       10   // Required for software serial!
#define BLUEFRUIT_UART_CTS_PIN         11   // Required for software serial!
#define BLUEFRUIT_UART_RTS_PIN         12   // Optional, set to -1 if unused


// HARDWARE UART SETTINGS
// ----------------------------------------------------------------------------------------------
// The following macros declare the HW serial port you are using. Uncomment
// this line if you are connecting the BLE to Leonardo/Micro or Flora
// ----------------------------------------------------------------------------------------------
#ifdef Serial1    // this makes it not complain on compilation if there's no Serial1
  #define BLUEFRUIT_HWSERIAL_NAME      Serial1
#endif


// SHARED UART SETTINGS
// ----------------------------------------------------------------------------------------------
// The following sets the optional Mode pin, its recommended but not required
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_UART_MODE_PIN        -1    // Set to -1 if unused


// HARDWARE SPI SETTINGS
// ----------------------------------------------------------------------------------------------
// The following macros declare the pins to use for HW SPI communication.
// SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno, etc.
// This should be used with nRF51822 based Bluefruit LE modules that use SPI.
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              6    // Optional but recommended, set to -1 if unused

#define FACTORYRESET_ENABLE            1
#define MINIMUM_FIRMWARE_VERSION       "0.7.0"

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BLEMIDI.h"

bool isConnected = false;

// error helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
void connected(void)
{
  isConnected = true;
  Serial.println(F(" CONNECTED!"));
}

void disconnected(void)
{
  Serial.println(F("DISCONNECTED"));
  isConnected = false;
}

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
Adafruit_BLEMIDI blemidi(ble);

void bluefruit_init() {

  Serial.print(F("Initializing the Bluefruit LE module: "));

  if(! ble.begin(VERBOSE_MODE)) {
    error(F("Couldn't find Bluefruit, check wiring?"));
  }
  Serial.println( F("OK!") );

  if(FACTORYRESET_ENABLE) {
    Serial.println(F("Performing a factory reset: "));
    if(! ble.factoryReset()) {
      error(F("Couldn't factory reset"));
    }
    Serial.println( F("OK!") );
  }

  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  ble.info();

  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);

  Serial.println(F("Enable MIDI: "));
  if(! blemidi.begin(true)) {
    error(F("Could not enable MIDI"));
  }

  ble.verbose(false);

  Serial.print(F("Waiting for a connection... "));
  while(! isConnected) {
    ble.update(500);
  }

}
