//Include MCP and SPI library
#include <mcp_can.h>
#include <Bounce2.h>

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
  if(CAN0.begin(MCP_ANY, current_baud_rate, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");
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
  Serial.write("Starting CAN Send Test.");
  
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
    green_LED_state = !green_LED_state;                       
    digitalWrite(green_LED,green_LED_state);

    //Print on serial for every sent message
    Serial.print("Message Sent: ");
    Serial.println(TXCount);
  }
}
