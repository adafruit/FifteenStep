// ---------------------------------------------------------------------------
//
// FifteenStep.cpp
// A step sequencer library for Arduino.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "Arduino.h"
#include "Timer.h"
#include "FifteenStep.h"

FifteenStep::FifteenStep()
{
  setTempo(120);
  setSteps(16);
  _timer.after(_sixteenth, _tick, this);
}

void FifteenStep::setTempo(int tempo)
{
  _tempo = tempo;
  _sixteenth = 60000 / _tempo / 4;
}

void FifteenStep::setSteps(int steps)
{
  _steps = steps;
}

void FifteenStep::run()
{
  _timer.update();
}

void FifteenStep::_tick(void *s)
{
  FifteenStep *fs = (FifteenStep *)s;
  fs->trigger();
}

void FifteenStep::trigger()
{
  _timer.after(_sixteenth, _tick, this);
  _noteOff();
  _noteOn();
}

void FifteenStep::_noteOn()
{
  // TODO callbacks
  // don't implement MIDI directly
  MIDIEvent noteOn = {0x09, 0x90, 0x3C, 0x40};
  MIDIUSB.write(noteOn);
  MIDIUSB.flush();
}

void FifteenStep::_noteOff()
{
  // TODO callbacks
  // don't implement MIDI directly
  MIDIEvent noteOff = {0x09, 0x80, 0x3C, 0x40};
  MIDIUSB.write(noteOff);
  MIDIUSB.flush();
}
