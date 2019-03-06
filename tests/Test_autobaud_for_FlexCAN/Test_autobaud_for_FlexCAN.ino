/*
 * Example file to test the autobaud function of the Teensy 3.x 
 * uing FlexCAN. The list of baud rates can be modified in the defines of the FlexCAN.h
 * 
 * The last known good value of the CAN bitrate is stored in EEPROM. The address is also 
 * in the defines section of the FlexCAN.h
 * 
 */
#include <FlexCAN.h>

//Create a counter to keep track of message traffic
uint32_t RXCount0 = 0;
uint32_t RXCount1 = 0;

//Define message structure from FlexCAN library
static CAN_message_t rxmsg;

boolean LED_state;

//A generic CAN Frame print function for the Serial terminal
void printFrame(CAN_message_t rxmsg, uint8_t channel, uint32_t RXCount)
{
  char CANdataDisplay[50];
  sprintf(CANdataDisplay, "%d %12lu %12lu %08X %d %d", channel, RXCount, micros(), rxmsg.id, rxmsg.ext, rxmsg.len);
  Serial.print(CANdataDisplay);
  for (uint8_t i = 0; i < rxmsg.len; i++) {
    char CANBytes[4];
    sprintf(CANBytes, " %02X", rxmsg.buf[i]);
    Serial.print(CANBytes);
  }
  Serial.println();
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  LED_state = true;
  digitalWrite(LED_BUILTIN, LED_state);
  
  while(!Serial);
  Serial.println("Starting CAN Autobaud Test.");
  
  //Initialize the CAN channels with autobaud
  Can0.begin(0);
//  // Uncomment for Teensy 3.6
//  Can1.begin(0);
}


void loop()
{
  while (Can0.read(rxmsg)) {
    printFrame(rxmsg,0,RXCount0++);
    LED_state = !LED_state;
    digitalWrite(LED_BUILTIN, LED_state);
  }
//  // Uncomment for Teensy 3.6
//  while (Can1.read(rxmsg)) {
//    printFrame(rxmsg,1,RXCount1++);
//    LED_state = !LED_state;
//    digitalWrite(LED_BUILTIN, LED_state);
//   }
}
