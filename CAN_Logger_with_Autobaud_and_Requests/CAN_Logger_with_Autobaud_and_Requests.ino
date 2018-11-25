/*
 * NMFTA CAN Logger 2 Project
 * 
 * Arduino Sketch to test the ability to receive and log CAN messages
 * 
 * Written By Dr. Jeremy S. Daily
 * The University of Tulsa
 * Department of Mechanical Engineering
 * 
 * 18 Nov 2018
 * 
 * Released under the MIT License
 *
 * Copyright (c) 2018        Jeremy S. Daily
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * 
 * This program logs data to a binary file.  
 * 
 * Samples are logged at regular intervals.  The maximum logging rate
 * depends on the quality of your SD card. 
 * 
 * Much of this sketch was inspired by the examples from https://github.com/greiman/SdFs
 * 
 * The program makes use of the SdFs library
 * 
 */


//included libraries 
#include <SdFs.h>
#include <mcp_can.h>
#include <FlexCAN.h>
#include <OneButton.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <error.h>
#include <FastCRC.h>

//Get access to a hardware based CRC32 
FastCRC32 CRC32;

// EEPROM memory addresses for creating file names
// The Address 0 and 1 are used for baud rates
#define EEPROM_DEVICE_ID_ADDR 2
#define EEPROM_FILE_ID_ADDR 8

// Setup a limit to turn off the CAN controller after this many messages.
#define ERROR_COUNT_LIMIT 5000

// Set up the SD Card object
SdFs sd;

// Create a file object
FsFile binFile;

// All CAN messages are received in the FlexCAN structure
CAN_message_t rxmsg,txmsg;

// Define CAN TXRX Transmission Silent pins
// See the CAN Logger 2 Schematic for the source pins
#define SILENT_0   39
#define SILENT_1   38
#define SILENT_2   37
#define CAN_SWITCH 2
#define BUTTON_PIN 21

// Use the button for multiple inputs: click, doubleclick, and long click.
OneButton button(BUTTON_PIN, true);

/*  code to process time sync messages from the serial port   */
char timeString[32];

// define a counter to reset after each second is counted.
elapsedMicros microsecondsPerSecond;

// Get a uniqueName for the Logger File
char logger_name[4] = "2AA";
bool file_open;

// Setup the MCP2515 controller
// Set CS to pin 15, according to schematics
#define CS_CAN 15
MCP_CAN Can2(CS_CAN); 
//variables for the CAN Message
uint32_t rxId;
uint8_t len;
uint8_t rxBuf[8];
uint8_t ext_flag;

#define FAST_BLINK_TIME 50 //milliseconds
#define SLOW_BLINK_TIME 500 //milliseconds

//Setup LEDs
#define GREEN_LED 6
#define RED_LED 14
#define YELLOW_LED 5

boolean GREEN_LED_state;
boolean RED_LED_state;
boolean YELLOW_LED_state;

elapsedMillis GREEN_LED_Slow_Timer;
elapsedMillis RED_LED_Slow_Timer;
elapsedMillis YELLOW_LED_Slow_Timer;

elapsedMillis GREEN_LED_Fast_Timer;
elapsedMillis RED_LED_Fast_Timer;
elapsedMillis YELLOW_LED_Fast_Timer;

boolean GREEN_LED_slow_blink_state;
boolean RED_LED_slow_blink_state;
boolean YELLOW_LED_slow_blink_state;

boolean GREEN_LED_fast_blink_state;
boolean RED_LED_fast_blink_state;
boolean YELLOW_LED_fast_blink_state;


// Setup a counter to keep track of the filename
uint16_t current_file;

// Keep track of the CAN Channel (0, 1, or 2, where 2 is MCP CAN)
uint8_t current_channel;


// There are 2 data buffers that get switched. We need to know which one
// we are using and where we are in the buffer.
#define CAN_FRAME_SIZE 25
#define MAX_MESSAGES 19
#define BUFFER_POSITION_LIMIT (CAN_FRAME_SIZE * MAX_MESSAGES)
#define BUFFER_SIZE 512
uint8_t data_buffer[2][BUFFER_SIZE];
uint8_t current_buffer;
uint16_t current_position;

//Counter and timer to keep track of transmitted messages
#define TX_MESSAGE_TIME 5 //milliseconds
uint32_t TXCount0 = 0;
uint32_t TXCount1 = 0;
uint32_t TXCount2 = 0;
elapsedMillis TXTimer0;
elapsedMillis TXTimer1;
elapsedMillis TXTimer2;

