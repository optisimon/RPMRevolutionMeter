# RPMRevolutionMeter

Application measuring rotation speed using minimal external hardware.


## Description

This linux application is typically used together with a reflex detector
connected to the microphone input of a computer.

It's possible to create a cable for that without needing external power.

Microphone inputs on computers and laptops typically 
provide a small and current limited voltage to the mic connector.
The laptops I've tested provides between 3.3 and 5V to the RING of the
connector.

And the circuit below have worked with all three laptops I've had at hand,
and will provide an audio signal back over the mic input showing what's
happening in front of the reflex detector. Note that all mic inputs are
AC coupled, which makes them ideal for measuring non-stationary signals.

I managed to fit a reflex detector and the resistors inside a small
highlighting marker, making this hardware/software combo really neat.


```
 _________________  __
 3.5 mm mic  |    \/  \
 plug        |    ||   )-------+
 ____________|____/\__/        |
  |  SLEVE    RING  TIP        |
  |            |      _____    |
  |            |-----|_____|---+
  |            |     10 kOhm   |
  |           | |              |
  |   330 Ohm | |             | |
  |           |_|             | | 1 kOhm
  |            |              |_|
  |           _|_              |
  |           \ /   --->    | /
  |           _V_   --->    |(    Photo transistor (NPN)
  |            |  LED       |_\/
  |            |               |
  |____________|_______________|
```

## Screenshot

![Screenshot](/../screenshots/screenshots/screenshot.png?raw=true "Screenshot")


## Controls available in the SDL window

```
+/-   change size of RPM digits
f     add some filtering of displayed RPM values
F11   go full screen
UP    increase threshold percentage
DOWN  decrease threshold percentage
ESC   quit the program
```


## Command line options

Most people would typically only use `./RPMRevolutionMeter -m`.

```
-v, --verbose
-b, --blind             Do not use the gui (text only output)
-m, --mic               Use mic directly
-D, --device            Specify alsa device (default hw:0,0)
-d, --rpm_divisor       Number to divide pulse frequency with
-a, --amplitude Number  minimum input waveform amplitude required before counting revolutions
-h, --help
-f, --file FILENAME.WAV (Analyzing a pre-recorded file)
```

## Text output
Besides some initial text (subject to change without notice), this application
prints the start time/date of the aquisition.
Those lines are followed by one line for each revolution, consisting of a timestamp, RPM (when looking at a single period), as well as a filtered rpm value (average of the last X periods).

```
date=2016-11-06 00:29:47
epoch=1478388587
ts=0.0, rpm=1706, rpm_filtered=1706
ts=0.59, rpm=937.965, rpm_filtered=1321.98
ts=0.129, rpm=963.934, rpm_filtered=1202.63
ts=0.189, rpm=951.799, rpm_filtered=1139.92
```

## Compile and install (ubuntu 14.04)
This application is currently only verified to on ubuntu 14.04 and 16.04.

```
# Check out the source code
sudo apt-get install git
git clone https://github.com/optisimon/RPMRevolutionMeter.git
cd RPMRevolutionMeter

# Install dependencies
sudo make prepare

# Compile it
make

# Install it
sudo make install
```


## Uninstall

```
# uninstall it
sudo make uninstall
```
