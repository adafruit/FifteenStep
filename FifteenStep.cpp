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
  _init(FS_DEFAULT_MEMORY);
}

FifteenStep::FifteenStep(int memory)
{
  _init(memory);
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

  // add shuffle offset to next beat if odd step
  if((_position % 2) != 0)
    _next_beat += _shuffle;

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

  // don't allow user to set a crazy amount of steps
  if(_steps > FS_MAX_STEPS)
    _steps = FS_MAX_STEPS;

  // clean up steps that aren't used anymore
  _cleanup();

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

void FifteenStep::setNote(bool on, byte pitch, byte velocity)
{

  int i = 0;

  // store note offs as velocity 0 to save space
  if(! on)
    velocity = 0;

  // clean up unused values
  _cleanup();

  // loop through the sequence and make sure we reuse existing slots
  for(i=0; i < _sequence_size; ++i)
  {

    // continue if this isn't the current step and pitch
    if(_sequence[i].step != _position && _sequence[i].pitch != pitch)
      continue;

    // if note on was sent and note is currently
    // on at this step, turn it off
    if(on && _sequence[i].velocity > 0)
    {
      _sequence[i].velocity = 0;
      return;
    }

    // set existing slot to new value
    _sequence[i].velocity = velocity;
    return;

  }

  // if we've made it here, we have to find a new slot to use.
  for(i=0; i < _sequence_size; ++i)
  {

    // used already, keep going
    if(_sequence[i].pitch > 0 && _sequence[i].velocity > 0)
      continue;

    // free slot. use it
    _sequence[i] = {pitch, velocity, _position};
    return;

  }

}

unsigned long FifteenStep::_shuffleDivision()
{
  // split the 16th into 8 parts
  // so user can change the shuffle
  return _sixteenth / 8;
}

void FifteenStep::_init(int memory)
{

  _next_beat = 0;
  _position = 0;
  _shuffle = 0;
  _sequence_size = memory / sizeof(FifteenStepNote);
  _sequence = new FifteenStepNote[_sequence_size];

  // set sequence to default note value
  for(int i=0; i < _sequence_size; ++i)
    _sequence[i] = DEFAULT_NOTE;

}

void FifteenStep::_cleanup()
{

  int i;
  byte pitches[16];

  // set up pitch defaults
  for(i=0; i < 16; ++i)
    pitches[i] = 0x0;

  // loop through the sequence and mark notes that have note ons
  for(i=0; i < _sequence_size; ++i)
  {

    // reset any steps that are over the current step count
    if(_sequence[i].step >= _steps)
      _sequence[i] = DEFAULT_NOTE;

    // ignore note offs
    if(_sequence[i].velocity == 0)
      continue;

    // flip the bit for the current pitch
    pitches[_sequence[i].pitch / 8] |= 1 << (_sequence[i].pitch % 8);

  }

  // clear any pitches that don't have note ons
  for(i=0; i < _sequence_size; ++i)
  {

    // if pitch has a note on, move forward
    if((pitches[_sequence[i].pitch / 8] >> (_sequence[i].pitch % 8)) & 1)
      continue;

    // clear notes if there are no note ons
    _sequence[i] = DEFAULT_NOTE;

  }

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

  // loop through the sequence and trigger notes at the current position
  for(int i=0; i < _sequence_size; ++i)
  {

    // ignore if it's not the current position
    if(_sequence[i].step != _position)
      continue;

    // if this position is in the default state, ignore it
    if(_sequence[i].pitch == 0 && _sequence[i].velocity == 0)
      continue;

    // send values to callback
    _midi_cb(
      _sequence[i].velocity > 0 ? 0x9 : 0x8,
      _sequence[i].pitch,
      _sequence[i].velocity
    );

  }

}

