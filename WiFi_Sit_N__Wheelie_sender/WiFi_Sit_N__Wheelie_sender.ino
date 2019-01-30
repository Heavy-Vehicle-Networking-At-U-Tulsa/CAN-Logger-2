#include <SPI.h>
#include <Adafruit_WINC1500.h>
#include <Adafruit_WINC1500Udp.h>

// Define the WINC1500 board connections below.
// If you're following the Adafruit WINC1500 board
// guide you don't need to modify these:
#define WINC_CS   31
#define WINC_IRQ  23
#define WINC_RST  7
#define WINC_EN   5

// The SPI pins of the WINC1500 (SCK, MOSI, MISO) should be

// Setup the WINC1500 connection with the pins above and the default hardware SPI.
Adafruit_WINC1500 WiFi(WINC_CS, WINC_IRQ, WINC_RST);

// Or just use hardware SPI (SCK/MOSI/MISO) and defaults, SS -> #10, INT -> #7, RST -> #5, EN -> 3-5V
//Adafruit_WINC1500 WiFi;

int status = WL_IDLE_STATUS;
char ssid[] = "SSID";  //  your network SSID (name)
char pass[] = "Password";       // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen on



Adafruit_WINC1500UDP Udp;


const int X_pin = A0; // analog pin connected to X output
const int Y_pin = A1; // analog pin connected to Y output
int x_value, y_value, x_constrain, y_constrain, x_map, y_map;


void setup() {
#ifdef WINC_EN
  pinMode(WINC_EN, OUTPUT);
  digitalWrite(WINC_EN, HIGH);
#endif

  //Initialize serial and wait for port to open:
  Serial.begin(9600);


  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);
}

void loop() {
  x_value = analogRead(X_pin);
  x_constrain = constrain(x_value,800,1023);
  x_map = map(x_constrain,800,1023,0,1023);
  //y_value = analogRead(Y_pin);
  //y_constrain = constrain(y_value,770,1023);
  //y_map = map(y_constrain,770,1023,0,1023);
  
  Udp.beginPacket("10.0.0.2", 2390);
  String stringOne;
  stringOne += x_map;
  //stringOne += y_map;
  Serial.println(stringOne);
  char charBuf[10];
  stringOne.toCharArray(charBuf,10);
  Udp.write(charBuf);
  Udp.endPacket();
  delay(10);



}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

}
