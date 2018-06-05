//Include MCP and SPI library
#include <mcp_can.h>
#include <SPI.h>

// Set CS to pin 15, acording to schematics
MCP_CAN CAN0(15);                                     

//Set up timing variables
#define TXPeriod100 100
elapsedMillis TXTimer100;

//Counter to keep track
uint32_t TXCount = 0;

//Set LEDs
#define green_LED 6
#define red_LED 14
boolean LED_state;

void setup()
{
  pinMode(green_LED, OUTPUT);
  pinMode(red_LED, OUTPUT);
  Serial.begin(115200);
  // init can bus, baudrate: 500k
  if(CAN0.begin(CAN_500KBPS) == CAN_OK){
    Serial.print("can init ok!!\r\n");
    digitalWrite(red_LED,HIGH); //Red LED on when succesful
  }
  else {
    Serial.print("Can init fail!!\r\n");
    digitalWrite(red_LED,LOW); //Red LED off when failed
  }
}
//Data to send
byte txmsg[8];     //8 bytes

void loop()
{
 if (TXTimer100>= TXPeriod100){
  TXTimer100 = 0; //Reset timer

  //Convert the 32-bit timestamp into 4 bytes with the most significant byte (MSB) first (Big endian).
    uint32_t sysMicros = micros();
    txmsg[0] = (sysMicros & 0xFF000000) >> 24;
    txmsg[1] = (sysMicros & 0x00FF0000) >> 16;
    txmsg[2] = (sysMicros & 0x0000FF00) >>  8;
    txmsg[3] = (sysMicros & 0x000000FF);

   //Convert the 32-bit transmit counter into 4 bytes with the most significant byte (MSB) first (Big endian). 
    TXCount++;
    txmsg[4] = (TXCount & 0xFF000000) >> 24;
    txmsg[5] = (TXCount & 0x00FF0000) >> 16;
    txmsg[6] = (TXCount & 0x0000FF00) >>  8;
    txmsg[7] = (TXCount & 0x000000FF);
  CAN0.sendMsgBuf(0x00, 0, 8, txmsg);  

  //Toggle LED light as messages are sent
  LED_state = !LED_state;                       
  digitalWrite(green_LED,LED_state);

  //Print on serial for every sent message
    Serial.println("Message Sent");
}
}
