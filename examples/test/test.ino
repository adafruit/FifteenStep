#include "FifteenStep.h"
#include "Adafruit_NeoPixel.h"
#include "Wire.h"
#include "Adafruit_MPR121.h"

#define PIN      6
#define STEPS    7
#define TEMPO    50

FifteenStep seq = FifteenStep();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(STEPS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_MPR121 cap = Adafruit_MPR121();

uint16_t lasttouched = 0;
uint16_t currtouched = 0;
int base = 60; // middle c

void setup() {

  delay(2000);

  pixels.begin();
  pixels.setBrightness(10);

  seq.begin(TEMPO, STEPS);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

  if (!cap.begin(0x5A)) {
    while (1);
  }

}

void loop() {

  seq.run();

  // TODO replace cap touch example code
  currtouched = cap.touched();

  for (uint8_t i=0; i<6; i++) {

    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      seq.set(true, base + i, 0x40);
    }

    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      seq.set(false, base + i, 0x40);
    }

  }

  lasttouched = currtouched;

}

void step(int current, int last) {
  pixels.setPixelColor(last, pixels.Color(0,0,0));
  pixels.setPixelColor(current, pixels.Color(0,0,150));
  pixels.show();
}

void midi(byte command, byte arg1, byte arg2) {
  MIDIEvent event = {command, command << 4, arg1, arg2};
  MIDIUSB.write(event);
  MIDIUSB.flush();
}
