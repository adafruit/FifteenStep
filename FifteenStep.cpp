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

  if(! _running)
    return;

  // what's the time?
  unsigned long now = millis(); // it's time to get ill.

  // send clock
  if(now >= _next_clock) {
    _tick();
    _next_clock = now + _clock;
  }

  // only step if it's time
  if(now < _next_beat)
    return;

  // advance and send notes
  _step();

  // add shuffle offset to next beat if odd step
  if((_position % 2) != 0)
    _next_beat = now + _sixteenth + _shuffle;
  else
    _next_beat = now + _sixteenth - _shuffle;

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

  if(_tempo < 0)
    _tempo = 0;

  if(_tempo > FS_MAX_TEMPO)
    _tempo = FS_MAX_TEMPO;

  // 60 seconds / bpm / 4 sixteeth notes per beat
  // gives you the value of a sixteenth note
  _sixteenth = 60000L / _tempo / 4;

  // midi clock messages should be sent 24 times
  // for every quarter note
  _clock = 60000L / _tempo / 24;

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

  // loop through the sequence and clear notes past the current step
  for(int i=0; i < _sequence_size; ++i)
  {

    // reset any steps that are over the current step count
    if(_sequence[i].step >= _steps)
      _sequence[i] = DEFAULT_NOTE;

  }

}

// increaseTempo
//
// Allows user to dynamically increase the tempo amount
// until the max tempo has been reached
//
// @access public
// @return void
//
void FifteenStep::increaseTempo()
{
  setTempo(_tempo + 1);
}

