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
3. Temperature sensor - 1-wire DS18B20 temperature sensor support connected to the 1-wire bus

### Supported hardware

1. Arduino Mega (with Ethernet shield) - 14 shades or 28 lights
2. Controllino Mega - 10 shades or 21 lights
3. Controllino Maxi - 6 shades or 12 lights

Feel free to contribute code to support a desired HW platform. For details regarding implementing new platforms see HW-SUPPORT.md.

### Libraries required

1. Ethernet2 (1.0.4)
2. SD (1.2.4)
3. DallasTemperature (3.9.0)
4. OneWire (2.3.5)
5. CONTROLLINO (3.0.5)


### Pin to devID mapping

The software creates devices objects which are referenced by a devID identifier. A device can be a single light, shade or temperature sensor. It has associated certain physical pins where it is connected. For example a device of type light will have two pins, one for to connect a switch (input pin) and second to connect a relay (output pin). 

The following list provides pin to devID mapping:

Controllino Mega (lights mode):
Input: A0  - Output D0  - devID: 1
Input: A1  - Output D1  - devID: 2
Input: A2  - Output D2  - devID: 3
Input: A3  - Output D3  - devID: 4
Input: A4  - Output D4  - devID: 5
Input: A5  - Output D5  - devID: 6
Input: A6  - Output D6  - devID: 7
Input: A7  - Output D7  - devID: 8
Input: A8  - Output D8  - devID: 9
Input: A9  - Output D9  - devID: 10
Input: A10 - Output D10 - devID: 11
Input: A11 - Output D11 - devID: 12
Input: A12 - Output D12 - devID: 13
Input: A13 - Output D13 - devID: 14
Input: A14 - Output D14 - devID: 15
Input: I15 - Output D15 - devID: 16
Input: I16 - Output D16 - devID: 17
Input: I17 - Output D17 - devID: 18
Input: I18 - Output D18 - devID: 19
Input: I0  - Output D19 - devID: 20
Input: I1  - Output D20 - devID: 21
		   - Output R0  - devID: 22
		   - Output R1  - devID: 23
		   - Output R2  - devID: 24
		   - Output R3  - devID: 25
		   - Output R4  - devID: 26
		   - Output R5  - devID: 27
		   - Output R6  - devID: 28
		   - Output R7  - devID: 29
		   - Output R8  - devID: 30
		   - Output R9  - devID: 31
		   - Output R10 - devID: 32
		   - Output R11 - devID: 33
		   - Output R12 - devID: 34
		   - Output R13 - devID: 35
		   - Output R14 - devID: 36
		   - Output R15 - devID: 37
		   
Controllino Mega (shades mode):
Input: A0  - Output D0  - devID: 1
Input: A1  - Output D1  - devID: 1
Input: A2  - Output D2  - devID: 2
Input: A3  - Output D3  - devID: 2
Input: A4  - Output D4  - devID: 3
Input: A5  - Output D5  - devID: 3
Input: A6  - Output D6  - devID: 4
Input: A7  - Output D7  - devID: 4
Input: A8  - Output D8  - devID: 5
Input: A9  - Output D9  - devID: 5
Input: A10 - Output D10 - devID: 6
Input: A11 - Output D11 - devID: 6
Input: A12 - Output D12 - devID: 7
Input: A13 - Output D13 - devID: 7
Input: A14 - Output D14 - devID: 8
Input: I15 - Output D15 - devID: 8
Input: I16 - Output D16 - devID: 9
Input: I17 - Output D17 - devID: 9
Input: I18 - Output D18 - devID: 10
Input: I0  - Output D19 - devID: 10



Controllino Maxi (lights mode):
Input: A0  - Output D0  - devID: 1
Input: A1  - Output D1  - devID: 2
Input: A2  - Output D2  - devID: 3
Input: A3  - Output D3  - devID: 4
Input: A4  - Output D4  - devID: 5
Input: A5  - Output D5  - devID: 6
Input: A6  - Output D6  - devID: 7
Input: A7  - Output D7  - devID: 8
Input: A8  - Output D8  - devID: 9
Input: A9  - Output D9  - devID: 10
Input: I0  - Output D19 - devID: 11
Input: I1  - Output D20 - devID: 12
		   - Output R0  - devID: 13
		   - Output R1  - devID: 14
		   - Output R2  - devID: 15
		   - Output R3  - devID: 16
		   - Output R4  - devID: 17
		   - Output R5  - devID: 18
		   - Output R6  - devID: 19
		   - Output R7  - devID: 20
		   - Output R8  - devID: 21
		   - Output R9  - devID: 22


## Licensing

This software is licensed under GPL.
