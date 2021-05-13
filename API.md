# ARiF Protocol Documentation #

ARiF is HTTP REST-like API that allows to control remotely devices connected to iot-hub-c running device. It currently supports two types of devices:
* Light
* Shade

And generic iot-hub-c level of settings (like the registration capability)

Each ARiF call is composed of the specific method (POST only currently) and URL. The URL needs to be written according to the following scheme (mandatory parameters only):
`/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=<command>`

Where `<devid>` is the identifier of one of the iot-hub-c'a devices represented by certain pins. `<ardid>` is the identifier of the entire iot-hub-c that is givien to it during registration. `<raspyID>` is the identifier of the Raspberry that performed the iot-hub-c registration. `<command>` represents the command sent that defines the type of operations

Additionally the following parameters can be added:
- `devType` - indicates the type of the device being subject to the command (e. g. `devType=shade` - it is rather used in the iot-hub-c -> Raspberry direction)
- `dataType` - indicates the type of data that is being transported in the `value` parameter (e. g. `dataType=bool`)
- `value` - value that is associated with certain command (e. g. can indicate desired shade position)

**Light Device**
----

* **switch on/off**
This command is used to set light ON or OFF.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=lightON`  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=lightOFF`  
* **state change report**
Command sent by VelenHub to indicate state change of a light device.  
Direction: out  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=status&devType=digitOUT&dataType=bool&value=<value>`  
Where:
  - `<value>` indicates the new state of the light device `0` if the light goes off and `1` if it goes on  
* **light timer**
Used to set the light timer. When the light is turned on the timer is enabled and after it passes the light device is turned off. The light device type must be first set using the `lightType` command.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=lightTimer&value=<value>`  
Where:
  - `<value>` indicates the timer value. The value of  timer is in milliseconds and the accepted range is 1 - 4 294 967 295  
* **light type**
Command used to set the type of the light device identified by `<devid>`.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=lightType&value=<value>`  
Where:
  - `<value>` indicates the type of the light device. The argument can take the following values:
    - `0` - the regular on/off light device
    - `1` - a timer based light device
* **light input type**
Command used to set the type of the light's input type. This setting is used to tell Arduino how the physical light switch will behave. It can either return to its normal position after pressing (push button) or it can stay in the position which he was switched to (toggle switch).
Direction: in
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=<cmd>`
Where:
  - `<cmd>` indicates the type of the light's input. The following two commands are used:
    - `inputHold` - the toggle switch
    - `inputRelease` - the push button

**Shade Device**
----


* **shade direction**
This set of commands is used to control the shade movment. `shadeUP` and `shadeDOWN` start the movment in the indicated direction. The movement will stop once the shade is fully open or fully closed respectively. `shadeSTOP` stops the movement.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadeUP`  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadeDOWN`  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadeSTOP`  
* **shade position:**
This command is used to place the shade into a desired position (one of the five predefined) irrespective of the current starting point of the shade.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadePOS&value=<value>`  
Where:
  - `<value>` indicates the position of the shade. It can take the following values:
    - `0`   - Shade is fully open 
    - `25`  - Shade is 25% closed
    - `50`  - Shade is half open
    - `75`  - Shade is 75% closed
    - `100` - Shade is fully closed
* **shade tilt**
This command is used to place the shade tilt into one of three predefined positions (see below). This command will be accepted irrespective if the shade is moving or not. If it is moving the new tilt will be set once the movement ends. When the shade is in fully open position (see *shade position* command) the tilt move will not happen, however the tilt value will be recorded and will be set once the shade lands in another position.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadeTILT&value=<value>`  
Where:
  - `<value>` indicates the position of the shade tilt. It can take the following values:
    - `0` - Tilt is shut
    - `45` - Tilt is half open
    - `90` - tilt is open

* **shade position timer** *(not implemented)*
This command is used to set the timer value of the shade. It specifies how many seconds it takes for the shade to reach from fully open to fully closed positions.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadePTimer&value=<value>`  
Where:
  - `<value>` indicates the position timer of the shade. Timer value can be set in range 10 - 65535 in unit of seconds.
* **shade tilt timer** *(not implemented)*
This command is used to set the tilt timer value of the shade. It specifies how many miliseconds it takes for the shade tilt move to reach from fully open to fully closed.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadeTTimer&value=<value>`  
Where:
  - `<value>` indicates the position timer of the shade tilt. Timer value can be set in range 500 - 10000 in unit of miliseconds.
* **set shade type** *(not implemented)*
This command is used to set the type of the shade device indicated by `<devid>`.  
Direction: in  
URL: `/?devID=<devid>&ardID=<ardid>&raspyID=<raspyid>&cmd=shadeType&value=<value>`  
Where:
  - `<value>` indicates the shade type. Type can be one of the following:
    - `0` - Tiltable shade device
    - `1` - shade device without tilt option (like garage door or cover)

**Generic Messages**
----

* **registration**
This command is used to register the VelenHub in a Raspy (VelenGW). It is sent by the VelenGW and the `<devid>` is set always to `0`. VelenHub will read the `<ardid>` value and treat it as its own ArdID. VelenHub will also save the `<raspyid>`.  
Direction: in  
URL: `/?devID=0&ardID=<ardid>&raspyID=<raspyid>&cmd=register`  
* **heartbeat**
This command is used to send heartbeats by the VelenHub.  
Direction: in  
URL: `/?devID=0&ardID=<ardid>&raspyID=<raspyid>&cmd=heartbeat`  
* **Central On/Off** 
Used for enabling or disabling central control by the last input pin (last devID). The pin serves a normal function but apart from that if it is enabled it turns on all outputs of all other devIDs (unless their type is set to *timer*). Works only in for light type devices.  
Direction: in  
URL: `/?devID=0&ardID=<ardid>&raspyID=<raspyid>&cmd=ctrlON`  
URL: `/?devID=0&ardID=<ardid>&raspyID=<raspyid>&cmd=ctrlOFF`  

* **VelenHub mode**
This command can be used to change the mode of VelenHub between Lights and Shades  
Direction: in  
URL: `/?devID=0&ardID=<ardid>&raspyID=<raspyid>&cmd=modeLights`  
URL: `/?devID=0&ardID=<ardid>&raspyID=<raspyid>&cmd=modeShades`  
