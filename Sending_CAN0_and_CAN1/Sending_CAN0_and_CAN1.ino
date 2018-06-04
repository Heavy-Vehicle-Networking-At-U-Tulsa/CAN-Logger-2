#include <FlexCAN.h>

//Define transmit message from FlexCAN library
static CAN_message_t txmsg;

//Set up timing variables
#define TXPeriod100 100
elapsedMillis TXTimer100;

//Counter to keep track
uint32_t TXCount = 0;

//Define default baudrate
#define BAUDRATE250K 250000

//Define LED
#define red_led 14
#define green_led 6
boolean LED_state; 

//Define CAN TXRX Transmission Silent pins
#define SILENT_0 39
#define SILENT_1 38
#define SILENT_2 37

void setup() {
 // put your setup code here, to run once:
 //Set baudrate
 Can1.begin(BAUDRATE250K);
 Can0.begin(BAUDRATE250K);
 
 //Set message extension, ID, and length
 txmsg.ext = 1;
 txmsg.id=0x101;
 txmsg.len=8;
 
 
 // Enable transmission for the CAN TXRX
 pinMode(SILENT_0,OUTPUT);
 pinMode(SILENT_1,OUTPUT);
 pinMode(SILENT_2,OUTPUT);
 digitalWrite(SILENT_0,LOW);
 digitalWrite(SILENT_1,LOW);
 digitalWrite(SILENT_2,LOW);
 
 //Start serial with red LED on
Serial.begin(9600);
 delay(2);
 pinMode (red_led, OUTPUT);
 pinMode (green_led, OUTPUT);
 digitalWrite (red_led, HIGH);
 Serial.println("Sending Basic Traffic on bus");
 delay(2000);
}
void loop() {
 // put your main code here, to run repeatedly:
  
if (TXTimer100 >= TXPeriod100)
	{
	TXTimer100 = 0;//Reset Timer
  
	 //Convert the 32-bit timestamp into 4 bytes with the most significant byte (MSB) first (Big endian).
    uint32_t sysMicros = micros();
    txmsg.buf[0] = (sysMicros & 0xFF000000) >> 24;
    txmsg.buf[1] = (sysMicros & 0x00FF0000) >> 16;
    txmsg.buf[2] = (sysMicros & 0x0000FF00) >>  8;
    txmsg.buf[3] = (sysMicros & 0x000000FF);

   //Convert the 32-bit transmit counter into 4 bytes with the most significant byte (MSB) first (Big endian). 
    TXCount++;
    txmsg.buf[4] = (TXCount & 0xFF000000) >> 24;
    txmsg.buf[5] = (TXCount & 0x00FF0000) >> 16;
    txmsg.buf[6] = (TXCount & 0x0000FF00) >>  8;
    txmsg.buf[7] = (TXCount & 0x000000FF);

  //Write the same message on CAN channel 0 and 1
  	Can1.write(txmsg);
  	Can0.write(txmsg);

  //Toggle LED light as messages are sent
  	LED_state = !LED_state;
  	digitalWrite(green_led,LED_state);

  //Print on serial for every sent message
	  Serial.println("Message Sent");
	}
}
