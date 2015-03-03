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

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                            CONSTRUCTORS                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// FifteenStep
//
// The default constructor that will reserve a chunk of
// sram in bytes equal to the value of FS_DEFAULT_MEMORY.
//
// @access public
//
FifteenStep::FifteenStep()
{
  _init(FS_DEFAULT_MEMORY);
}

// FifteenStep
//
// An alternative constructor that allows the user to set
// the amount of memory the sequencer will reserve. Setting
// the memory value to a custom value will alter the number of
// steps and the amount of polyphony the sequencer supports.
//
// @access public
// @param the amount of sram to reserve in bytes
//
FifteenStep::FifteenStep(int memory)
{
  _init(memory);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                            PUBLIC METHODS                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// begin
//
// Sets the tempo and step count to the default values.
//
// @access public
// @return void
//
void FifteenStep::begin()
{
  begin(FS_DEFAULT_TEMPO, FS_DEFAULT_STEPS);
}

// begin
//
// Sets the tempo to user supplied value in beats per
// minute (BPM), and sets the step count to the default
// value of 16.
//
// @access public
// @param tempo in beats per minute
// @return void
//
void FifteenStep::begin(int tempo)
{
  begin(tempo, FS_DEFAULT_STEPS);
}

// begin
//
// Sets the tempo to user supplied value in beats per
// minute (BPM), and sets the 16th note step count to the user
// supplied count value.
//
// @access public
// @param tempo in beats per minute
// @param number of 16th note steps before looping
// @return void
//
void FifteenStep::begin(int tempo, int steps)
{
  setTempo(tempo);
  setSteps(steps);
}

// run
//
// IMPORTANT: This method should only be called in the
// main loop of the sketch, and is required for the
// sequencer to work.
//
// This method checks the current time against the time of the
// next scheduled beat and steps the progression forward if the
// current time is equal to or greater than the next beat time.
//
// @access public
// @return void
//
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

  // add shuffle offset to next beat if odd step
  if((_position % 2) != 0)
    _next_beat += _shuffle;

}

// setTempo
//
// Allows user to dynamiclly set the tempo in
// beats per minute. The tempo value will be used to
// calculate the length of the 16th note steps, and the
// shuffle division value.
//
// @access public
// @return void
//
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

// setSteps
//
// Allows user to dynamically set the number of 16th note
// steps the sequencer will increment to before looping
// back to the beginning. Increasing the step count will
// decrease the amount of polyphony the sequencer supports.
//
// @access pubilc
// @return void
//
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

// increaseShuffle
//
// Allows user to dynamically increase the shuffle
// amount until the max shuffle value is reached.
//
// @access public
// @return void
//
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

// decreaseShuffle
//
// Allows user to dynamically decrease the shuffle
// amount until the minimum (0) shuffle amount has
// been reached.
//
// @access public
// @return void
//
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

// setMidiHandler
//
// IMPORTANT: Setting a MIDI handler is required for the sequencer
// to operate.
//
// Allows user to set a callback that will be called when
// the sequencer needs to send MIDI messages. Using this
// callback allows the sequencer to be decoupled from the
// MIDI implementation.
//
// The callback will be called with three arguments: the MIDI
// command, the first argument for that MIDI command, and the
// second argument for that command. For example, if the callback
// was called with a note on message, the arguments would be the
// note on command (0x9), the pitch value (0x3C), and the velocity
// value (0x40). The user is responsible for passing those values
// to the specific MIDI library they are using. Please check the
// typedef for MIDIcallback in FifteenStep.h for more info about
// the arguments.
//
// @access public
// @param the midi callback that the sequencer will use
// @return void
//
void FifteenStep::setMidiHandler(MIDIcallback cb)
{
  // store the passed callback
  _midi_cb = cb;
}

// setStepHandler
//
// Allows user to specifiy a callback that will be called
// whenever the progression steps forward. The callback will
// be called with the current position as the first argument,
// and the previous position as the second argument. Please check
// the typedef for StepCallback in FifteenStep.h for more info
// about the arguments passed to the callback.
//
// @access public
// @param the callback function to call when the progression advances
// @return void
//
void FifteenStep::setStepHandler(StepCallback cb)
{
  // store the passed callback
  _step_cb = cb;
}

// setNote
//
// Allows user to set a note on or off value at the current
// step position. If there is already a note on value at this
// position, the note will be turned off.
//
// @access public
// @param note on or off message
// @param pitch of note
// @param velocity of note
// @return void
//
void FifteenStep::setNote(bool on, byte pitch, byte velocity)
{

  int i = 0;
  int pos = _quantizedPosition();

  // store note offs as velocity 0 to save space
  if(! on)
    velocity = 0;

  // clean up unused values
  //_cleanup();

  // send midi note if the callback is set
  if(_midi_cb)
    _midi_cb(velocity > 0 ? 0x9 : 0x8, pitch, velocity);

  // loop through the sequence and make sure we reuse existing slots
  for(i=0; i < _sequence_size; ++i)
  {

    // continue if this isn't the current step and pitch
    if(_sequence[i].step != pos || _sequence[i].pitch != pitch)
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

    // no need to continue
    return;

  }

  // if we've made it here, we have to find a new slot to use.
  for(i=0; i < _sequence_size; ++i)
  {

    // used already, keep going
    if(_sequence[i].pitch > 0)
      continue;

    // free slot. use it
    _sequence[i].pitch = pitch;
    _sequence[i].velocity = velocity;
    _sequence[i].step = pos;

    // no need to continue
    return;

  }

}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                            PRIVATE METHODS                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// _init
//
// A common init method for the constructors to
// use when the class is initialized. Lowering the
// amount of memory the sequencer uses will effect the
// amount of polyphony the sequencer will support. By
// default the sequencer allocates 1k of sram.
//
// @access private
// @param the amount of sram to use in bytes
// @return void
//
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

// _shuffleDivision
//
// Calculates the size of the shuffle division
// to use when increasing or decreasing the shuffle
// amount.
//
// @access private
// @return size of shuffle division
//
unsigned long FifteenStep::_shuffleDivision()
{
  // split the 16th into 8 parts
  // so user can change the shuffle
  return _sixteenth / 8;
}

// _quantizedPosition
//
// Returns the closest 16th note to the
// present time. This is used to see where to
// save the new note.
//
// @access private
// @return quantized position
//
int FifteenStep::_quantizedPosition()
{

  // what's the time?
  unsigned long now = millis();

  // calculate value of 32nd note
  unsigned long thirty_second = _sixteenth / 2;

  // add or subtract shuffle if needed
  if((_position % 2) != 0)
    thirty_second -= (_shuffle / 2);
  else
    thirty_second += (_shuffle / 2);

  // use current position if below middle point
  if(now <= (_next_beat - thirty_second))
    return _position;

  // return first step if the next step
  // is past the step count
  if((_position + 1) >= _steps)
    return 0;

  // return next step
  return _position + 1;

}

// _cleanup
//
// Resets any set notes that don't have note on messages
// to the default note state. This allows their slots
// to be reused by new notes. This method also cleans up
// any notes that are outside of the current step range.
//
// @access private
// @return void
//
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

// _step
//
// Moves _position forward by one step, calls the
// step callback with the current & last step position,
// and triggers any notes at the current position.
//
// @access private
// @return void
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

  // trigger next set of notes
  _triggerNotes();

  // tell the callback where we are
  // if it has been set by the sketch
  if(_step_cb)
    _step_cb(_position, last);

}

// _triggerNotes
//
// Calls the user defined MIDI callback with
// all of the note on and off messages at the
// current step position.
//
// @access private
// @return void
//
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
