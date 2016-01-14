// Audio Spectrum Display
// Copyright 2013 Tony DiCola (tony@tonydicola.com)

// This code is part of the guide at http://learn.adafruit.com/fft-fun-with-fourier-transforms/

#define ARM_MATH_CM4
#include <arm_math.h>
#include <Adafruit_NeoPixel.h>


////////////////////////////////////////////////////////////////////////////////
// CONIFIGURATION 
// These values can be changed to alter the behavior of the spectrum display.
////////////////////////////////////////////////////////////////////////////////

int SAMPLE_RATE_HZ = 9000;             // Sample rate of the audio in hertz.
float SPECTRUM_MIN_DB = 30.0;          // Audio intensity (in decibels) that maps to low LED brightness.
float SPECTRUM_MAX_DB = 60.0;          // Audio intensity (in decibels) that maps to high LED brightness.
const int FFT_SIZE = 256;              // Size of the FFT.  Realistically can only be at most 256 
                                       // without running out of memory for buffers and other state.
const int AUDIO_INPUT_PIN = 0;        // Input ADC pin for audio data.
const int ANALOG_READ_RESOLUTION = 10; // Bits of resolution for the ADC.
const int ANALOG_READ_AVERAGING = 16;  // Number of samples to average with each ADC reading.
const int POWER_LED_PIN = 13;          // Output pin for power LED (pin 13 to use Teensy 3.0's onboard LED).
const int ARDUINO_SIGNAL_PIN = 1; //PIN_D1

const int THRESHOLD_VOICE_NORMAL = 1000000000;
const int THRESHOLD_VOICE_HAPPY = 1500000000;
const int THRESHOLD_VOICE_IMPASSIONED = 2000000000;
const int WINDOW_US = 1000000;

int LEDS_ENABLED = 1;
const int NEO_PIXEL_PIN = 3;
const int NEO_PIXEL_COUNT = 1;

////////////////////////////////////////////////////////////////////////////////
// INTERNAL STATE
// These shouldn't be modified unless you know what you're doing.
////////////////////////////////////////////////////////////////////////////////

IntervalTimer samplingTimer;
IntervalTimer moodTimer;
float samples[FFT_SIZE*2];
float magnitudes[FFT_SIZE];
int sampleCounter = 0;

IntervalTimer colorChangeTimer;
volatile int currR=0;
volatile int currG=255;
volatile int currB=0;
volatile int nextR=0;
volatile int nextG=255;
volatile int nextB=0;

const int pin_low = 11;
const int pin_hi = 12;
#define SAD 0
#define NEUTRAL 1
#define HAPPY 2
#define IMPASSIONED 3
volatile int mood_nxt = SAD;
int mood[4][2] = {{LOW, LOW}, {LOW, HIGH}, {HIGH, LOW}, {HIGH, HIGH}};
float hues[NEO_PIXEL_COUNT];
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_PIXEL_COUNT, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
 
////////////////////////////////////////////////////////////////////////////////
// MAIN SKETCH FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void setup() {
  // Set up serial port.
  Serial.begin(38400);
  
  // Set up ADC and audio input.
  pinMode(AUDIO_INPUT_PIN, INPUT);
  analogReadResolution(ANALOG_READ_RESOLUTION);
  analogReadAveraging(ANALOG_READ_AVERAGING);
  
  // Turn on the power indicator LED.
  pinMode(POWER_LED_PIN, OUTPUT);
  digitalWrite(POWER_LED_PIN, HIGH);

  //Set up input pin from Arduino that tells us when we need to wait before changing output
  pinMode(ARDUINO_SIGNAL_PIN, INPUT_PULLUP); //set to INPUT_PULLUP so it'll still work even when not connected to arduino. should be safe since we're going through a level-shifting transistor anyway between the two devices. may need to tweak polarity accordingly, or may even be able to remove this entirely depending on that transistor structure (since it'll probably just be a NOT gate with a 3.3V Vdd and 0-5V Vgs.

  //Set up the two interrupts
  colorChangeTimer.priority(3);
  samplingTimer.priority(2);
  moodTimer.priority(1);
  pinMode(pin_low, OUTPUT);
  pinMode(pin_hi, OUTPUT);
  digitalWrite(pin_low, mood[mood_nxt][0]);
  digitalWrite(pin_hi, mood[mood_nxt][1]);
  
  // Begin sampling audio
  samplingBegin();

  //Start mood timer
  moodBegin();

  //Initialize neo pixels
  pixels.begin();
  pixels.show();
}

void loop() {
  // Calculate FFT if a full sample is available.
  if (samplingIsDone()) {
    // Run FFT on sample data.
    arm_cfft_radix4_instance_f32 fft_inst;
    arm_cfft_radix4_init_f32(&fft_inst, FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&fft_inst, samples);
    // Calculate magnitude of complex numbers output by the FFT.
    arm_cmplx_mag_f32(samples, magnitudes, FFT_SIZE);
    int mymood=mood_nxt;
    Serial.println(stdBin());

    //Compute mood_nxt
    calcMood();

    //Update neopixel
    //pixels.setPixelColor(0,255,0,0);
    //pixels.show();
  
    // Restart audio sampling.
    samplingBegin();
  }

}


