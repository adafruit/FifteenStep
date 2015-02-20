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
}

void FifteenStep::begin()
{
  setTempo(FS_DEFAULT_TEMPO);
  setSteps(FS_DEFAULT_STEPS);
}

void FifteenStep::begin(int tempo)
{
  setTempo(tempo);
  setSteps(FS_DEFAULT_STEPS);
}

void FifteenStep::begin(int tempo, int steps)
{
  setTempo(tempo);
  setSteps(steps);
}

void FifteenStep::setTempo(int tempo)
{
  _tempo = tempo;
  _sixteenth = 60000L / _tempo / 4;
}

void FifteenStep::setSteps(int steps)
{

  // clear memory if notes array is set
  if(_sequence) {

    // clear second dimension
    for(int i=0; i < _steps; i++) {

      if(_sequence[i])
        delete [] _sequence[i];

    }

    // clear main array
    delete [] _sequence;

  }

  // set new value
  _steps = steps;

  // initialize a new seqence array
  _sequence = new FifteenStepNote *[_steps];

}

void FifteenStep::run()
{

  // what's the time?
  unsigned long now = millis();

  // don't step yet
  if(now < _next_beat)
    return;

  // advance and send notes
  _step();

  // add the length of a sixteenth note to now
  // so we know when to trigger the next step
  _next_beat = now + _sixteenth;

}

void FifteenStep::setMidiHandler(MIDIcallback cb)
{
  // the passed callback will be used
  // to send MIDI commands to the user's device
  _midi_cb = cb;
}

void FifteenStep::setStepHandler(StepCallback cb)
{
  // set the callback to call on position change
  _step_cb = cb;
}

void FifteenStep::_step()
{

  int last = _position;

  // increment the position
  _position++;

  // start over if we've reached the end
  if(_position >= _steps)
    _position = 0;

  // tell the callback where we are
  // if it has been set by the sketch
  if(_step_cb)
    _step_cb(_position, last);

  // trigger next set of notes
  _triggerNotes();

}

void FifteenStep::_triggerNotes()
{

  // bail if the midi callback isn't set
  if(! _midi_cb)
    return;

  // bail if no notes have been set for this position
  if(! _sequence || ! _sequence[_position])
    return;

  // get the number of notes stored at this position
  int length = sizeof(_sequence[_position]) / sizeof(FifteenStepNote);

  // loop through the position and trigger notes
  for(int i=0; i < length; i++)
  {

    _midi_cb(
      _sequence[_position][i].on ? 0x9 : 0x8,
      _sequence[_position][i].pitch,
      _sequence[_position][i].velocity
    );

  }

}