// To use a button to send requests, we need to track it.
#define SEND_REQUEST_TIME
elapsedMillis send_request_timer;
boolean send_requests;
boolean send_additional_requests;
uint16_t request_index;
    
//Create a counter and timer to keep track of received message traffic
#define RX_TIME_OUT 1000 //milliseconds
elapsedMillis RXTimer;
uint32_t RXCount0 = 0;
uint32_t RXCount1 = 0;
uint32_t RXCount2 = 0;
elapsedMillis lastCAN0messageTimer;
elapsedMillis lastCAN1messageTimer;
elapsedMillis lastCAN2messageTimer;
uint32_t ErrorCount0 = 0;
uint32_t ErrorCount1 = 0;
uint32_t ErrorCount2 = 0;



/*
 * The load_buffer() function maps the received CAN message into
 * the positions in the buffer to store. Each buffer is 512 bytes.
 * 20 CAN messages can fit in the buffer since they are 25 bytes long.
 * Note: an additional channel byte was added when comparing to the 
 * NMFTA CAN Logger 1 files. 
 * 
 * Use this function to understand the binary file format.
 * When reading the binary, be sure to read it in 512 byte chunks.
 */
void load_buffer(){
  // reset the timer
  RXTimer = 0;

  //Toggle the LED
  GREEN_LED_state = !GREEN_LED_state;
  digitalWrite(GREEN_LED, GREEN_LED_state);
    
  data_buffer[current_buffer][current_position] = current_channel;
  current_position += 1;
  
  time_t timeStamp = now();
  memcpy(&data_buffer[current_buffer][current_position], &timeStamp, 4);
  current_position += 4;
  
  memcpy(&data_buffer[current_buffer][current_position], &rxmsg.micros, 4);
  current_position += 4;

  memcpy(&data_buffer[current_buffer][current_position], &rxmsg.id, 4);
  current_position += 4;
  
  uint32_t DLC = (rxmsg.len << 24) | (0x00FFFFFF & uint32_t(microsecondsPerSecond));
  memcpy(&data_buffer[current_buffer][current_position], &DLC, 4);
  current_position += 4;

  memcpy(&data_buffer[current_buffer][current_position], &rxmsg.buf, 8); 
  current_position += 8;

  if ((rxmsg.id & CAN_ERR_FLAG) == CAN_ERR_FLAG){
    RED_LED_state = !RED_LED_state;
    digitalWrite(RED_LED, RED_LED_state);  
  }
  
  //Create a file if it is not open yet
  if (!file_open) open_binFile();
  
  // Check the current position and see if the buffer needs to be written to memory
  check_buffer();
}


/*
 * Check to make sure the characters are valid in a character array
 * This is used to check the data coming from EEPROM.
 */
bool isFileNameValid( const char * fileName )
{
  char c;
  while ( (c = *fileName++) )
    if ( c != '.' && !isalnum(c) )
      return false;
  return true;
}

/*
 * Reset the microsecond counter and return the realtime clock
 * This function requires the sync interval to be exactly 1 second.
 */
time_t getTeensy3Time(){
  microsecondsPerSecond = 0;
  return Teensy3Clock.get();
}

/*
 * Routine to enable date and time for the sdcard 
 * file system
 */
void dateTime(uint16_t* FSdate, uint16_t* FStime) {
  // Return date using FS_DATE macro to format fields.
  *FSdate = FS_DATE(year(), month(), day());

  // Return time using FS_TIME macro to format fields.
  *FStime = FS_TIME(hour(), minute(), second());
}


/*
 * Routines for the Microchip MCP2515 CAN Controller on Can2
 * This third CAN channel is wired to the J1708 pins (F and G) found
 * on some newer PACCAR trucks.
 */
void send_Can2_messages(CAN_message_t &txmsg){
  if (TXTimer2 >= TX_MESSAGE_TIME){
    //Send message in format: ID, Standard (0) or Extended ID (1), message length, txmsg
    Can2.sendMsgBuf(txmsg.id, 1, txmsg.len, txmsg.buf);  
    TXCount2++;
    TXTimer2 = 0;
    //Toggle LED light as messages are sent
    RED_LED_state = !RED_LED_state;                       
    digitalWrite(RED_LED,RED_LED_state);  
  }  
}

