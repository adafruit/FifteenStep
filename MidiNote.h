// ---------------------------------------------------------------------------
//
// MidiNote.h
// A generic MIDI note class.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#ifndef _MidiNote_h
#define _MidiNote_h

#include "Arduino.h"

class MidiNote
{
  public:
    byte  on;
    byte  pitch;
    byte  velocity;
};

#endif

