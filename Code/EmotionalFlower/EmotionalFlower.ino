/*  Emotional Flower Code, using code adapted from Adafruit
 *   
 *  Flower changes from happy, neutral and sad states depending on 
 *  what data has been recieved. Finite state machine keeps track
 *  of past and current states to determine appropriate movement.
 *  
 *  By: Jenna MacCarley
 */

/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  Servo test - this will drive 16 servos, one after the other

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These displays use I2C to communicate, 2 pins are required to  
  interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define MOVE_SIGNAL_PIN 3

// Define states
#define SAD 0
#define NEUTRAL 1
#define HAPPY 2
#define IMPASSIONED 3

// Define delay between movements
#define MOVE_DELAY 20

// Max and min for flower petals
#define SERVOMIN  200
#define SERVOMAX  350 

#define STEM_MIN 250
#define STEM_MAX 400
#define STEM_NEUTRAL 325

int oldState = 1;
int newState = 1;

void setup() {
  Serial.begin(9600);
  Serial.println("16 channel Servo test!");

#ifdef ESP8266
  Wire.pins(2, 14);   // ESP8266 can use any two pins, such as SDA to #2 and SCL to #14
#endif

  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(MOVE_SIGNAL_PIN, OUTPUT);

  digitalWrite(MOVE_SIGNAL_PIN, HIGH);

  // Set to neutral pos at beginning
  uint16_t flowerPos = SERVOMAX-75;
  uint16_t pulselen = STEM_NEUTRAL;
  pwm.setPWM(0, 0, 500-pulselen);
  pwm.setPWM(1, 0, pulselen);
  pwm.setPWM(2, 0, flowerPos);
  pwm.setPWM(3, 0, flowerPos);
  pwm.setPWM(4, 0, flowerPos);
  pwm.setPWM(5, 0, flowerPos);

  yield();
}

void loop() {

  int val11 = digitalRead(11);
  int val12 = digitalRead(12);

  Serial.println("pin 11");
  Serial.println(val11);
  Serial.println("pin 12");
  Serial.println(val12);

  digitalWrite(MOVE_SIGNAL_PIN, LOW);

  // Finite state machine based on previous and current state
  oldState = newState;
  if(!val11 && !val12){
    Serial.println("Sad");
    newState = SAD;

    if(oldState == NEUTRAL) neutral_to_sad();
    else if(oldState == HAPPY) happy_to_sad();
  }
  else if(!val11 && val12){
    Serial.println("Neutral");
    newState = NEUTRAL;

    if(oldState == HAPPY) happy_to_neutral();
    else if(oldState == SAD || oldState == IMPASSIONED) sad_to_neutral();
  }
  else if(val11 && !val12){
    Serial.println("Happy");
    newState = HAPPY;

    if(oldState == NEUTRAL) neutral_to_happy();
    else if(oldState == SAD || oldState == IMPASSIONED) sad_to_neutral();
  }
  else if(val11 && val12){
    Serial.println("Impassioned");
    newState = IMPASSIONED;

    if(oldState == NEUTRAL) neutral_to_sad();
    else if(oldState == HAPPY) happy_to_sad();
  }

  digitalWrite(MOVE_SIGNAL_PIN, HIGH);
  
  delay(1000);  
}

void happy_to_sad(){
  uint16_t flowerPos = SERVOMIN;
  for (uint16_t pulselen = STEM_MAX; pulselen > STEM_MIN; pulselen--) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
  }
  for (uint16_t count = 0; count < 50; count++) {
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
  }
}

void sad_to_happy(){
  uint16_t flowerPos = SERVOMAX;
  for (uint16_t pulselen = STEM_MIN; pulselen < STEM_MAX; pulselen++) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
  }
  for (uint16_t count = 0; count < 50; count++) {
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
  }
}

void sad_to_neutral(){
  uint16_t flowerPos = SERVOMAX;
  for (uint16_t pulselen = STEM_MIN; pulselen < STEM_NEUTRAL; pulselen++) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
  }
}

void happy_to_neutral(){
  uint16_t flowerPos = SERVOMIN;
  for (uint16_t pulselen = STEM_MAX; pulselen > STEM_NEUTRAL; pulselen--) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
  }
}

void neutral_to_happy(){
  uint16_t flowerPos = SERVOMAX-75;
  for (uint16_t pulselen = STEM_NEUTRAL; pulselen < STEM_MAX; pulselen++) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
  }
  for (uint16_t count = 0; count < 50; count++) {
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
  }
}

void neutral_to_sad(){
  uint16_t flowerPos = SERVOMIN + 75;
  for (uint16_t pulselen = STEM_NEUTRAL; pulselen > STEM_MIN; pulselen--) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
  }
  for (uint16_t count = 0; count < 50; count++) {
    delay(MOVE_DELAY);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
  }
}


