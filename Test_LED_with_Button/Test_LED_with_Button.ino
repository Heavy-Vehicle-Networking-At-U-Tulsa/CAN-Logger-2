//Define LED pins based on schematic
#define GREEN_LED_PIN 6
#define RED_LED_PIN 14
#define YELLOW_LED_PIN 5

//Define button pin, the button is soldered on SW21
#define button 21

//Create a on/off boolean for the button
bool buttonState;

void setup() {
  // put your setup code here, to run once:
  //Define LED pin mode
  pinMode(GREEN_LED_PIN,OUTPUT);
  pinMode(YELLOW_LED_PIN,OUTPUT);
  pinMode(RED_LED_PIN,OUTPUT);

  //Pull button high
  pinMode(button,INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  //If button is pushed, the pin will pull low
  buttonState = !digitalRead(button); 
  digitalWrite(GREEN_LED_PIN,buttonState);
  digitalWrite(YELLOW_LED_PIN,buttonState);
  digitalWrite(RED_LED_PIN,buttonState);
  
}
