#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <neopixel.cpp>
/*
  Piezo Sensor

  The circuit:
  - positive connection of the piezo attached to analog in 0
  - negative connection of the piezo attached to ground
  - 1 megohm resistor attached from analog in 0 to ground

*/

// these constants won't change:
const int ledPin = 12;      // LED connected to digital pin 12
const int piezoSensor = A0; // the piezo is connected to analog pin 0
const int panic = 800;  // above this level, max brightness rainbow
const int passive = 60; //below this level reduce brightness and slowly cycle colours
const int timeDelay = 100; //don't change the light effect too frequently
String prevReaction;
int prevSaturation = 55; //use this for fading things down
long prevHue = 55; //use this for fading things down

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 31

void LedStringComplete();

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines

NeoPatterns LedString(31, 12, NEO_GRB + NEO_KHZ800, &LedStringComplete);


// these variables will change:
int sensorReading = 0;      // variable to store the value read from the sensor pin
int brightness = 50;  //variable to store the brightness for reacting
long lastChange;
long currentMillis;
long changeDelay = 100;
int variableSpeed;
//Add in some stuff to

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)

// Declare the NeoPixel strip object:
// Adafruit_NeoPixel strip(LED_COUNT, ledPin, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(ledPin, OUTPUT); // declare the ledPin as as OUTPUT

  // strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  // strip.show();            // Turn OFF all pixels ASAP
  // strip.setBrightness(brightness); // Set BRIGHTNESS to about 1/2 (max = 255)
  //
  // Serial.begin(9600);       // use the serial port

  Serial.begin(115200);

    // Initialize the shiny lights
    LedString.begin();

    // Kick off a pattern
    LedString.TheaterChase(LedString.Color(0,100,100), LedString.Color(0,30,50), 50);
}

void loop() {

  currentMillis = millis();

  // read the sensor and store it in the variable sensorReading
  sensorReading = analogRead(piezoSensor);

  if((currentMillis - changeDelay) > lastChange){



    lastChange = currentMillis;

    // Switch patterns depending on input thresholds
    if(sensorReading <= passive) //just chillin
    {
        // Switch to FADE pattern
        LedString.ActivePattern = FADE;
        LedString.Interval = 300;
        changeDelay = 50;
        // Update the String.
        LedString.Update();

    }
    else if(sensorReading >= panic) //omg too much
    {

        LedString.ActivePattern = RAINBOW_CYCLE;
        LedString.Interval = 30;
        Serial.print(sensorReading);
        Serial.print("\t PANIC");
        Serial.println("");
        // Update the String.
        LedString.Update();
        // delay(5000);
        changeDelay = 5000;
    }
    else  // things are occurring and I don't know what I think about that
    {
      changeDelay = 50;
        LedString.ActivePattern = THEATER_CHASE;

        variableSpeed = 200 - ((sensorReading * 200) / 1023); //make the speed be 0-200 depending on the reading
        // Serial.print("variable speed");
        // Serial.print("\t");
        // Serial.print(sensorReading);
        // Serial.print("\t");
        // Serial.print(variableSpeed);
        // Serial.println("");
        LedString.Interval = variableSpeed;
        // Update the String.
        LedString.Update();

    }
  }



}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Stick Completion Callback
void LedStringComplete()
{
    // Random color change for next scan
    LedString.Color1 = LedString.Wheel(random(255));
}
