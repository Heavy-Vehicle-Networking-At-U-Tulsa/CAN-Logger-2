//Include MCP and SPI library
#include <mcp_can.h>

// Set CS to pin 15, acording to schematics
#define CS_CAN 15
MCP_CAN CAN2(CS_CAN);                                     

//Data to send
byte txmsg[8];     //8 bytes

//Set up timing variables
#define TXPeriod2 100
elapsedMillis TXTimer2;

//Counter to keep track
uint32_t TXCount = 0;

//Set LEDs
#define green_LED 6
#define red_LED 14
boolean LED_state;

void setup()
{
  pinMode(CS_CAN, OUTPUT);
  digitalWrite(CS_CAN,HIGH);
  pinMode(green_LED, OUTPUT);
  digitalWrite(green_LED,HIGH);
  pinMode(red_LED, OUTPUT);
  
  Serial.write("Starting CAN Send Test.");
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN2.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");
  
  CAN2.enOneShotTX();
  CAN2.setMode(MCP_NORMAL);
}


void loop()
{
  if (TXTimer2 >= TXPeriod2){
    TXTimer2 = 0; //Reset timer

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

    //Send messaeg in format: ID, Standard (0) or Extended ID (1), message length, txmsg
    CAN2.sendMsgBuf(0x18FEF100, 1, 8, txmsg);  

    //Toggle LED light as messages are sent
    LED_state = !LED_state;                       
    digitalWrite(green_LED,LED_state);

    //Print on serial for every sent message
    Serial.print("Message Sent: ");
    Serial.println(TXCount);
  }
}
