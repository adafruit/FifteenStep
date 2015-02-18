#include "FifteenStep.h"
#include "Adafruit_NeoPixel.h"

#define PIN            6
#define STEPS          7

FifteenStep seq = FifteenStep();
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(STEPS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {

  pixels.begin();
  pixels.setBrightness(10);

  seq.setTempo(50);
  seq.setSteps(STEPS);
  seq.midiHandler(midi);
  seq.stepHandler(step);

}

void loop() {
  seq.run();
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
