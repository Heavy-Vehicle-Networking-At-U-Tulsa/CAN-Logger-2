//Include MCP and SPI library
#include <mcp_can.h>
#include <SPI.h>

INT32U rxId;
INT8U len = 0;
INT8U rxBuf[8];
INT8U ext;

// Set CS to pin 15, acording to schematics
#define CS_CAN 15
MCP_CAN CAN0(CS_CAN);  

void setup()
{
  pinMode(2,OUTPUT);
  digitalWrite(2,HIGH);
  Serial.begin(115200);
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);
  Serial.println("MCP2515 Initialized");
}

void loop()
{
  if (CAN0.readMsgBuf(&rxId, &ext, &len, rxBuf) == CAN_OK){             // Read data: len = data length, buf = data byte(s)
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
      delay(10);                                // Display available for every 10 milliseconds
  }
}