// decreaseTempo
//
// Allows user to dynamically decrease the tempo
// amount until the minimum (0) tempo has
// been reached.
//
// @access public
// @return void
//
void FifteenStep::decreaseTempo()
{
  setTempo(_tempo - 1);
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

  // we're safe, increase by division
  _shuffle += div;

  // make sure the next value doesn't
  // push the note past the next one
  if(_sixteenth <= _shuffle)
    _shuffle = _sixteenth - div;

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
  unsigned long previous = _shuffle;

  // decrease by division
  _shuffle -= div;

  // make sure we stop at zero
  if(previous > _shuffle)
    _shuffle = 0;

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
void FifteenStep::setNote(byte channel, byte pitch, byte velocity)
{

  if(! _running)
    return;

  int i = 0;
  int position = _quantizedPosition();
  bool existing_on = false;
  bool existing_off = false;
  bool added = false;
  int toggle_note = 0;

  // find a new slot to use
  for(i=0; i < _sequence_size; ++i)
  {

    // used by another pitch, keep going
    if(_sequence[i].pitch > 0 && _sequence[i].pitch != pitch)
      continue;

    // used by another step, keep going
    if(_sequence[i].step != position && _sequence[i].pitch != 0)
      continue;

    // used by another channel, keep going
    if(_sequence[i].channel != channel && _sequence[i].pitch != 0)
      continue;

    // matches the sent step, pitch & channel
    if(_sequence[i].pitch == pitch && _sequence[i].step == position && _sequence[i].channel == channel)
    {

      // this is a note on
      if(_sequence[i].velocity > 0)
      {

        // check if there's already a note on at this step
        if(existing_on)
        {

          // free this one since we don't need two at this step
          _sequence[i] = DEFAULT_NOTE;

          // turn toggle note off since this step already had a note on
          _sequence[toggle_note].velocity = 0;

          // now there's an existing off
          existing_off = true;

        }
        else
        {
          // this is the first on at this step
          existing_on = true;
          // mark the location of this note on so we can toggle it if needed
          toggle_note = i;
        }

      }
      else
      {

        // if there is already a note off at this step, we can reuse this slot.
        // if not, then mark this as the existing off note
        if(existing_off)
          _sequence[i] = DEFAULT_NOTE;
        else
          existing_off = true;

      }

    }

    // not a match, but there's a free slot and we haven't saved the new note yet.
    if(_sequence[i].pitch == 0 && _sequence[i].step == 0 && _sequence[i].channel == 0 && !added)
    {

      // make sure there already isn't a note on at this step
      if(existing_on && velocity > 0)
      {

        // turn toggle note off since this step already had a note on
        _sequence[toggle_note].velocity = 0;

        // now there's an existing off
        existing_off = true;

        // we don't need to add anything, so mark it as complete
        added = true;
        continue;

      }

      // make sure there already isn't a note off at this step
      if(existing_off && velocity == 0)
      {
        // we don't need to add anything, so mark it as complete
        added = true;
        continue;
      }

      // free slot. use it
      _sequence[i].channel = channel;
      _sequence[i].pitch = pitch;
      _sequence[i].velocity = velocity;
      _sequence[i].step = position;

      // mark this as the appropriate message
      if(velocity > 0)
      {
        existing_on = true;
        toggle_note = i;
      }
      else
      {
        existing_off = true;
      }

      // mark as added
      added = true;

    }

  }

}

// pause
//
// Pauses and unpauses the sequencer at
// the current position
//
// @access public
// @return void
//
void FifteenStep::pause()
{
  _running = _running ? false : true;
}

// start
//
// Starts sequencer at position 0
//
// @access public
// @return void
//
void FifteenStep::start()
{
  _position = 0;
  _running = true;
}

// stop
//
// Stops sequencer at current position
//
// @access public
// @return void
//
void FifteenStep::stop()
{
  _running = false;
}

// panic
//
// Turns all notes off and resets sequence
//
// @access public
// @return void
//
void FifteenStep::panic()
{

  for(int i=0; i < 16; ++i)
  {
    // send all notes off for each channel if callback is set
    if(_midi_cb)
      _midi_cb(i, 0x7B, 0x0, 0x0);
  }

  // clear notes
  _resetSequence();

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

  _running = true;
  _next_beat = 0;
  _next_clock = 0;
  _position = 0;
  _shuffle = 0;
  _sequence_size = memory / sizeof(FifteenStepNote);
  _sequence = new FifteenStepNote[_sequence_size];

  // set up default notes
  _resetSequence();

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

// _resetSequence
//
// Sets sequence to default state
//
// @access private
// @return void
void FifteenStep::_resetSequence()
{
  // set sequence to default note value
  for(int i=0; i < _sequence_size; ++i)
    _sequence[i] = DEFAULT_NOTE;
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

  if(_shuffle > 0)
    return _position;

  // what's the time?
  unsigned long now = millis();

  // calculate value of 32nd note
  unsigned long thirty_second = _sixteenth / 2;

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

  // tell the callback where we are
  // if it has been set by the sketch
  if(_step_cb)
    _step_cb(_position, last);

  // trigger next set of notes
  _triggerNotes();

}

// _tick
//
// Calls the user defined MIDI callback with
// the midi clock message
//
// @access private
// @return void
//
void FifteenStep::_tick()
{

  // bail if the midi callback isn't set
  if(! _midi_cb)
    return;

  // tick
  _midi_cb(0x0, 0xF8, 0x0, 0x0);

}

// _loopPosition
//
// Calls the user defined MIDI callback with
// the position of playback
//
// @access private
// @return void
//
void FifteenStep::_loopPosition()
{

  // bail if the midi callback isn't set
  if(! _midi_cb)
    return;

  // send position
  _midi_cb(0x0, 0xF2, 0x0, _position);

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

  int i;

  // bail if the midi callback isn't set
  if(! _midi_cb)
    return;

  // loop through the sequence and trigger note offs at the current position.
  // they should be sent first.
  for(i=0; i < _sequence_size; ++i)
  {

    // ignore if it's not the current position
    if(_sequence[i].step != _position)
      continue;

    // ignore if not a note off for this loop
    if(_sequence[i].velocity > 0)
      continue;

    // if this position is in the default state, ignore it
    if(_sequence[i].pitch == 0 && _sequence[i].velocity == 0 && _sequence[i].step == 0)
      continue;

    // send value to callback
    _midi_cb(
      _sequence[i].channel,
      0x8,
      _sequence[i].pitch,
      _sequence[i].velocity
    );

  }

  // loop through the sequence again and trigger note ons at the current position
  for(i=0; i < _sequence_size; ++i)
  {

    // ignore if it's not the current position
    if(_sequence[i].step != _position)
      continue;

    // ignore if not a note off for this loop
    if(_sequence[i].velocity == 0)
      continue;

    // if this position is in the default state, ignore it
    if(_sequence[i].pitch == 0 && _sequence[i].velocity == 0 && _sequence[i].step == 0)
      continue;

    // send value to callback
    _midi_cb(
      _sequence[i].channel,
      0x9,
      _sequence[i].pitch,
      _sequence[i].velocity
    );

  }

}
