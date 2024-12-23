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
[here](THAThc). It offers an object based interface to the hybrid controller as
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
The shell expects simple commands via the serial line interface and is most
easily accessed using the builtin serial monitor of the Arduino IDE which 
must be set to the proper baud rate (default is 250000 baud) and `No line 
ending`.

To control the analog computer the hybrid controller must first be enabled 
by typing `enable`. This deactivates the MODE switch on the front panel of 
THE ANALOG THING. (`disable` will disable the hybrid controller again.)

Simple manual control can be achieved using the commands `ic` (set initial
condition), `op` (run the analog computer), and `halt` (halt the current 
computation). 

Much more interesting, though, are automatic modes such as single or repetitive
run. Single run performs a single cycle of initial condition (IC) and 
operate (OP) with IC- and OP-times controlled by the microcontroller. The 
IC-time is set with `ictime=1` to one millisecond (this is sufficient if all
integrators within a particular setup are running at their highest time scale
factor), while OP-time is set accordingly with a command like `optime=5` to
five milliseconds.

Issuing a `run` command will perform a single IC/OP-cycle. In this mode it 
is possible to automatically collect data from the analog computer run and 
export it for further processing. The following example session shows how 
this is done:
```
enable
ictime=1
optime=5
arm
run
read
```
The first three commands are self explanatory. `arm` arms the data logger which
will start when the analog computer is switched to OP mode. `run` performs a 
single run with the specified IC- and OP-times. `read` reads the data gathered
by the data logger.

Due to the rather slow analog-digital-converters of the Arduino (things could
be sped up by modifying the corresponding timer/counter settings) there is a 
minimum sample interval of 110 microseconds imposed. For longer OP-times the
sampling interval is automatically set based on the OP-time and the available
memory for the data logger (2048 entries). 

By default only one data channel is sampled (connected to the X-jack on the 
patch panel). If, e.g., four channels (the maximum) are to be sampled this
can be set by `channels=4` before arming the data logger.

`rep` will repeat IC/OP cycles with the times set with `ictime=` and 
`optime=`. This mode is especially useful to produce pictures on an attached
oscilloscope. Data cannot be gathered during repetitive runs. 

`status` returns some basic status information about the hybrid controller
settings, while `help` will display some basic information on the available
commands.

