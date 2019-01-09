// ---------------------------------------------------------------------------
//
// neotrellism4.ino
//
// A MIDI sequencer example using the Adafruit NeoTrellis M4
// 1x NeoTrellis M4 Kit: https://www.adafruit.com/product/4020
//
// Required dependencies:
// Adafruit NeoPixel
// Adafruit DMA NeoPixel
// Adafruit Zero DMA
// Adafruit QSPI
// Adafruit Keypad
// Adafruit NeoTrellis M4
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015-2019 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "FifteenStep.h"

#define SEQUENCER_MEMORY 1024
FifteenStep seq = FifteenStep(SEQUENCER_MEMORY);

// set initial state for dynamic values
int tempo = 60;
int channel = 10;
int pitch[] = {36, 38, 42, 46}; // general midi channel 10: kick, snare, hat closed, hat open
int vel[] = {100, 80, 80, 80};
int steps = 16;

#include "config/ui.h"
#include "config/trellis.h"

#include "helpers/button.h"
#include "helpers/callback.h"
#include "helpers/display.h"
#include "helpers/command.h"

void setup() {

  trellis_init();

  // start sequencer and set callbacks
  seq.begin(tempo, steps);
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

  prevReadTime = millis();
}

void loop() {
  seq.run();
  readButtons();
}
