#include <Wire.h>
#include "Adafruit_Trellis.h"
#include "Adafruit_UNTZtrument.h"

// uncomment the define below if you are using the HELLA UNTZtrument.
// #define HELLA

#ifdef HELLA
  // A HELLA UNTZtrument has eight Trellis boards...
  Adafruit_Trellis     T[8];
  Adafruit_UNTZtrument untztrument(&T[0], &T[1], &T[2], &T[3],
                                   &T[4], &T[5], &T[6], &T[7]);
  const uint8_t        addr[] = { 0x70, 0x71, 0x72, 0x73,
                                  0x74, 0x75, 0x76, 0x77 };
#else
  // A standard UNTZtrument has four Trellises in a 2x2 arrangement
  // (8x8 buttons total).  addr[] is the I2C address of the upper left,
  // upper right, lower left and lower right matrices, respectively,
  // assuming an upright orientation, i.e. labels on board are in the
  // normal reading direction.
  Adafruit_Trellis     T[4];
  Adafruit_UNTZtrument untztrument(&T[0], &T[1], &T[2], &T[3]);
  const uint8_t        addr[] = { 0x70, 0x71,
                                  0x72, 0x73 };
#endif

#define WIDTH     ((sizeof(T) / sizeof(T[0])) * 2)
#define N_BUTTONS ((sizeof(T) / sizeof(T[0])) * 16)

void untz_init() {

#ifndef HELLA
  untztrument.begin(addr[0], addr[1], addr[2], addr[3]);
#else
  untztrument.begin(addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7]);
#endif

// Default Arduino I2C speed is 100 KHz, but the HT16K33 supports
// 400 KHz.  We can force this for faster read & refresh, but may
// break compatibility with other I2C devices...so be prepared to
// comment this out, or save & restore value as needed.
#ifdef ARDUINO_ARCH_SAMD
  Wire.setClock(400000L);
#endif
#ifdef __AVR__
  TWBR = 12; // 400 KHz I2C on 16 MHz AVR
#endif

  untztrument.clear();
  untztrument.writeDisplay();

}
