# iot-hub-c

IoT-hub-c is designed to provide a univeral home and industry automation and remote control capabilities. It enables remote control of output pins and reporting on input pin state changes. REST API allows to integrate it with other systems.

## Getting started

To install the software on a support hardware device follow these steps:

1. Download latest release
2. Unpack it and open in Arduino SDK
3. Upload to the device
4. (optional) if using device that has SD Card slot copy files from webui directory into a FAT32 or FAT16 formatted SD Card and insert the card into the slot.

## Features

The following features are currently supported:

1. Light type device - One light type device is composed of one input pin (where the physical switch is suppposed to be connected) and one output pin (where the reley control circuit is supposed to be connected.
2. Shades type device - One shade type device is composed of two input pins (where the physical switches are supposed to be connected for controlling of the shade in both directions) and two output pins (for controlling shades motor in both directions - it supposed to be connected to two relay circuits)

### Supported hardware

1. Arduino Mega (with Ethernet shield) - 14 shades or 28 lights
2. Controllino Mega - 10 shades or 21 lights
3. Controllino Maxi - 6 shades or 12 lights

Feel free to contribute code to support a desired HW platform. For details regarding implementing new platforms see HW-SUPPORT.md.

## Licensing

This software is licensed under GPL.
