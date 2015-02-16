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
}

void FifteenStep::setTempo(int tempo)
{
  _tempo = tempo;
}

void FifteenStep::setSteps(int steps)
{
  _steps = steps;
}

void FifteenStep::update(void)
{
  _timer.update();
}
