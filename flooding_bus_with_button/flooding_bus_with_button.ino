#include <FlexCAN.h>

//Define transmit message from FlexCAN library
static CAN_message_t txmsg;



//Define default baudrate
#define BAUDRATE250K 250000
#define BAUDRATE500K 500000

//Define LED
#define red_led 14
#define green_led 6


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
 
 //Set message extension, ID, and length
txmsg.ext = 1;
txmsg.len=8;
txmsg.id = 0x00000000;
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
 
 //Start serial with red LED on for power indicator
Serial.begin(9600);
 delay(2);
 pinMode (red_led, OUTPUT);
 pinMode (green_led, OUTPUT);
 digitalWrite (red_led, HIGH);
 pinMode(button,INPUT_PULLUP);
 delay(2000);
}
void loop() {
 // put your main code here, to run repeatedly:
 //If button is pushed, green LED turns on and send messages
  button_state = !digitalRead(button);
  digitalWrite(green_led,LOW);
  while(button_state == true){
    digitalWrite(green_led,button_state);
    Can0.write(txmsg);
    Serial.println("Sent!");
    if(digitalRead(button)){
      break;
    }
    }
}
