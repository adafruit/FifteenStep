// ---------------------------------------------------------------------------
//
// FifteenStep.cpp
// A generic MIDI sequencer library for Arduino.
//
// Author: Todd Treece <todd@uniontownlabs.org>
// Copyright: (c) 2015 Adafruit Industries
// License: GNU GPLv3
//
// ---------------------------------------------------------------------------
#include "Arduino.h"
#include "FifteenStep.h"

FifteenStep::FifteenStep()
{

  _next_beat = 0;
  _position = 0;

  setTempo(120);
  setSteps(16);
}

void FifteenStep::setTempo(int tempo)
{
  _tempo = tempo;
  _sixteenth = 60000L / _tempo / 4;
}

void FifteenStep::setSteps(int steps)
{
  _steps = steps;
}

void FifteenStep::run()
{

  // what's the time?
  unsigned long now = millis(); // it's time to get ill.

  // don't step yet
  if(now < _next_beat)
    return;

  // advance and send notes
  _step();

  // add the length of a sixteenth note to now
  // so we know when to trigger the next step
  _next_beat = now + _sixteenth;

}

void FifteenStep::registerOutput(MIDIcallback cb)
{

  // set the callback to use with _noteOn & _noteOff
  _midi_cb = cb;

}

void FifteenStep::_step()
{

  _noteOff();

  // increment the position
  _position++;

  // start over if we've reached the end
  if(_position >= _steps)
    _position = 0;

  _noteOn();

}

void FifteenStep::_noteOn()
{

  // bail if the callback isn't set
  if(! _midi_cb)
    return;

  _midi_cb(0x9, 50 + _position, 0x40);

}

void FifteenStep::_noteOff()
{

  // bail if the callback isn't set
  if(! _midi_cb)
    return;

  _midi_cb(0x8, 50 + _position, 0x40);

}
