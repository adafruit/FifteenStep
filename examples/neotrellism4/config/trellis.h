#include <Adafruit_NeoTrellisM4.h>

#define WIDTH     8
#define N_BUTTONS 32

Adafruit_NeoTrellisM4 trellis = Adafruit_NeoTrellisM4();

uint8_t xy2i(uint8_t x, uint8_t y) {
  return (y * WIDTH) + x;
}

void i2xy(uint8_t i, uint8_t *x, uint8_t *y) {
  *y = i / WIDTH;
  *x = i - (*y * WIDTH);
}

void trellis_init() {
  trellis.begin();
  trellis.setBrightness(80);
  trellis.enableUSBMIDI(true);
  trellis.enableUARTMIDI(true);
}
