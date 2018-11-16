//Include MCP Library
#include <mcp_can.h>
#include <mcp_CAN_dfs.h>
#include <FlexCAN.h>

//Define transmit message from FlexCAN library
static CAN_message_t txmsg, rxmsg;
uint32_t TXCount, RXCount;

//Define CAN TXRX Transmission Silent pins
#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37

//Set up timing variables
#define TXPeriod0 100
elapsedMillis TXTimer0;

//Set LEDs
#define green_LED 6
#define red_LED 14
#define yellow_LED 5
boolean green_LED_state;
boolean red_LED_state;
boolean yellow_LED_state;

uint32_t rxId;
uint8_t len;
uint8_t rxBuf[8];
uint8_t ext_flag;

// Set CS to pin 15, according to schematics
#define CS_CAN 15
MCP_CAN CAN2(CS_CAN);  

//Set the pin for the CAN Logger 2 to switch between J1708 and CAN 2 on the 9-pin
#define CAN2_J1708_SWITCH 2

//Define LED
#define GREEN_LED_PIN 6
#define RED_LED_PIN 14
#define YELLOW_LED_PIN 5

boolean GREEN_LED_state; 
boolean RED_LED_state;
boolean YELLOW_LED_state;

void setup()
{
  pinMode(CS_CAN, OUTPUT);
  digitalWrite(CS_CAN,HIGH);
  
  pinMode(CAN2_J1708_SWITCH,OUTPUT);
  digitalWrite(CAN2_J1708_SWITCH,LOW);
  
  pinMode(SILENT_0,OUTPUT);
  pinMode(SILENT_1,OUTPUT);
  pinMode(SILENT_2,OUTPUT);
  // Enable transmission for the CAN TXRX
  digitalWrite(SILENT_0,LOW); //Change these values to ge different errors
  digitalWrite(SILENT_1,LOW);
  digitalWrite(SILENT_2,LOW); 

  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);
  pinMode(YELLOW_LED_PIN,OUTPUT);
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN2.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN2.setMode(MCP_NORMAL);
  Serial.println("MCP2515 Initialized");

  Can0.begin(250000);
  Can1.begin(250000);

  txmsg.ext = 1;
  txmsg.id=0x18FEF250;
  txmsg.len=8;
  
  CAN_filter_t allPassFilter;
  allPassFilter.ext=1;
  for (uint8_t filterNum = 8; filterNum < 16;filterNum++){ //only use half the available filters for the extended IDs
    Can0.setFilter(allPassFilter,filterNum); 
    Can1.setFilter(allPassFilter,filterNum); 
  }
}


void loop()
{
  if (CAN2.readMsgBuf(&rxId,&ext_flag, &len, rxBuf) == CAN_OK){             // Read data: len = data length, buf = data byte(s)
    Serial.print("ID: ");
    Serial.print(rxId, HEX);
      Serial.print("  Data: ");
      for(int i = 0; i<len; i++)                // Print each byte of the data
      {
        if(rxBuf[i] < 0x10)                     // If data byte is less than 0x10, add a leading zero
        {
          Serial.print("0");
        }
        Serial.print(rxBuf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    GREEN_LED_state = !GREEN_LED_state;
    digitalWrite(GREEN_LED_PIN,GREEN_LED_state);                              
  }
  if (TXTimer0 >= TXPeriod0){
    TXTimer0 = 0; //Reset timer

    //Convert the 32-bit timestamp into 4 bytes with the most significant byte (MSB) first (Big endian).
    uint32_t sysMicros = micros();
    txmsg.buf[0] = (sysMicros & 0xFF000000) >> 24;
    txmsg.buf[1] = (sysMicros & 0x00FF0000) >> 16;
    txmsg.buf[2] = (sysMicros & 0x0000FF00) >>  8;
    txmsg.buf[3] = (sysMicros & 0x000000FF);

    //Convert the 32-bit transmit counter into 4 bytes with the most significant byte (MSB) first (Big endian). 
    txmsg.buf[4] = (TXCount & 0xFF000000) >> 24;
    txmsg.buf[5] = (TXCount & 0x00FF0000) >> 16;
    txmsg.buf[6] = (TXCount & 0x0000FF00) >>  8;
    txmsg.buf[7] = (TXCount & 0x000000FF);

    //Send messaeg in format: ID, Standard (0) or Extended ID (1), message length, txmsg
    Can0.write(txmsg); 

    //Toggle LED light as messages are sent
    green_LED_state = !green_LED_state;                       
    digitalWrite(green_LED,green_LED_state);

    //Print on serial for every sent message
    Serial.print("Message Sent: ");
    Serial.println(TXCount);
    TXCount++;
    //Toggle the LED
    RED_LED_state = !RED_LED_state;
    digitalWrite(RED_LED_PIN,RED_LED_state);
    
  }

  if (Can1.available()) {
    Can1.read(rxmsg);
    YELLOW_LED_state = !YELLOW_LED_state;
    digitalWrite(YELLOW_LED_PIN,YELLOW_LED_state);
  }
}

