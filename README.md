# CAN-Logger-2
Source code and documentation for the CAN Logger 2

## Purpose: 
Use as an educational tool to interface with CAN, including dual CAN channels, MCP2515 third CAN, Single Wire CAN, LIN, J1708, WIFI, LEDs, and pushed button

## Logger Device Test Programs
### LED with button: 
Test codes used to turn on three LEDs by pushing the button

### Dual CAN channels 
Test codes used to send/receive CAN message periodically on both channels CAN0 and CAN1 using FLEX CAN library. The library can be accessed here: https://github.com/Heavy-Vehicle-Networking-At-U-Tulsa/FlexCAN_Library

### MCP2515 
test codes used to send/receive CAN message periodically through third CAN channel using MCP library. The library can be accessed here: https://github.com/Heavy-Vehicle-Networking-At-U-Tulsa/NMFTA-CAN-Logger/tree/master/Arduino_Libraries_to_Install/MCP2515

### WiFi 
Test codes used to wireless communicate through WiFi between the WiFi chip on the logger with one another. The WiFi 101 library can be accessed here: https://github.com/adafruit/Adafruit_WINC1500. To correctly use the WiFi chip on the Logger 2, SPI1 has to be selected by: Adafruit_WINC1500-master>src>bus_wrapper>source>nm_bus_wrapper_samd221.cpp, change line 54 to #define WINC1501_SPI SPI1.

### Flood the BUS with button
Test code to send messages with top priority messages at maximum speed to flood the bus as the button is pressed.

## Acknowledgements 
This material is based upon work supported by the National Science Foundation under Grant No. 1715409.
