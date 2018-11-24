#define FAST_BLINK_TIME 50 //milliseconds
#define SLOW_BLINK_TIME 500 //milliseconds

//Set LEDs
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

elapsedMillis main_timer;


void setup() {
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
  delay(1000);
  GREEN_LED_slow_blink_state = LOW;
  RED_LED_slow_blink_state = HIGH;
  YELLOW_LED_slow_blink_state = HIGH;
  
  GREEN_LED_fast_blink_state = !GREEN_LED_slow_blink_state;
  RED_LED_fast_blink_state = !RED_LED_slow_blink_state;
  YELLOW_LED_fast_blink_state = !YELLOW_LED_slow_blink_state;
}

void loop() {

  if (main_timer >= 5000)
  {
    main_timer = 0;
    GREEN_LED_fast_blink_state = GREEN_LED_slow_blink_state;
    RED_LED_fast_blink_state = RED_LED_slow_blink_state;
    YELLOW_LED_fast_blink_state = YELLOW_LED_slow_blink_state;  
    
    GREEN_LED_slow_blink_state = !GREEN_LED_slow_blink_state;
    RED_LED_slow_blink_state = !RED_LED_slow_blink_state;
    YELLOW_LED_slow_blink_state = !YELLOW_LED_slow_blink_state;  
  }
  
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
}


