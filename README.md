# CAN-Logger-2
Source code and documentation for the CAN Logger 2

## Purpose: 
Use as an educational tool to interface with CAN, including dual CAN channels, MCP2515 third CAN, Single Wire CAN, LIN, J1708, WIFI, LEDs, and pushed button

## Submodules
The CAN Logger 2 depends on the following libraries:
  * FlexCAN_Library forked from Collin80: https://github.com/Heavy-Vehicle-Networking-At-U-Tulsa/FlexCAN_Library
  * MCP_CAN_lib forked from Cory Fowler: https://github.com/Heavy-Vehicle-Networking-At-U-Tulsa/MCP_CAN_lib
  * Arduino's Wifi101 library: https://github.com/arduino-libraries/WiFi101
  
To use these submodules, you have to open a Git Bash command perform an update. Assuming Windows with Git installed:

```cd Documents/GitHub/CAN-Logger-2/``` (or equivalent directory for this repository.

```git submodule update --init```

### Installing Arduino Libraries
In Arduino 1.8.5, Click Sketch -> Include Library -> Add .Zip Library

Navigate to the submodule directory within this repository and select the name of the directory. A message should show up in Arduino saying "Library added to your libraries."

If there is an error in importing the libraries, you may need to remove exising libraries (see below).

#### Removing Existing Libraries 
Once the modules are updated, you need to make them available to your Arduino environment. Again, assuming Windows, open Git Bash.

```cd ~/Documents/Arduino/libraries```

``` rm -r FlexCAN_Library``` or other libraries.

You may also need to remove the pre-installed FlexCAN_Library:

```cd C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries```

``` rm -r FlexCAN_Library``` or other libraries.


## Usage
The following charts show the functionality of the CAN Logger 2 units. These charts are available as a pdf and include program flow and data structures.

[NMFTA TU CAN Logger 2 Diagrams](docs/NMFTA_TU_CAN_Logger_2%20Block%20Diagrams.pdf)

## Logger Device Test Programs
The test programs are in the `tests` directory

### `Test_LED_with_Button` 
Test code used to turn on three LEDs by pushing the button. The sketch makes use of the follwing defines based on the circuit design:

```
#define GREEN_LED_PIN 6
#define RED_LED_PIN 14
#define YELLOW_LED_PIN 5
#define button 21
```

### `Test_CAN0_and_CAN1`
Test codes used to send/receive CAN message periodically on both channels CAN0 and CAN1 using FLEX CAN library. 

This makes use of the following defines for the Silent Pins on the transceivers:
```
#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37
```
Pull the silent pins low for transmission.


### `Test_MCP2515_Receive and Test_MCP2515_Send`
Test code used to send/receive CAN message periodically through third CAN channel using MCP library. 

## Utilities
There are a few utilities useful when using the CAN Logger 2.

### Flood the BUS with button
Test code to send messages with top priority messages at maximum speed to flood the bus as the button is pressed.

## WiFi
The CAN Logger 2 has a the ATWINC1500 Wifi module. By default, this module is disabled (i.e. not wired correctly.) If the wiring is fixed, then the Wifi can be used. 
Test codes used to wireless communicate through WiFi between the WiFi chip on the logger with one another. The WiFi 101 library can be accessed here: https://github.com/adafruit/Adafruit_WINC1500. To correctly use the WiFi chip on the Logger 2, SPI1 has to be selected by: Adafruit_WINC1500-master>src>bus_wrapper>source>nm_bus_wrapper_samd221.cpp, change line 54 to #define WINC1501_SPI SPI1.


## Acknowledgements 
This material is based upon work supported by the National Science Foundation under Grant No. 1715409.
