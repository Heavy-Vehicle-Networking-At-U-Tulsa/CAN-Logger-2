/*
 * Using teh CAN Logger 2, flood the J1939 bus and the other CAN Bus with all zeros
 */
#include <FlexCAN.h>

//Define transmit message from FlexCAN library
static CAN_message_t txmsg;

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
#define BAUDRATE250K 250000
#define BAUDRATE500K 500000

//Define Button
#define button 21
bool button_state;

//Define CAN TXRX Transmission Silent pins
#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37

void setup() {
  // put your setup code here, to run once:
  //Set baudrate
  Can0.begin(BAUDRATE250K);
  Can1.begin(BAUDRATE250K);
  
  //Set message extension, ID, and length
  txmsg.ext = 1;
  txmsg.len=8;
  txmsg.id = 0;
  txmsg.len = 8;
  txmsg.buf[0] = 0;
  txmsg.buf[1] = 0;
  txmsg.buf[2] = 0;
  txmsg.buf[3] = 0;
  txmsg.buf[4] = 0;
  txmsg.buf[5] = 0;
  txmsg.buf[6] = 0;
  txmsg.buf[7] = 0;
 
  // Enable transmission for the CAN TXRX
  pinMode(SILENT_0,OUTPUT);
  pinMode(SILENT_1,OUTPUT);
  pinMode(SILENT_2,OUTPUT);
  digitalWrite(SILENT_0,LOW);
  digitalWrite(SILENT_1,LOW);
  digitalWrite(SILENT_2,LOW);
  
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);
  pinMode(YELLOW_LED_PIN,OUTPUT);
  digitalWrite(GREEN_LED_PIN, HIGH);
  
  pinMode(button,INPUT_PULLUP);
}

void loop() {
 // put your main code here, to run repeatedly:
 //If button is pushed, red LED turns on and send messages
  if (!digitalRead(button)){  
    digitalWrite(GREEN_LED_PIN, LOW);  
    digitalWrite(RED_LED_PIN, HIGH);
    Can0.write(txmsg);  
    digitalWrite(YELLOW_LED_PIN, HIGH);
    Can1.write(txmsg);
  }
  else {
    digitalWrite(GREEN_LED_PIN, HIGH);  
    digitalWrite(RED_LED_PIN, LOW);  
    digitalWrite(YELLOW_LED_PIN, LOW);  
  }
}