void send_Can0_message(CAN_message_t &txmsg){
  if (TXTimer0 >= TX_MESSAGE_TIME){    
    //Send message in FlexCAN format
    Can0.write(txmsg);
    TXCount0++;
    TXTimer0 = 0;
    //Toggle LED light as messages are sent
    RED_LED_state = !RED_LED_state;                       
    digitalWrite(RED_LED,RED_LED_state);  
  }
}    

void send_Can1_message(CAN_message_t &txmsg){
  if (TXTimer1 >= TX_MESSAGE_TIME){    
    //Send message in FlexCAN format
    Can1.write(txmsg);
    TXCount1++;
    TXTimer1 = 0;
    //Toggle LED light as messages are sent
    RED_LED_state = !RED_LED_state;                       
    digitalWrite(RED_LED,RED_LED_state);  
  }
}

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

void sdErrorFlash(){
  while(true){
    digitalWrite(RED_LED,HIGH);
    delay(50);
    digitalWrite(RED_LED,LOW);
    delay(50);
    // Try starting the SD Card again
    if (sd.begin(SdioConfig(FIFO_SDIO))) break; 
  }
}

void open_binFile(){
  current_file++;
  char filename[13];
  sprintf(filename,"TU%03s%03d.bin",logger_name,current_file);
  while (!binFile.open(filename, O_RDWR | O_CREAT)) {
    Serial.println("Binary Log File Creation Failed.");
    sdErrorFlash();
  }
  // Write the filename to each line in the 512 byte block
  memcpy(&data_buffer[0][497], &filename, 8);
  memcpy(&data_buffer[1][497], &filename, 8);
  
  //Move the current position to 4 since the first 4 bytes are taken.
  current_position = 4;
  binFile.truncate(0);
  file_open = true;
  RXTimer = 0;
}

void close_binFile(){
  //Write the last set of data
  if (BUFFER_SIZE != binFile.write(data_buffer[current_buffer], BUFFER_SIZE)) {
      Serial.println("write failed");
  }
  binFile.close();
  EEPROM.put(EEPROM_FILE_ID_ADDR,current_file);
  delay(100);
  sd.ls(LS_DATE | LS_SIZE);
  file_open = false;
  //Initialize the CAN channels with autobaud.
  
  RXCount0 = 0;
  RXCount1 = 0;
  RXCount2 = 0;
  Can0.begin(0);
  Can1.begin(0);
}

void check_buffer(){
  //Check to see if there is anymore room in the buffer
  if (current_position >= BUFFER_POSITION_LIMIT){ //max number of messages
    uint32_t start_micros = micros();
    memcpy(&data_buffer[current_buffer][479], &RXCount0, 4);
    memcpy(&data_buffer[current_buffer][483], &RXCount1, 4);
    memcpy(&data_buffer[current_buffer][487], &RXCount2, 4);
    
    data_buffer[current_buffer][491] = Can0.readREC();
    data_buffer[current_buffer][492] = Can1.readREC();
    data_buffer[current_buffer][493] = Can2.errorCountRX();
    
    data_buffer[current_buffer][494] = Can0.readTEC();
    data_buffer[current_buffer][495] = Can1.readTEC();
    data_buffer[current_buffer][496] = Can2.errorCountTX();
    
    uint32_t checksum = CRC32.crc32(data_buffer[current_buffer], BUFFER_SIZE);
    memcpy(&data_buffer[current_buffer][508], &checksum, 4);
    
    if (BUFFER_SIZE != binFile.write(data_buffer[current_buffer], BUFFER_SIZE)) {
      Serial.println("write failed");
      sdErrorFlash(); 
    }
    current_position = 4; //Let the first four bytes remain the same
    current_buffer += 1;
    current_buffer = current_buffer % 2; // Switch back to zero if the value is 2
    
    //Record write times for the previous frame, since the existing frame was just written
    uint32_t elapsed_micros = micros() - start_micros;
    memcpy(&data_buffer[current_buffer][505], &elapsed_micros, 3);
    
    // Set all values in the array to FF so old messages don't show up at the end.
    memset(&data_buffer[current_buffer], 0xFF, BUFFER_SIZE);
    
    //Toggle LED to show SD card writing
    YELLOW_LED_state = !YELLOW_LED_state;
    digitalWrite(YELLOW_LED,YELLOW_LED_state);
  }
}

