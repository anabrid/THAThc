# THAThc
This repository contains an Arduino library as well as build instructions for
A simple Arduino based hybrid controller for THE ANALOG THING.

## Overview
[THE ANALOG THING](https://github.com/anabrid/THAThc) is a small analog 
computer for educational purposes, featuring five integrators, four summers,
four inverters, two multipliers, two comparators, and eight coefficient
potentiometers. It also has a HYBRID port which can be used to connect it 
to a microcontroller such an Arduino Mega 2560 as shown in the following:

![HCsetup.jpg](HCsetup.jpg)

To facilitate the use of such a setup a simple Arduino library is provided
which allows to control the analog computer either by an interactive shell 
or under full program control of the microcontroller.

## Hardware
The hybrid port of THE ANALOG THING allows full control over the analog
computer's modes of operation as well as readout of up to four analog 
channels available at the front panel at the X-, Y-, Z-, and U- connections.

Only eight connections must be wired between the Mega 2560 and THE ANALOG
THING as shown in the following table:

|HYBRID port pin|description|Pin on Mega 2560|
|---------------|-----------|----------------|
|2|analog X output|AnalogIn 0|
|4|analog Y output|AnalogIn 1|
|6|analog Z output|AnalogIn 2|
|8|analog U output|AnalogIn 3|
|9 and 10|GND|GND|
|13|enable hybrid mode|D2|
|14|Mode OP|D3|
|17|Mode IC|D4|

These connections were made on a piggy back board as shown in the picture above.
The analog output lines are level compatible due to level shifters on THE
ANALOG THING, so they can be connected directly to the microcontroller.

## Software
The Arduino library for the hybrid controller can be found 
[here]{THAThc}. It offers an object based interface to the hybrid controller as
shown in the following code example:
```
#include "THAThc.h"
#define BAUD_RATE             250000

void setup() {
  Serial.begin(BAUD_RATE);
  THAThc.begin();
}

void loop() {
  THAThc.shell();
}
```

This example assumes that the library `.h`- and `.cpp`- files reside in the 
same directory as the actual application program. If the library files are 
moved to the appropriate folder in the Arduino environment, the first line
should read `#include <THAThc>`. The call to `THAThc.begin()` in `setup()` 
instantiates a hybrid controller singleton that can then be used in the 
following.

The method `shell()` (called in `loop()`) provides an interactive shell to 
control the analog computer.

### Using the shell

