/*

 This example connects to an unencrypted WiFi network.
 Then it prints the  MAC address of the WiFi shield,
 the IP address obtained, and other network details.

 Circuit:
 * CAN Logger 2
 * A Jumper wire is installed from D20 to Pin 4 on the ATWINC1500


 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe

 Adapted by Jeremy Daily

 Need to change line 54 in
 CAN-Logger-2\WiFi101\src\bus_wrapper\source\nm_bus_wrapper_samd21.cpp
 to use SPI1
 #if !defined(WINC1501_SPI)
   #define WINC1501_SPI SPI1
 #endif
 */
#include <SPI.h>

#include <WiFi101.h>
#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

#define WINC_CS   31
#define WINC_IRQ  23
#define WINC_RST  20
#define WINC_EN   5

int status = WL_IDLE_STATUS;     // the WiFi radio's status

void setup() {
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
  
  //Initialize serial and wait for port to open:
  while(!Serial);
  Serial.println("Starting up.");
  
  //Configure pins for Adafruit ATWINC1500 Breakout
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST);
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  Serial.println("Attempt to connect to WiFi network");
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWiFiData();

}

void loop() {
  // check the network connection once every 10 seconds:
  delay(10000);
  printCurrentNet();
}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