void led_blink_routines(){
  if (GREEN_LED_fast_blink_state)
  {
    if (GREEN_LED_Fast_Timer >= FAST_BLINK_TIME)
    {
      GREEN_LED_Fast_Timer = 0;
      GREEN_LED_state = !GREEN_LED_state;
      digitalWrite(GREEN_LED,GREEN_LED_state);
    }
  }
  else if (GREEN_LED_slow_blink_state)
  {
    if (GREEN_LED_Slow_Timer >= SLOW_BLINK_TIME)
    {
      GREEN_LED_Slow_Timer = 0;
      GREEN_LED_state = !GREEN_LED_state;
      digitalWrite(GREEN_LED,GREEN_LED_state);
    }
  }
  else
  {
    //Turn on the green LED after receiving.
    if (lastCAN1messageTimer > 200){
      lastCAN1messageTimer = 0;
      GREEN_LED_state = HIGH;
      digitalWrite(GREEN_LED,GREEN_LED_state);
    }
  }

  /*
   * YELLOW LED functions
   * Use the yellow LED for status
   * Flicker the yellow LED for for received messages
   */
  if (YELLOW_LED_fast_blink_state)
  {
    if (YELLOW_LED_Fast_Timer >= FAST_BLINK_TIME)
    {
      YELLOW_LED_Fast_Timer = 0;
      YELLOW_LED_state = !YELLOW_LED_state;
      digitalWrite(YELLOW_LED,YELLOW_LED_state);
    }
  }
  else if (YELLOW_LED_slow_blink_state)
  {
    if (YELLOW_LED_Slow_Timer >= SLOW_BLINK_TIME)
    {
      YELLOW_LED_Slow_Timer = 0;
      YELLOW_LED_state = !YELLOW_LED_state;
      digitalWrite(YELLOW_LED,YELLOW_LED_state);
    }
  }
  else
  {
    //Turn off the yellow LED after receiving.
    if (lastCAN0messageTimer > 200){
      lastCAN0messageTimer = 0;
      YELLOW_LED_state = LOW;
      digitalWrite(YELLOW_LED,YELLOW_LED_state);
    }
  }
  
  /*
   * RED LED functions
   * Use the Red LED for error states
   * Also, flicker the RED LED for transmitted messages
   */
  if (RED_LED_fast_blink_state)
  {
    if (RED_LED_Fast_Timer >= FAST_BLINK_TIME)
    {
      RED_LED_Fast_Timer = 0;
      RED_LED_state = !RED_LED_state;
      digitalWrite(RED_LED,RED_LED_state);
    }
  }
  else if (RED_LED_slow_blink_state)
  {
    if (RED_LED_Slow_Timer >= SLOW_BLINK_TIME)
    {
      RED_LED_Slow_Timer = 0;
      RED_LED_state = !RED_LED_state;
      digitalWrite(RED_LED,RED_LED_state);
    }
  }
  else
  {
    //Turn off the red LED after some time after transmitting.
    if (TXTimer0 >= 200 && TXTimer1 >= 200 && TXTimer2 >= 200)
    {
      RED_LED_state = LOW;
      digitalWrite(RED_LED,RED_LED_state);  
    }
  }
}

void myClickFunction(){
    send_request_timer = 0;
    send_requests = true;
}

void myDoubleClickFunction(){
    close_binFile();
  }

void myLongPressStopFunction(){
  send_request_timer = 0;
  send_requests = true;
  send_additional_requests = true;
  RED_LED_state = LOW;
  digitalWrite(RED_LED,RED_LED_state);
}

void myLongPressStartFunction(){
  
}

void myLongPressFunction(){
  RED_LED_state = HIGH;
  digitalWrite(RED_LED,RED_LED_state);
}


