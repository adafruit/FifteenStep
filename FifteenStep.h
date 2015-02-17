// ---------------------------------------------------------------------------
//
// FifteenStep.h
// A generic MIDI sequencer library for Arduino.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#ifndef _FifteenStep_h
#define _FifteenStep_h

#include "Arduino.h"

typedef void (*MIDIcallback) (byte command, byte arg1, byte arg2);

class FifteenStep
{
  public:
    FifteenStep();
    void            setTempo(int tempo);
    void            setSteps(int steps);
    void            run();
    void            registerOutput(MIDIcallback cb);
  private:
    MIDIcallback    _midi_cb;
    int             _tempo;
    int             _steps;
    int             _position;
    unsigned long   _sixteenth;
    unsigned long   _next_beat;
    void            _step();
    void            _noteOn();
    void            _noteOff();
};

#endif