////////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

// Compute the average magnitude of a target frequency window vs. all other frequencies.
void windowMean(float* magnitudes, int lowBin, int highBin, float* windowMean, float* otherMean) {
    *windowMean = 0;
    *otherMean = 0;
    // Notice the first magnitude bin is skipped because it represents the
    // average power of the signal.
    for (int i = 1; i < FFT_SIZE/2; ++i) {
      if (i >= lowBin && i <= highBin) {
        *windowMean += magnitudes[i];
      }
      else {
        *otherMean += magnitudes[i];
      }
    }
    *windowMean /= (highBin - lowBin) + 1;
    *otherMean /= (FFT_SIZE / 2 - (highBin - lowBin));
}

// Convert a frequency to the appropriate FFT bin it will fall within.
int frequencyToBin(float frequency) {
  float binFrequency = float(SAMPLE_RATE_HZ) / float(FFT_SIZE);
  return int(frequency / binFrequency);
}

////////////////////////////////////////////////////////////////////////////////
// SAMPLING FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

void samplingCallback() {
  // Read from the ADC and store the sample data
  samples[sampleCounter] = (float32_t)analogRead(AUDIO_INPUT_PIN);
  // Complex FFT functions require a coefficient for the imaginary part of the input.
  // Since we only have real data, set this coefficient to zero.
  samples[sampleCounter+1] = 0.0;
  // Update sample buffer position and stop after the buffer is filled
  sampleCounter += 2;
  if (sampleCounter >= FFT_SIZE*2) {
    samplingTimer.end();
  }
}

void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate.
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000/SAMPLE_RATE_HZ);
}

boolean samplingIsDone() {
  return sampleCounter >= FFT_SIZE*2;
}

void moodCallback() {
  //first check input pin from Arduino to see if this should even execute
  if (digitalRead(ARDUINO_SIGNAL_PIN)) { //pin should be high when arduino disconnected or when color change is active, low when we don't want any state change
    digitalWrite(pin_low, mood[mood_nxt][0]);
    digitalWrite(pin_hi, mood[mood_nxt][1]);
    setNextMoodColor(mood_nxt);
    colorChangeTimer.begin(colorChangeCallback, 1500); //one step per 1.5ms, this means the color fade *should* never exceed the measurement period. may want to fine-tune for visual effect.
    mood_nxt = SAD;
  }
  //else do nothing
}

void colorChangeCallback() {
  if (currR==nextR && currG==nextG && currB==nextB) {
    colorChangeTimer.end();
    return;
  }
  else {
    if (currR != nextR) {
      if (currR > nextR) currR--;
      else currR++;
    }
    if (currG != nextG) {
      if (currG > nextG) currG--;
      else currG++;
    }
    if (currB != nextB) {
      if (currB > nextB) currB--;
      else currB++;
    }
    for (int i=0; i<NEO_PIXEL_COUNT; i++) {
      pixels.setPixelColor(i, currR, currG, currB);
    }
    pixels.show();
  }
}

void setNextMoodColor(int mood) {
  switch (mood) {
    case SAD:
      nextR=0;
      nextG=255;
      nextB=0;
      break;
    case NEUTRAL:
      nextR=255;
      nextG=255;
      nextB=0;
      break;
    case HAPPY:
      nextR=255;
      nextG=127;
      nextB=0;
      break;
    case IMPASSIONED:
      nextR=255;
      nextG=0;
      nextB=0;
      break;
  }
}

void moodBegin() {
  moodTimer.begin(moodCallback, WINDOW_US); //once per second
}

////////////////////////////////////////////////////////////////////////////////
// Frequency table functions
////////////////////////////////////////////////////////////////////////////////

//Get fundamental frequency f0 by finding the mode frequency bin
int fundamental(){
  int i = 1;
  float currMax = magnitudes[1];
  int currMaxIdx = 0;
  for(i=2;i<128;i++){
    if(magnitudes[i] > currMax){
      currMaxIdx = i;
      currMax = magnitudes[i];
    }
  }
  return currMaxIdx;
}

//Get mean frequency bin
int meanBin(){
  int sum=0;
  int count=0;
  for (int i=1; i<256; i++) {
    sum+=i*magnitudes[i];
    count+=magnitudes[i];
  }
  return sum/count;
}

//Get standard deviation
float stdBin(){
  float variance=0;
  int mean=meanBin();
  
  for (int i=1; i<256; i++) {
    if (i==mean) continue;
    else {
      variance+=magnitudes[i]*(i-mean)*(i-mean);
    }
  }

  return variance;
}

void calcMood() {
  float std = stdBin();
  if (mood_nxt==IMPASSIONED || std > THRESHOLD_VOICE_IMPASSIONED) {
    mood_nxt = IMPASSIONED;
    return;
  }
  else if (mood_nxt==HAPPY || std > THRESHOLD_VOICE_HAPPY) {
    mood_nxt=HAPPY;
    return;
  }
  else if (mood_nxt==NEUTRAL || std > THRESHOLD_VOICE_NORMAL) {
    mood_nxt=NEUTRAL;
    return;
  }
  else mood_nxt=SAD;
  return;
}