void setup(void) {
  // Write the begining of each line in the 512 byte block
  char prefix[5];
  sprintf(prefix,"CAN2");
  memcpy(&data_buffer[0][0], &prefix, 4);
  memcpy(&data_buffer[1][0], &prefix, 4);
  current_position = 4;
  
  pinMode(SILENT_0,OUTPUT);
  pinMode(SILENT_1,OUTPUT);
  pinMode(SILENT_2,OUTPUT);
  //Set High to prevent transmission for the CAN TXRX
  digitalWrite(SILENT_0,LOW); 
  digitalWrite(SILENT_1,LOW);
  digitalWrite(SILENT_2,LOW);
  
  pinMode(CS_CAN, OUTPUT);
  digitalWrite(CS_CAN,HIGH);
  
  // put your setup code here, to run once:
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  GREEN_LED_state = HIGH;
  YELLOW_LED_state = HIGH;
  RED_LED_state = HIGH;
  digitalWrite(GREEN_LED,GREEN_LED_state);
  digitalWrite(YELLOW_LED,YELLOW_LED_state);
  digitalWrite(RED_LED,RED_LED_state);

  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  button.attachClick(myClickFunction);
  button.attachDoubleClick(myDoubleClickFunction);
  button.attachLongPressStart(myLongPressStartFunction);
  button.attachLongPressStop(myLongPressStopFunction);
  button.attachDuringLongPress(myLongPressFunction);
  button.setDebounceTicks(50);
  button.setPressTicks(2000);
  button.setClickTicks(500);
  
  pinMode(CAN_SWITCH,OUTPUT);
  digitalWrite(CAN_SWITCH,LOW);

  //while(!Serial);
  Serial.println("Starting CAN logger.");
  
  //Initialize the CAN channels with autobaud.
  Can0.begin(0);
  Can1.begin(0);

  // log what we transmit
  Can0.setSelfReception(false);
  Can1.setSelfReception(false);

  Can0.setListenOnly(false);
  Can1.setListenOnly(false);
  
  Can0.setReportErrors(true);
  Can1.setReportErrors(true);
    
  //Flex CAN defaults
  txmsg.ext = 1;
  txmsg.len = 8;

  // Setup MCP CAN
  if(Can2.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");
  Can2.setMode(MCP_LISTENONLY);

  // Setup timing services
  setSyncProvider(getTeensy3Time);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
  setSyncInterval(1);
  sprintf(timeString,"%04d-%02d-%02d %02d:%02d:%02d.%06d",year(),month(),day(),hour(),minute(),second(),uint32_t(microsecondsPerSecond));
  Serial.println(timeString);
  
   // Set callback
  FsDateTime::callback = dateTime;

  // Mount the SD Card
  if (!sd.begin(SdioConfig(FIFO_SDIO))) {
      // If the SD card is missing, then it will f
      Serial.println("SdFs begin() failed. Please check the SD Card.");
      sdErrorFlash(); 
  }
  
  // Check for free space on the card.
  Serial.print("SD Total Cluster Count: ");
  Serial.println(sd.clusterCount());
  Serial.print("SD Free Cluster Count: ");
  Serial.println(sd.freeClusterCount());
  
  while (sd.freeClusterCount() < 10000){
    Serial.println("There needs to be more free space on the SD card.");
    sdErrorFlash();
  }
  
  sd.chvol();
  
  char default_name[] = "___";
  EEPROM.get(EEPROM_DEVICE_ID_ADDR,logger_name);
  if (!isFileNameValid(logger_name)) memcpy(&logger_name, &default_name, 4);
  EEPROM.get(EEPROM_FILE_ID_ADDR,current_file);
}


void loop(void) {
  // monitor the CAN channels
  if (Can0.read(rxmsg)){
    RXCount0++;
    current_channel = 0;
    load_buffer();
    printFrame(rxmsg,0,RXCount0);
    if ((rxmsg.id & CAN_ERR_FLAG) == CAN_ERR_FLAG){
      ErrorCount0++;
      Serial.print("Error Count 0: ");
      Serial.println(ErrorCount0); 
//      if (ErrorCount0 > ERROR_COUNT_LIMIT){
//        ErrorCount0 = 0;
//        Can0.end();
//      }
    }
  }
  if (Can1.read(rxmsg)){
    RXCount1++;
    current_channel = 1;
    load_buffer();
    printFrame(rxmsg,1,RXCount1);
    if ((rxmsg.id & CAN_ERR_FLAG) == CAN_ERR_FLAG){
      ErrorCount1++;
      Serial.print("Error Count 1: ");
      Serial.println(ErrorCount1); 
//      if (ErrorCount1 > ERROR_COUNT_LIMIT){
//        ErrorCount1 = 0;
//        Can1.end();
//      }
    }
  }
  if (Can2.readMsgBuf(&rxId, &ext_flag, &len, rxBuf) == CAN_OK){
    RXCount2++;
    memcpy(&rxmsg.buf, rxBuf, 8);
    rxmsg.len = uint8_t(len);
    rxmsg.id = uint32_t(rxId);
    current_channel = 2;
    load_buffer();
    printFrame(rxmsg,2,RXCount2);
  } 

  // Close the file if messages stop showing up.
  if (RXTimer >= RX_TIME_OUT && file_open){
    close_binFile();
    RXTimer = 0;
  }
  
  // Send requests for additional data if needed
  if (send_requests){
      
  }
  if (send_additional_requests){
    
  }

  // Blink the LEDs if needed
  led_blink_routines();
  
  // keep watching the push button:
  button.tick();
}

