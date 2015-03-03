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

#define FS_DEFAULT_TEMPO 120
#define FS_DEFAULT_STEPS 16
#define FS_MAX_MEMORY 1024
#define FS_MAX_STEPS 256

typedef struct
{
  byte pitch;
  byte velocity;
  byte step;
} FifteenStepNote;
typedef void (*MIDIcallback) (byte command, byte arg1, byte arg2);
typedef void (*StepCallback) (int current, int last);

const FifteenStepNote DEFAULT_NOTE = {0x0, 0x0, 0x0};

class FifteenStep
{
  public:
    FifteenStep();
    FifteenStep(int memory);
    void  begin();
    void  begin(int tempo);
    void  begin(int tempo, int steps);
    void  begin(int tempo, int steps, int polyphony);
    void  run();
    void  setTempo(int tempo);
    void  setSteps(int steps);
    void  increaseShuffle();
    void  decreaseShuffle();
    void  setMidiHandler(MIDIcallback cb);
    void  setStepHandler(StepCallback cb);
    void  setNote(bool on, byte pitch, byte velocity);
  private:
    MIDIcallback      _midi_cb;
    StepCallback      _step_cb;
    FifteenStepNote*  _sequence;
    int               _sequence_size;
    int               _tempo;
    byte              _steps;
    byte              _position;
    unsigned long     _sixteenth;
    unsigned long     _shuffle;
    unsigned long     _next_beat;
    unsigned long     _shuffleDivision();
    void              _cleanup();
    void              _step();
    void              _triggerNotes();
};

#endif
