/*
 * NMFTA CAN Logger Project
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
 * Copyright (c) 2016        Jeremy S. Daily
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
#include <Bounce2.h>
#include <TimeLib.h>

// Instantiate a Bounce object to debounce the pushbutton
Bounce debouncer = Bounce(); 

// Set up the SD Card object
SdFs sd;

// Create a file object
FsFile binFile;

// All CAN messages are received in the FlexCAN structure
CAN_message_t rxmsg;

// Define CAN TXRX Transmission Silent pins
// See the CAN Logger 2 Schematic for the source pins
#define SILENT_0   39
#define SILENT_1   38
#define SILENT_2   37
#define CAN_SWITCH 2
#define BUTTON_PIN 21


// Set CS to pin 15, according to schematics
#define CS_CAN 15
MCP_CAN Can2(CS_CAN); 

elapsedMicros microsecondsPerSecond;
elapsedMillis lastCANmessageTimer;
elapsedMillis LEDblinkTimer;
elapsedMillis txTimer;

//Set LEDs
#define green_LED 6
#define red_LED 14
#define yellow_LED 5

boolean green_LED_state;
boolean red_LED_state;
boolean yellow_LED_state;

// Setup a counter to keep track of the filename
uint16_t current_file;
// Keep track of the CAN Channel (0, 1, or 2, where 2 is MCP CAN)
uint8_t current_channel;
//There are 2 data buffers that get switched
uint8_t current_buffer;
uint16_t current_position;

#define MAX_MESSAGES 20
#define BUFFER_POSITION_LIMIT 499
#define BUFFER_SIZE 512
uint8_t data_buffer[2][BUFFER_SIZE];

//Counter to keep track
uint32_t TXCount = 0;

//Create a counter to keep track of message traffic
uint32_t RXCount0 = 0;
uint32_t RXCount1 = 0;

boolean file_open;

void load_buffer(){
  // reset the timer
  lastCANmessageTimer = 0;
  
  data_buffer[current_buffer][current_position] = current_channel;
  current_position += 1;
  
  time_t timeStamp = now();
  memcpy(&data_buffer[current_buffer][current_position], &timeStamp, 4);
  current_position += 4;
  
  uint32_t usec = micros();
  memcpy(&data_buffer[current_buffer][current_position], &usec, 4);
  current_position += 4;

  uint32_t ID =  rxmsg.id;
  memcpy(&data_buffer[current_buffer][current_position], &ID, 4);
  current_position += 4;
  
  uint32_t DLC = (rxmsg.len << 24) | (0x00FFFFFF & uint32_t(microsecondsPerSecond));
  memcpy(&data_buffer[current_buffer][current_position], &DLC, 4);
  current_position += 4;

  memcpy(&data_buffer[current_buffer][current_position], &rxmsg.buf, 8); 
  current_position += 8;
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define DEFAULT_TIME  1357041600 // Jan 1 2013 

uint32_t processSyncMessage() {
  uint32_t pctime = 0L;
  
  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

time_t getTeensy3Time(){
  microsecondsPerSecond = 0;
  return Teensy3Clock.get();
}

void dateTime(uint16_t* FSdate, uint16_t* FStime) {
  // Return date using FS_DATE macro to format fields.
  *FSdate = FS_DATE(year(), month(), day());

  // Return time using FS_TIME macro to format fields.
  *FStime = FS_TIME(hour(), minute(), second());
}

void send_mcp_messages(){
  //if (txTimer >= 10){
    txTimer = 0;
  
    //Convert the 32-bit timestamp into 4 bytes with the most significant byte (MSB) first (Big endian).
    uint32_t sysMicros = micros();
    byte txmsg[8];
    txmsg[0] = (sysMicros & 0xFF000000) >> 24;
    txmsg[1] = (sysMicros & 0x00FF0000) >> 16;
    txmsg[2] = (sysMicros & 0x0000FF00) >>  8;
    txmsg[3] = (sysMicros & 0x000000FF);
    
    //Convert the 32-bit transmit counter into 4 bytes with the most significant byte (MSB) first (Big endian). 
    
    txmsg[4] = (TXCount & 0xFF000000) >> 24;
    txmsg[5] = (TXCount & 0x00FF0000) >> 16;
    txmsg[6] = (TXCount & 0x0000FF00) >>  8;
    txmsg[7] = (TXCount & 0x000000FF);
    
    //Send messaeg in format: ID, Standard (0) or Extended ID (1), message length, txmsg
    Can2.sendMsgBuf(0x1CFEFE00, 1, 1, txmsg);  
    
    //Toggle LED light as messages are sent
    red_LED_state = !red_LED_state;                       
    digitalWrite(red_LED,red_LED_state);
    
    //Print on serial for every sent message
    Serial.print("Message Sent: ");
    Serial.println(TXCount);
    TXCount++;
  //}  
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

void errorFlash(){
  while(true){
    digitalWrite(red_LED,HIGH);
    delay(50);
    digitalWrite(red_LED,LOW);
    delay(50);
  }
}

void setup(void) {

  char prefix[5];
  sprintf(prefix,"CAN2");
  memcpy(&data_buffer[0][0], &prefix, 4);
  memcpy(&data_buffer[1][0], &prefix, 4);
  current_position = 4;
  
  pinMode(SILENT_0,OUTPUT);
  pinMode(SILENT_1,OUTPUT);
  pinMode(SILENT_2,OUTPUT);
  //Prevent transmission for the CAN TXRX
  digitalWrite(SILENT_0,LOW); //Change these values to get different errors
  digitalWrite(SILENT_1,LOW);
  digitalWrite(SILENT_2,LOW);
  
  pinMode(CS_CAN, OUTPUT);
  digitalWrite(CS_CAN,HIGH);
  
  pinMode(green_LED, OUTPUT);
  pinMode(yellow_LED, OUTPUT);
  pinMode(red_LED, OUTPUT);
  green_LED_state = HIGH;
  digitalWrite(green_LED,green_LED_state);
  digitalWrite(yellow_LED,yellow_LED_state);
  digitalWrite(red_LED,red_LED_state);

  // Setup the button with an internal pull-up :
  pinMode(BUTTON_PIN,INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(20); // interval in ms
  
  pinMode(CAN_SWITCH,OUTPUT);
  digitalWrite(CAN_SWITCH,LOW);

  while(!Serial);
  Serial.println("Starting CAN logger.");
  
  //Initialize the CAN channels
  Can0.begin(1000000);
  Can1.begin(1000000);
  
  //The default filters exclude the extended IDs, so we have to set up CAN filters to allow those to pass.
  CAN_filter_t allPassFilter;
  allPassFilter.ext=1;
  for (uint8_t filterNum = 0; filterNum < 8;filterNum++){ //only use half the available filters for the extended IDs
    Can0.setFilter(allPassFilter,filterNum); 
    Can1.setFilter(allPassFilter,filterNum); 
  }
  // Setup MCP CAN
  if(Can2.begin(MCP_ANY, CAN_1000KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");
  Can2.setMode(MCP_NORMAL);
  
  setSyncProvider(getTeensy3Time);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
  setSyncInterval(1);
  char timeString[32];
  sprintf(timeString,"%04d-%02d-%02d %02d:%02d:%02d.%06d",year(),month(),day(),hour(),minute(),second(),uint32_t(microsecondsPerSecond));
  Serial.println(timeString);
  
   // Set callback
  FsDateTime::callback = dateTime;
  
  if (!sd.begin(SdioConfig(FIFO_SDIO))) {
      Serial.println("SdFs begin() failed");
      errorFlash();
  }
  
  sd.chvol();
  
 
}

void open_binFile(){
  current_file++;
  char filename[200];
  sprintf(filename,"CAN_%04d.bin",current_file);
  if (!binFile.open(filename, O_RDWR | O_CREAT)) {
    Serial.println("open failed");
    errorFlash();
  }
  binFile.truncate(0);
  file_open = true;
}

void close_binFile(){
  binFile.close();
  delay(100);
  sd.ls(LS_DATE | LS_SIZE);
  file_open = false;
}



void loop(void) {
  // keep watching the push button:
  debouncer.update();

  // Perform logging operation based on the button push.
  // This assumes all the CAN Channels are connected together.
  if (debouncer.fell()) open_binFile();
  else if (debouncer.rose()) close_binFile();
  else if (!debouncer.read() ) send_mcp_messages();

  //Check to see if there is anymore room in the buffer
  if (current_position >= BUFFER_POSITION_LIMIT){ //20 messages
    uint32_t start_micros = micros();
    current_position = 4; //Let the first four bytes remain the same
    if (BUFFER_SIZE != binFile.write(data_buffer[current_buffer], BUFFER_SIZE)) {
      Serial.println("write failed");
    }
    else 
    {
   ;
    }

    //Record write times
    uint32_t elapsed_micros = micros() - start_micros;
    memcpy(&data_buffer[current_buffer][508], &elapsed_micros, 4);
    
    current_buffer += 1;
    current_buffer = current_buffer % 2; // Switch back to zero if the value is 2
    //Toggle LED to show SD card writing
    yellow_LED_state = !yellow_LED_state;
    digitalWrite(yellow_LED,yellow_LED_state);
  }
  
  if (Can0.read(rxmsg)){
    current_channel = 0;
    load_buffer();
    //printFrame(rxmsg,0,RXCount0++);
  }
  if (Can1.read(rxmsg)){
    current_channel = 1;
    load_buffer();
    //printFrame(rxmsg,1,RXCount1++);
  }

  if (LEDblinkTimer >= 500){
    LEDblinkTimer = 0; 
    green_LED_state = !green_LED_state;
    digitalWrite(green_LED, green_LED_state);
  }

  //Turn off the yellow LED after some time.
  if (lastCANmessageTimer > 1000){
    lastCANmessageTimer = 0;
    if (binFile.isOpen()) close_binFile();
    yellow_LED_state = LOW;
    digitalWrite(yellow_LED,yellow_LED_state);
  }

  //Turn off the red LED after some time.
  if (txTimer >= 1000);
    txTimer = 0;
    red_LED_state = LOW;
    digitalWrite(red_LED,red_LED_state);  
}
