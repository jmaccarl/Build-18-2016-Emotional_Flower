/*  Emotional Flower Code, using code adapted from Adafruit
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

  yield();
}

void loop() {

  int val11 = digitalRead(11);
  int val12 = digitalRead(12);
  Serial.println("pin 11");
  Serial.println(val11);
  Serial.println("pin 12");
  Serial.println(val12);

  if(!val11 && !val12) Serial.println("Sad");
  if(!val11 && val12) Serial.println("Neutral");
  if(val11 && !val12) Serial.println("Happy");
  if(val11 && val12) Serial.println("Impassioned");

  
  delay(500);
  uint16_t flowerPos = SERVOMIN;
  for (uint16_t pulselen = 350; pulselen > 200; pulselen--) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(10);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
  }
  for (uint16_t count = 0; count < 50; count++) {
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos++;
    delay(10);
  }

  delay(500);
  
  for (uint16_t pulselen = 200; pulselen < 350; pulselen++) {
    if (pulselen > 250) pwm.setPWM(0, 0, 500-pulselen);
    pwm.setPWM(1, 0, pulselen);
    delay(10);
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
  }
  for (uint16_t count = 0; count < 50; count++) {
    pwm.setPWM(2, 0, flowerPos);
    pwm.setPWM(3, 0, flowerPos);
    pwm.setPWM(4, 0, flowerPos);
    pwm.setPWM(5, 0, flowerPos);
    flowerPos--;
    delay(10);
  }
  
}
