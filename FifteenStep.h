// ---------------------------------------------------------------------------
//
// FifteenStep.h
// A generic MIDI sequencer library for Arduino.
// // Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
// // ---------------------------------------------------------------------------
#ifndef _FifteenStep_h
#define _FifteenStep_h

#include "Arduino.h"

#define FS_DEFAULT_TEMPO 120
#define FS_DEFAULT_STEPS 16
#define FS_DEFAULT_MEMORY 512
#define FS_MIN_TEMPO 10
#define FS_MAX_TEMPO 250
#define FS_MAX_STEPS 256

// MIDIcallback
//
// This defines the MIDI callback function format that is required by the
// sequencer.
//
// Most of the time these arguments will represent the following:
//
// channel: midi channel
// command: note on or off (0x9 or 0x8)
// arg1: pitch value
// arg1: velocity value
//
// It's possible that there will be other types of MIDI messages sent
// to this callback in the future, so please check the command sent if
// you are doing something other than passing on the MIDI messages to
// a MIDI library.
//
typedef void (*MIDIcallback) (byte channel, byte command, byte arg1, byte arg2);

// StepCallback
//
// This defines the format of the step callback function that will be used
// by the sequencer. This callback will be called with the current
// step position and last step position whenever the step changes.
// Please check FifteenStep.cpp for more info about setting the callback
// that will be used.
//
typedef void (*StepCallback) (int current, int last);

// FifteenStepNote
//
// This defines the note type that is used when storing sequence note
// values. The notes will be set to DEFAULT_NOTE until they are modified
// by the user.
typedef struct
{
  byte channel;
  byte pitch;
  byte velocity;
  byte step;
} FifteenStepNote;

// default values for sequence array members
const FifteenStepNote DEFAULT_NOTE = {0x0, 0x0, 0x0, 0x0};

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
    void  pause();
    void  start();
    void  stop();
    void  panic();
    void  setTempo(int tempo);
    void  setSteps(int steps);
    void  increaseTempo();
    void  decreaseTempo();
    void  increaseShuffle();
    void  decreaseShuffle();
    void  setMidiHandler(MIDIcallback cb);
    void  setStepHandler(StepCallback cb);
    void  setNote(byte channel, byte pitch, byte velocity, byte step = -1);
    byte  getPosition();
    FifteenStepNote* getSequence();
  private:
    MIDIcallback      _midi_cb;
    StepCallback      _step_cb;
    FifteenStepNote*  _sequence;
    bool              _running;
    int               _sequence_size;
    int               _tempo;
    byte              _steps;
    byte              _position;
    unsigned long     _clock;
    unsigned long     _sixteenth;
    unsigned long     _shuffle;
    unsigned long     _next_beat;
    unsigned long     _next_clock;
    unsigned long     _shuffleDivision();
    int               _quantizedPosition();
    int               _greater(int first, int second);
    void              _init(int memory);
    void              _heapSort();
    void              _siftDown(int root, int bottom);
    void              _resetSequence();
    void              _loopPosition();
    void              _tick();
    void              _step();
    void              _triggerNotes();
};

#endif
