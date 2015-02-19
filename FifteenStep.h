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
#include "MidiNote.h"

typedef void (*MIDIcallback) (byte command, byte arg1, byte arg2);
typedef void (*StepCallback) (int current, int last);

class FifteenStep
{
  public:
    FifteenStep();
    void            run();
    void            setTempo(int tempo);
    void            setSteps(int steps);
    void            midiHandler(MIDIcallback cb);
    void            stepHandler(StepCallback cb);
    void            on(byte pitch, byte velocity);
    void            off(byte pitch, byte velocity);
  private:
    MIDIcallback    _midi_cb;
    StepCallback    _step_cb;
    MidiNote        **_notes;
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
