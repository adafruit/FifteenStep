// ---------------------------------------------------------------------------
//
// FifteenStepNote.cpp
// A generic MIDI note class used by FifteenStep.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "Arduino.h"
#include "FifteenStep.h"

FifteenStepNote::FifteenStepNote() {
  on = 0x0;
  pitch = 0x0;
  velocity = 0x0;
  available = true;
}

void FifteenStepNote::set(bool new_on, byte new_pitch, byte new_velocity)
{

  // shut note off if it's on already
  if(on && new_on)
    new_on = 0x0;

  on = new_on;
  velocity = new_velocity;
  pitch = new_pitch;
  available = false;

}

