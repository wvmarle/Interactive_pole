#include <FastLED.h>
const uint8_t NUM_LEDS = 114;                               // The total number of LEDs in the strip (or a little less than this, as they're all the same colour anyway).
CRGB leds[NUM_LEDS];                                        // Define the array of leds
uint8_t red, green, blue;                                   // The current colours.

enum LEDStates {
  LED_IDLE_FADE_UP,                                         // Fade from idle colour [0] to idle colour [1]
  LED_IDLE_FADE_DOWN,                                       // Fade from idle colour [1] to idle colour [0]
  LED_ACTIVE_TRANSITION_IN,                                 // Fade to active colour [0]
  LED_ACTIVE_FADE_UP,                                       // Fade from active colour [0] to active colour [1]
  LED_ACTIVE_FADE_DOWN,                                     // Fade from active colour [1] to active colour [0]
  LED_ACTIVE_TRANSITION_OUT,                                // Fade to idle colour [0]
};
LEDStates LEDState = LED_IDLE_FADE_DOWN;

void initLEDs() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  if (proximityDetected) {                                  // We set the "wrong" transition here to trigger fading.
    LEDState = LED_ACTIVE_TRANSITION_OUT;
  }
  else {
    LEDState = LED_ACTIVE_TRANSITION_IN;
  }
  CRGB rgb = CRGB(0, 0, 0);                                 // Switch off the LEDs.
  FastLED.showColor(rgb);
}

void handleLEDs() {
  static uint32_t lastFadeTime;                             // When a fade cycle (0 to 1 to 0) started.
  static uint8_t oldRed, oldGreen, oldBlue;                 // Old colours - when doing transition fade.
  static uint8_t fadeStep;
  static uint16_t fadeSpeed = idleFadeSpeed;
  static uint8_t fadeRed[2] = {idleRed[0], idleRed[1]};
  static uint8_t fadeGreen[2] = {idleGreen[0], idleGreen[1]};
  static uint8_t fadeBlue[2] = {idleBlue[0], idleBlue[1]};
  static uint8_t nSteps;
  static bool oldProximityDetected;

  // Handle proximity sensor state.
  if (proximityDetected != oldProximityDetected) {          // Change of state; act upon this.
    oldProximityDetected = proximityDetected;
    lastFadeTime = millis();                                // Starting a fade event: to active colour.
    oldRed = red;                                           // Keep track of where we started.
    oldGreen = green;
    oldBlue = blue;
    if (proximityDetected) {
      LEDState = LED_ACTIVE_TRANSITION_IN;
      memcpy(fadeRed, activeRed, 2);                        // The new colours between which to cycle.
      memcpy(fadeBlue, activeBlue, 2);
      memcpy(fadeGreen, activeGreen, 2);
    }
    else {
      LEDState = LED_ACTIVE_TRANSITION_OUT;
      memcpy(fadeRed, idleRed, 2);                          // The new colours between which to cycle.
      memcpy(fadeBlue, idleBlue, 2);
      memcpy(fadeGreen, idleGreen, 2);
    }
    fadeStep = 0;
    fadeSpeed = transitionSpeed;                            // How long the transition should take.
  }
                                                            
  nSteps = min(fadeSpeed / 50, 255);                        // Don't fade too fast, but also no more than 256 total steps. It takes about 32 ms to transmit data to 100 LEDs!
  if (millis() - lastFadeTime > (float)fadeStep * (float)fadeSpeed / (float)nSteps) {
    fadeStep++;
    switch (LEDState) {
      case LED_ACTIVE_TRANSITION_OUT:
      case LED_ACTIVE_TRANSITION_IN:
        setLEDs(oldRed, fadeRed[0],
                oldGreen, fadeGreen[0],
                oldBlue, fadeBlue[0],
                fadeStep, nSteps);
        if (fadeStep == nSteps) {                           // We reached the end of the sequence.
          if (LEDState == LED_ACTIVE_TRANSITION_OUT) {      // We just transitioned to IDLE state.
            LEDState = LED_IDLE_FADE_UP;
            fadeSpeed = idleFadeSpeed / 2;                  // The pace at which we do the idle fading up/down.
          }
          else {                                            // We just transitioned to ACTIVE state.
            LEDState = LED_ACTIVE_FADE_UP;
            fadeSpeed = activeFadeSpeed / 2;                // The pace at which we do the fading up/down.
          }
          fadeStep = 0;
          lastFadeTime = millis();
        }
        break;

      case LED_IDLE_FADE_UP:
      case LED_ACTIVE_FADE_UP:
        setLEDs(fadeRed[0], fadeRed[1],
                fadeGreen[0], fadeGreen[1],
                fadeBlue[0], fadeBlue[1],
                fadeStep, nSteps);
        if (fadeStep == nSteps) {
          if (LEDState == LED_IDLE_FADE_UP) {               // We're doing IDLE fading.
            LEDState = LED_IDLE_FADE_DOWN;                  // Start fading down (colour 1 to 0).
          }
          else {                                            // We're doing ACTIVE fading.
            LEDState = LED_ACTIVE_FADE_DOWN;                // Start fading down (colour 1 to 0).
          }
          fadeStep = 0;
          lastFadeTime = millis();
        }
        break;

      case LED_IDLE_FADE_DOWN:
      case LED_ACTIVE_FADE_DOWN:
        setLEDs(fadeRed[1], fadeRed[0],
                fadeGreen[1], fadeGreen[0],
                fadeBlue[1], fadeBlue[0],
                fadeStep, nSteps);
        if (fadeStep == nSteps) {
          if (LEDState == LED_IDLE_FADE_DOWN) {             // We're doing IDLE fading.
            LEDState = LED_IDLE_FADE_UP;                    // Start     (colour 0 to 1).
          }
          else {                                            // We're doing ACTIVE fading.
            LEDState = LED_ACTIVE_FADE_UP;                  // Start fading up (colour 0 to 1).
          }
          fadeStep = 0;
          lastFadeTime = millis();
        }
        break;
    }
  }
}

void setLEDs(uint8_t fromRed, uint8_t toRed,
             uint8_t fromGreen, uint8_t toGreen,
             uint8_t fromBlue, uint8_t toBlue,
             uint8_t fadeStep, uint8_t nSteps) {
  red = (float)fromRed + ((float)toRed - (float)fromRed) * (float)fadeStep / (float)nSteps;
  green = (float)fromGreen + ((float)toGreen - (float)fromGreen) * (float)fadeStep / (float)nSteps;
  blue = (float)fromBlue + ((float)toBlue - (float)fromBlue) * (float)fadeStep / (float)nSteps;
  CRGB rgb = CRGB(red, green, blue);
  FastLED.showColor(rgb);
}
