//Include MCP and SPI library
#include <mcp_can.h>
#include <Bounce2.h>
#include <FlexCAN.h>


#define BUTTON_PIN 21

// Instantiate a Bounce object
Bounce debouncer = Bounce(); 
uint8_t rate_index = 0;
uint8_t current_baud_rate = CAN_250KBPS;
#define num_rates 4
uint8_t baudrate_list[num_rates] = {CAN_250KBPS, CAN_500KBPS, CAN_125KBPS, CAN_666KBPS};
uint32_t TX_ID_list[num_rates] = {0x250, 0x500, 0x125, 0x666};
uint32_t TX_ID = 0x250;

// Set CS to pin 15, acording to schematics
#define CS_CAN 15
MCP_CAN CAN0(CS_CAN);                                     

//Data to send
byte txmsg[8];     //8 bytes

//Set up timing variables
#define TXPeriod10 100
elapsedMillis TXTimer10;

//Counter to keep track
uint32_t TXCount = 0;

//Set LEDs
#define green_LED 6
#define red_LED 14
#define yellow_LED 5
boolean green_LED_state;
boolean red_LED_state;
boolean yellow_LED_state;

//Define message from FlexCAN library
static CAN_message_t txmsg0;
static CAN_message_t txmsg1;
static CAN_message_t rxmsg0;
static CAN_message_t rxmsg1;

//Set up timing variables (Use prime numbers so they don't overlap)
#define TXPeriod0 149
elapsedMillis TXTimer0;

#define TXPeriod1 89
elapsedMillis TXTimer1;


//Create a counter to keep track of message traffic
uint32_t TXCount0 = 0;
uint32_t TXCount1 = 0;
uint32_t RXCount0 = 0;
uint32_t RXCount1 = 0;

//Define LED
#define GREEN_LED_PIN 6
#define RED_LED_PIN 14
#define YELLOW_LED_PIN 5

boolean GREEN_LED_state; 
boolean RED_LED_state;
boolean YELLOW_LED_state;

//Define CAN TXRX Transmission Silent pins
#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37

//Define default baudrate
#define BAUDRATE125K 125000
#define BAUDRATE250K 250000
#define BAUDRATE666K 666666
#define BAUDRATE500K 500000

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

void change_baud(){
  rate_index++;
  if (rate_index >= num_rates) rate_index = 0;
  current_baud_rate = baudrate_list[rate_index];
  TX_ID = TX_ID_list[rate_index];
  setup_CAN();
  digitalWrite(yellow_LED,(current_baud_rate & 0x02) >> 1);
  digitalWrite(red_LED,current_baud_rate & 0x01);
  
}

void setup_CAN(){
  if(!CAN0.begin(MCP_ANY, current_baud_rate, MCP_16MHZ) == CAN_OK){
    Serial.println("Error Initializing MCP2515...");
  }
  CAN0.enOneShotTX();
  CAN0.setMode(MCP_NORMAL);
}

void setup()
{
  pinMode(CS_CAN, OUTPUT);
  digitalWrite(CS_CAN,HIGH);
  pinMode(green_LED, OUTPUT);
  digitalWrite(green_LED,HIGH);
  pinMode(yellow_LED, OUTPUT);
  pinMode(red_LED, OUTPUT);
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  //while(!Serial);
  Serial.println("Starting CAN Send Test.");

  rate_index = 1;
  current_baud_rate = baudrate_list[rate_index];
  TX_ID = TX_ID_list[rate_index];
  
  Can0.begin(BAUDRATE250K);
  Can1.begin(BAUDRATE500K);
  Can0.setReportErrors(true);
  Can1.setReportErrors(true);
  
   
  // Enable transmission for the CAN TXRX
  pinMode(SILENT_0,OUTPUT);
  pinMode(SILENT_1,OUTPUT);
  pinMode(SILENT_2,OUTPUT);
  digitalWrite(SILENT_0,LOW);
  digitalWrite(SILENT_1,LOW);
  digitalWrite(SILENT_2,LOW);
  
  setup_CAN();
  

  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(20); // interval in ms
}


void loop()
{
  // keep watching the push button:
  debouncer.update();
  // Get the updated value :
  if ( debouncer.fell() ) change_baud();
  
  if (TXTimer10 >= TXPeriod10){
    TXTimer10 = 0; //Reset timer

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
    CAN0.sendMsgBuf(TX_ID, 1, 8, txmsg);  

    //Toggle LED light as messages are sent
    change_baud();
    //Print on serial for every sent message
    //Serial.print("Message Sent: ");
    //Serial.println(TXCount);
    Serial.print("Can0 REC count:");
    Serial.println(Can0.readREC());
    Serial.print("Can1 REC count:");
    Serial.println(Can1.readREC());
   
  
  }
  if (Can0.available()) {
    Can0.read(rxmsg0);
    printFrame(rxmsg0,0,RXCount0++);
    //Toggle the LED
    GREEN_LED_state = !GREEN_LED_state;
    digitalWrite(GREEN_LED_PIN,GREEN_LED_state);
  }
  if (Can1.available()) {
    Can1.read(rxmsg1);
    printFrame(rxmsg1,1,RXCount1++);
    //Toggle the LED
    GREEN_LED_state = !GREEN_LED_state;
    digitalWrite(GREEN_LED_PIN,GREEN_LED_state);
  }
}
