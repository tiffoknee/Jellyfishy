#include <Adafruit_NeoPixel.h>

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)


// set to pin connected to data input of WS8212 (NeoPixel) strip
#define PIN         0

// set to pin connected to piezo
#define PIEZOPIN    1

// any pin with analog input (used to initialize random number generator)
#define RNDPIN      2 //use P3 (A2)

// number of LEDs (NeoPixels) in your strip
// (please note that you need 3 bytes of RAM available for each pixel)
#define NUMPIXELS   31

// max LED brightness (1 to 255) – start with low values!
// (please note that high brightness requires a LOT of power)
#define BRIGHTNESS  60

// set to 1 to display FPS rate
#define DEBUG       0

// if true, wrap color wave over the edge (used for circular stripes)
#define WRAP        1

//below this level reduce brightness and slowly cycle colours
#define PASSIVE 60

//don't change the light effect too frequently
#define CHANGEDELAY 1000

// above this level, top speed, rainbow
#define PANIC 800


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// we have 3 color spots (reg, green, blue) oscillating along the strip with different speeds

// increase focus to get narrow spots, decrease to get wider spots
// decrease speed to speed up, increase to slow down (it's not a delay actually)

float spdr, spdg, spdb;
float offset, speed, focus;

//use these to turn off a blob and only have two colours
bool liveR = false;
bool liveB = true;
bool liveG = true; //use these to turn on and off colours..

long currentMillis, lastChange;

int sensorReading = 0;

#if DEBUG
// track fps rate
long nextms = 0;
int pfps = 0, fps = 0;
#endif

// the real exponent function is too slow, so I created an approximation (only for x < 0)
float myexp(float x) {
  return (1.0/(1.0-(0.634-1.344*x)*x));
}


void setup() {

  focus = 80;
  speed = 12000;
  liveR = false;
  liveB = true;
  liveG = true;

  // initialize pseudo-random number generator with some random value
  randomSeed(analogRead(RNDPIN));

  // assign random speed to each spot
  spdr = 1.0 + random(200) / 100.0;
  spdg = 1.0 + random(200) / 100.0;
  spdb = 1.0 + random(200) / 100.0;

  // set random offset so spots start in random locations
  offset = random(10000) / 100.0;

  // initialize LED strip
  strip.begin();
  strip.show();
}

void loop() {

  currentMillis = millis();

  //get the largest reading since last triggered
  sensorReading = (analogRead(PIEZOPIN) > sensorReading) ? analogRead(PIEZOPIN) : sensorReading;

  if((currentMillis - CHANGEDELAY) > lastChange){

    // read the sensor and store it in the variable sensorReading
    sensorReading = analogRead(PIEZOPIN);

    lastChange = currentMillis;

    // Switch patterns depending on input thresholds
    if(sensorReading <= PASSIVE) //just chillin
    {

      focus = 80;
      speed = 12000;
      liveR = false;
      liveB = true;
      liveG = true;

    }
    else if(sensorReading >= PANIC) //omg too much
    {
      focus = 40;
      speed = 600;
      liveR = true;
      liveB = true;
      liveG = true;
    }
    else  // things are occurring and I don't know what I think about that
    {
      focus = 80;
      speed = 8000 - ((sensorReading * 8000) / 1023); //make the speed be 0-200 depending on the reading;
      // liveR = (rand() % 100) < 33.33;
      // liveB = (rand() % 100) < 33.33;
      // liveG = (rand() % 100) < 33.33;

    }
  }

  // use real time to recalculate position of each color spot
  long ms = millis();
  // scale time to float value
  float m = offset + (float)ms/speed;
  // add some non-linearity
  m = m - 42.5*cos(m/552.0) - 6.5*cos(m/142.0);

  // recalculate position of each spot (measured on a scale of 0 to 1)
  float posr = 0.15 + 0.55*sin(m*spdr);
  float posg = 0.50 + 0.65*sin(m*spdg);
  float posb = 0.85 + 0.75*sin(m*spdb);

  // now iterate over each pixel and calculate it's color
  for (int i=0; i<NUMPIXELS; i++) {
    // pixel position on a scale from 0.0 to 1.0
    float ppos = (float)i / NUMPIXELS;

    // distance from this pixel to the center of each color spot
    float dr = ppos-posr;
    float dg = ppos-posg;
    float db = ppos-posb;
#if WRAP
    dr = dr - floor(dr + 0.5);
    dg = dg - floor(dg + 0.5);
    db = db - floor(db + 0.5);
#endif

    // set each color component from 0 to max BRIGHTNESS, according to Gaussian distribution
    // strip.setPixelColor(i,
    //   constrain(BRIGHTNESS*myexp(-focus*dr*dr),0,BRIGHTNESS),
    //   constrain(BRIGHTNESS*myexp(-focus*dg*dg),0,BRIGHTNESS),
    //   constrain(BRIGHTNESS*myexp(-focus*db*db),0,BRIGHTNESS)
    //   );
    strip.setPixelColor(i,
      liveR ? constrain(BRIGHTNESS*myexp(-focus*dr*dr),0,BRIGHTNESS) : 0,
      liveB ? constrain(BRIGHTNESS*myexp(-focus*dg*dg),0,BRIGHTNESS) : 0,
      liveG ? constrain(BRIGHTNESS*myexp(-focus*db*db),0,BRIGHTNESS) : 0
    );
  }

#if DEBUG
  // keep track of FPS rate
  fps++;
  if (ms>nextms) {
    // 1 second passed – reset counter
    nextms = ms + 1000;
    pfps = fps;
    fps = 0;
  }
  // show FPS rate by setting one pixel to white
  strip.setPixelColor(pfps,BRIGHTNESS,BRIGHTNESS,BRIGHTNESS);
#endif

  // send data to LED strip
  strip.show();
}
