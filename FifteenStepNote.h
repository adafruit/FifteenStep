// ---------------------------------------------------------------------------
//
// FifteenStepNote.h
// A generic MIDI note class used by FifteenStep.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#ifndef _FifteenStepNote_h
#define _FifteenStepNote_h

#include "Arduino.h"

class FifteenStepNote
{
  public:
    FifteenStepNote();
    void  set(bool new_on, byte new_pitch, byte new_velocity);
    byte  pitch;
    byte  velocity;
    bool  on;
    bool  available;
};

#endif

