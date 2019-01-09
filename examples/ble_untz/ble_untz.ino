// ---------------------------------------------------------------------------
//
// ble_untz.ino
//
// A MIDI sequencer example using the Adafruit UNTZ and a Bluefruit Feather.
//
// 1x Feather Bluefruit LE 32u4: https://www.adafruit.com/products/2829
// 1x Adafruit UNTZ: https://www.adafruit.com/product/1929
//
// Required dependencies:
// Adafruit Bluefruit nRF51 Library
// Adafruit UNTZtrument Library
// Adafruit Trellis Library
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015-2016 Adafruit Industries
// License: GNU GPLv3
// // ---------------------------------------------------------------------------
#include "FifteenStep.h"

#define SEQUENCER_MEMORY 512
FifteenStep seq = FifteenStep(SEQUENCER_MEMORY);

// set initial state for dynamic values
int tempo = 60;
int channel = 0;
int pitch[] = {36, 38, 42, 46, 44, 55, 56, 57};
int vel[] = {100, 80, 80, 80, 80, 80, 80, 80};
int steps = 16;

#include "config/bluefruit.h"
#include "config/ui.h"
#include "config/untz.h"

#include "helpers/button.h"
#include "helpers/callback.h"
#include "helpers/display.h"
#include "helpers/command.h"
#include "helpers/button.h"

void setup() {

  bluefruit_init();
  untz_init();

  // start sequencer and set callbacks
  seq.begin(tempo, steps);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

  prevReadTime = millis();
}

void loop() {

  ble.update(1);

  if(! isConnected) {
    untztrument.clear();
    untztrument.writeDisplay();
    return;
  }

  // this is needed to keep the sequencer
  // running. there are other methods for
  // start, stop, and pausing the steps
  seq.run();

  readButtons();

}

