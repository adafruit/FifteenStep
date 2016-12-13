#include "Wire.h"
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
