# HummelRummel usermod

This usermod enables the HummelRummel feature set, allowing the WLED system to be used in interactive toys.

## Installation 

To install this Usermod, you instruct PlatformIO to compile the Project with the USERMOD_HUMMELRUMMEL flag.
WLED then automatically provides you with various settings on the Usermod Page.

## Features

- Replacing the default button handling, to prevent unwanted default behaviour, like WIFI reset, factory reset, turn on/off
- Instrument mode (currently called Guitare mode), triggers note on and off on keypress and the GuitareEffect plays the notes