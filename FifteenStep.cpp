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
  _tempo = 120;
  _steps = 16;
}

void FifteenStep::update(void)
{
  _timer.update();
}
