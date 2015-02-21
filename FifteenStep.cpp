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
  _shuffle = 0;
  _polyphony = FS_DEFAULT_POLYPHONY;
}

FifteenStep::FifteenStep(int polyphony)
{
  _next_beat = 0;
  _position = 0;
  _shuffle = 0;
  _polyphony = polyphony;
}

void FifteenStep::begin()
{
  begin(FS_DEFAULT_TEMPO, FS_DEFAULT_STEPS);
}

void FifteenStep::begin(int tempo)
{
  begin(tempo, FS_DEFAULT_STEPS);
}

void FifteenStep::begin(int tempo, int steps)
{
  setTempo(tempo);
  setSteps(steps);
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

void FifteenStep::setTempo(int tempo)
{

  // tempo in beats per minute
  _tempo = tempo;

  // 60 seconds / bpm / 4 sixteeth notes per beat
  // gives you the value of a sixteenth note
  _sixteenth = 60000L / _tempo / 4;

  // grab new shuffle division
  unsigned long div = _shuffleDivision();

  // make sure the shuffle doesn't push the
  // note past the new sixteenth note value
  if((_sixteenth - div) > _shuffle)
    return;

  // reset shuffle to last value
  _shuffle = _sixteenth - div;

}

void FifteenStep::setSteps(int steps)
{
  // set new step value
  _steps = steps;
}

void FifteenStep::increaseShuffle()
{

  // grab current shuffle division
  unsigned long div = _shuffleDivision();

  // make sure the next value doesn't
  // push the note past the next one
  if(_sixteenth <= (_shuffle + div))
    return;

  // we're safe, increase by division
  _shuffle += div;

}

void FifteenStep::decreaseShuffle()
{

  // grab current shuffle division
  unsigned long div = _shuffleDivision();

  // make sure the next value doesn't
  // push the shuffle value negative
  if(0 > (_shuffle - div))
    return;

  // we're safe, decrease by division
  _shuffle -= div;

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

void FifteenStep::setNote(bool on, byte pitch, byte velocity) {


}

unsigned long FifteenStep::_shuffleDivision()
{
  // split the 16th into 8 parts
  // so user can change the shuffle
  return _sixteenth / 8;
}

void FifteenStep::_step()
{

  // save the last position so we
  // can provide it to the callback
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

  // loop through the position and trigger notes
  for(int i=0; i < length; ++i)
  {

    _midi_cb(
      _sequence[_position][i].on ? 0x9 : 0x8,
      _sequence[_position][i].pitch,
      _sequence[_position][i].velocity
    );

  }

}

