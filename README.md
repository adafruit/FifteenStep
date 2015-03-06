# FifteenStep

The goal of this library is to create a general purpose sequencer
that can be used with anything that can handle MIDI info. The library
provides a way for you to define callbacks that will be called whenever
MIDI data needs to be handled. This allows you to send MIDI data over standard
MIDI cables, through USB MIDI, or any other method you can dream up. It's
up to you.

Why is it called FifteenStep? Well... naming things is hard.

## Features

* The length of the loop and the amount of polyphony are based on how much memory you allocate to the sequencer
* Polyphony is global. You could use all of it on the first step, or evenly distribute notes over each step in the loop
* You can define your own callback that will be called on every position change. This can be used to make a simple UI.
* Quantization
* Tempo can be changed on the fly
* The loop point can be changed on the fly
* Shuffle can be added or subtracted on the fly
* Start, Stop, Pause the sequencer
* MIDI clock out
* MIDI song position out

## Contributing

In lieu of a formal styleguide, take care to maintain the existing coding style. Please test your code, and feel free
to send a pull request explaining your changes when you are ready.

## License

Copyright (c) 2015 Adafruit Industries. Licensed under the GPL v3 license.
