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
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  200 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  350 // this is the 'maximum' pulse length count (out of 4096)

// our servo # counter
uint8_t servonum = 0;

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

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 60;   // 60 Hz
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000;
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}

void loop() {
  // Drive each servo one at a time
  //Serial.println(servonum);
  //for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
   // pwm.setPWM(2, 0, pulselen);
  //  pwm.setPWM(3, 0, pulselen);
  //  pwm.setPWM(4, 0, pulselen);
  //  pwm.setPWM(5, 0, pulselen);
 // }

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
  /*
  for (uint16_t pulselen = 200; pulselen < 500; pulselen++) {
    pwm.setPWM(0, 0, pulselen);
  }

  delay(500);
  for (uint16_t pulselen = 500; pulselen > 200; pulselen--) {
    pwm.setPWM(0, 0, pulselen);
  }
  */
  
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
  
  //delay(500);
  //for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
 ////   pwm.setPWM(2, 0, pulselen);
  //  pwm.setPWM(3, 0, pulselen);
   // pwm.setPWM(4, 0, pulselen);
   // pwm.setPWM(5, 0, pulselen);
 // }

 //

  //delay();

  //servonum ++;
  //if (servonum > 7) servonum = 0;
}
