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
}

void FifteenStep::setSteps(int steps)
{

  int i = 0;

  // free memory if sequence array is set
  if(_sequence)
  {

    // clear positions
    for(; i < _steps; ++i) {

      if(_sequence[i])
        delete [] _sequence[i];

    }

    // free main sequence array
    delete [] _sequence;

  }

  // set new step value
  _steps = steps;

  // initialize the main dimension of new sequence array
  _sequence = new FifteenStepNote*[_steps];

  // guess how many notes to track for each
  // position by using step count as default
  for(i=0; i < _steps; ++i)
  {
    _sequence[i] = new FifteenStepNote[_steps];
  }

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

void FifteenStep::set(bool on, byte pitch, byte velocity) {

  // bail if the sequence is broken
  if(! _sequence || ! _sequence[_position])
    return;

  int i = 0;

  for(; i < _positionLength(); ++i)
  {

    // if note is already set at position
    // just modify that instance
    if(pitch == _sequence[_position][i].pitch)
    {
      _sequence[_position][i].set(on, pitch, velocity);
      return;
    }

    // if note doesn't exist at position
    // set the first available free spot
    if(_sequence[_position][i].available) {
      _sequence[_position][i].set(on, pitch, velocity);
      return;
    }

  }

  // if we reach here, we're out of free spots
  // at this location and need to increase the size
  // of the array of notes
  _positionResize();

  // increment i by 1 so we can use the new array slots
  i++;

  // bail. there should be a new free spot here
  if(! _sequence[_position][i].available)
    return;

  // all is good. set the note
  _sequence[_position][i].set(on, pitch, velocity);

}

void FifteenStep::_positionResize()
{

  // double size of position to avoid
  // doing this a bunch of times
  int length = _positionLength();
  int new_length = length * 2;
  FifteenStepNote *tmp = new FifteenStepNote[new_length];

  // copy old array to new array
  memcpy(tmp, _sequence[_position], length * sizeof(FifteenStepNote));

  // delete old array
  delete [] _sequence[_position];

  // set position to new array
  _sequence[_position] = tmp;

}

int FifteenStep::_positionLength()
{
  // TODO: this doesn't work with pointers
  return sizeof(_sequence[_position]) / sizeof(FifteenStepNote);
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

  // loop through the position and trigger notes
  for(int i=0; i < _positionLength(); ++i)
  {

    // we've reached the end of set notes
    // at this position, so we can bail
    if(_sequence[_position][i].available)
      break;

    _midi_cb(
      _sequence[_position][i].on ? 0x9 : 0x8,
      _sequence[_position][i].pitch,
      _sequence[_position][i].velocity
    );

  }

}

