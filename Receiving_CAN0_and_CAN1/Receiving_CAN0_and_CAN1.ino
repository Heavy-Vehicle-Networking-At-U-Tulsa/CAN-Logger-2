#include <FlexCAN.h>

#ifndef __MK66FX1M0__
#error "Teensy 3.6 with dual CAN bus is required to run this example"
#endif


//Define default baudrate
#define BAUDRATE250K 250000

//Create a counter to keep track of message traffic
uint32_t RXCount0 = 0;
uint32_t RXCount1 = 0;


//Define transmit message from FlexCAN library
static CAN_message_t rxmsg;

//Define CAN TXRX Transmission Silent pins
#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37



bool buttonState = true;

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


void setup() {
  // put your setup code here, to run once:
  
  pinMode(SILENT_0,OUTPUT);
  pinMode(SILENT_1,OUTPUT);
  pinMode(SILENT_2,OUTPUT);
  // Enable transmission for the CAN TXRX
  digitalWrite(SILENT_0,LOW);
  digitalWrite(SILENT_1,LOW);
  digitalWrite(SILENT_2,LOW);

   //Initialize the CAN channels
  Can0.begin(BAUDRATE250K);
  Can1.begin(BAUDRATE250K);

  //The default filters exclude the extended IDs, so we have to set up CAN filters to allow those to pass.
  CAN_filter_t allPassFilter;
  allPassFilter.ext=1;
  for (uint8_t filterNum = 8; filterNum < 16;filterNum++){ //only use half the available filters for the extended IDs
    Can0.setFilter(allPassFilter,filterNum); 
    Can1.setFilter(allPassFilter,filterNum); 
  }
}

void loop() {
  // put your main code here, to run repeatedly:
 
  
  while (Can0.available()) {
    Can0.read(rxmsg);
    printFrame(rxmsg,0,RXCount0++);
  }
  while (Can1.available()) {
    Can1.read(rxmsg);
    printFrame(rxmsg,1,RXCount1++);
  }

}