# 4BY4

4BY4 is a prototype MIDI / OSC / WebSockets controller. It does one key thing differently to other controllers: when changing between "pages" of assignments, values are locked (ie. do not send) until the position of the knob returns or passes the previously recorded position. This is so that assignments don't "jump" when you go to tweak them. To indicate this, an LCD is used to display the locked value alongside the knob position, and is highlighted in red until the knob position passes the locked value. 

**Pots vs Encoders**

The design can be thought of as a low-cost alternative to using rotary encoders, such as on controllers like the MIDI Fighter Twister. Rotary encoders have the benefit of no fixed start or end point, but the downside of having less fidelity than potentiometers (unless very expensive), as well as lacking the tactile feel of an analogue potentiometer. A single rotary encoder is used in the prototype for switching between pages, but could also be replaced by push buttons, a touchscreen etc.

**Sending Messages**

When connected via USB, the 4BY4 doesn't immediately send MIDI / OSC / WebSockets, and insteads sends serial messages directly over USB from the device, with 1024 points of fidelity. The ([bridge.js](bridge.js)) script will forward those messages onto MIDI, OSC or WebSockets. It's possible to load MIDI firmware onto the Arduino and skip this process, but will mean 128 points of fidelity only.

**Open Source**

Everything here is open sourced under MIT, circa August 2021. Currently there's nothing like this "on the market", so I'm getting this out there before anyone can dib it. 

# Parts

* 16 x 1K potentiometers
* 1 x MUX74HC4067
* 1 x 1.8" TFT
* 1 x rotary encoder
* 1 x Arduino Uno

# Usage

**Configuration**

Configuration for CLI compiler and bridge script is handled by [config.env](config.env) like so:

```
BOARD_TYPE=arduino:avr:uno
# use arduino-cli board list to find BOARD_TYPE

BOARD_PORT=/dev/cu.usbmodem1D131
# use ls /dev/tty* to find BOARD_PORT

OSC_ADDRESS=0.0.0.0
# the IP of the computer your OSC messages will be sent to
# 0.0.0.0 is localhost

OSC_PORT=4444
```

**Arduino**

[4BY4.ino](4BY4.ino) can be opened in the Arduino IDE and uploaded, or if you have [arduino-cli](https://github.com/arduino/arduino-cli) installed run it with `./compile.sh` (after making sure to set `BOARD` and `PORT` have been set correctly in ENV).

**Bridge**

Once it is uploaded to the Arduino, you then run the bridge script which will forward serial messages into MIDI, OSC or WebSockets. This is done with `pnpm start`.

# Wiring

```
=======================
POTENTIOMETERS x16 (1K)
=======================

MIDDLE PINS = MULTIPLEXER C0-C15
LEFT PINS = 5V
RIGHT PINS = GND

=========================
MULTIPLEXER (MUX74HC4067)
=========================

C0-C15 = Potentiometer Center Pins
S0 = Arduino D3
S1 = Arduino D4
S2 = Arduino D5
S3 = Arduino D6
EN = Arduino D7
SIG = Arduino A0

========
TFT 1.8"
========

SCK/CLOCK = Arduino D13
SDA/MOSI = Arduino D11
CS/SS/NSS/SELECT = Arduino D10
A0/DC = Arduino D9
RST/RESET = Arduino RESET

==============
ROTARY ENCODER
==============

CLK = D12
DT = 8
SW = N/A

```

# Future

* make a KiCad design with through-hole potentiometers
* use better quality potentiometers, and correct resistor values
* switch MIDI / OSC / WebSockets script into micro-gui panel

