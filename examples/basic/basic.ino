// ---------------------------------------------------------------------------
//
// basic.ino
//
// A MIDI sequencer example using a standard MIDI cable and a push button
// attached to pin 4.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "FifteenStep.h"

// sequencer init
FifteenStep seq = FifteenStep();

// save button state
int button_last = 0;

void setup() {

  // set MIDI baud
  Serial.begin(31250);

  // initialize digital pin 13 as an output
  pinMode(13, OUTPUT);

  // initialize digital pin 4 as an input for a button
  pinMode(4, INPUT);

  // start sequencer and set callbacks
  seq.begin();
  seq.setMidiHandler(midi);
  seq.setStepHandler(step);

}

void loop() {

  // read the state of the button
  int button = digitalRead(4);

  // check for button press or release and
  // send note on or off to seqencer if needed
  if(button == HIGH && button_last == LOW) {

    // button pressed. play middle C preview now
    midi(0x0, 0x9, 0x3C, 0x40);
    // store note in sequence
    seq.setNote(0x0, 0x3C, 0x40);

  } else if(button == LOW && button_last == HIGH) {

    // button released. send middle C note off preview now
    midi(0x0, 0x8, 0x3C, 0x0);
    // store note off in sequence
    seq.setNote(0x0, 0x3C, 0x0);

  }

  // save button state
  button_last = button;

  // this is needed to keep the sequencer
  // running. there are other methods for
  // start, stop, and pausing the steps
  seq.run();

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                         SEQUENCER CALLBACKS                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// called when the step position changes. both the current
// position and last are passed to the callback
void step(int current, int last) {

  // blink on even steps
  if(current % 2 == 0)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);

}

// the callback that will be called by the sequencer when it needs
// to send midi commands. this specific callback is designed to be
// used with a standard midi cable.
//
// the following image will show you how your MIDI cable should
// be wired to the Arduino:
// http://arduino.cc/en/uploads/Tutorial/MIDI_bb.png
void midi(byte channel, byte command, byte arg1, byte arg2) {

  if(command < 128) {
    // shift over command
    command <<= 4;
    // add channel to the command
    command |= channel;
  }

  // send MIDI data
  Serial.write(command);
  Serial.write(arg1);
  Serial.write(arg2);

}

