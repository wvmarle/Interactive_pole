#include <FastLED.h>
const uint8_t NUM_LEDS = 120;                               // The total number of LEDs in the strip (or a little less than this, as they're all the same colour anyway).
CRGB leds[NUM_LEDS];                                        // Define the array of leds
uint8_t red, green, blue;                                   // The current colours.

enum LEDStates {
  LED_IDLE,                                                 // Show idle.
  LED_TRANSITION_TO_ACTIVE,                                 // Fade to active colour [0].
  LED_TRANSITION_TO_MUSIC,                                  // Fade to music playing colour.
  LED_TRANSITION_TO_IDLE,                                   // Fade to idle colour.
  LED_ACTIVE_FADE_UP,                                       // Fade from active colour [0] to active colour [1].
  LED_ACTIVE_FADE_DOWN,                                     // Fade from active colour [1] to active colour [0].
  LED_MUSIC,                                                // Show music playing colour.
};
LEDStates LEDState = LED_IDLE;

void initLEDs() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  if (proximityDetected) {                                  // We set the "wrong" transition here to trigger fading.
    LEDState = LED_TRANSITION_TO_IDLE;
  }
  else {
    LEDState = LED_TRANSITION_TO_ACTIVE;
  }
  CRGB rgb = CRGB(0, 0, 0);                                 // Switch off the LEDs.
  FastLED.showColor(rgb);
}

void handleLEDs() {
  static uint32_t lastFadeTime;                             // When a fade cycle (0 to 1 to 0) started.
  static uint8_t oldRed, oldGreen, oldBlue;                 // Old colours - when doing transition fade.
  static uint8_t fadeStep;
  static uint16_t fadeSpeed = activeFadeSpeed;
  static uint8_t fadeRed;
  static uint8_t fadeGreen;
  static uint8_t fadeBlue;
  static uint8_t nSteps;
  static bool oldProximityDetected;
  static bool oldMusicPlaying;

  // Handle proximity sensor state.
  if (proximityDetected != oldProximityDetected &&          // Change of state; act upon this,
      musicPlaying == false) {                              // but only when we're not currently playing music!
    oldProximityDetected = proximityDetected;
    lastFadeTime = millis();                                // Starting a fade event.
    oldRed = red;                                           // Keep track of where we started.
    oldGreen = green;
    oldBlue = blue;
    if (proximityDetected) {                                // Become active.
      LEDState = LED_ACTIVE_FADE_UP;
      fadeSpeed = activeFadeSpeed;
    }
    else if (musicPlaying == false) {
      LEDState = LED_TRANSITION_TO_IDLE;
      fadeSpeed = transitionSpeed;
      fadeRed = idleRed;
      fadeGreen = idleGreen;
      fadeBlue = idleBlue;
    }
    fadeStep = 0;
    fadeSpeed = transitionSpeed;                            // How long the transition should take.
  }

  // Handle start/stop of music playing.
  if (musicPlaying != oldMusicPlaying) {
    oldMusicPlaying = musicPlaying;
    oldRed = red;                                           // Keep track of where we started.
    oldGreen = green;
    oldBlue = blue;
    if (musicPlaying) {                                     // Music just started.
      LEDState = LED_TRANSITION_TO_MUSIC;                   // Transition fade to music colour.
      fadeRed = musicRed;
      fadeGreen = musicGreen;
      fadeBlue = musicBlue;
      fadeSpeed = transitionSpeed;                          // How long the transition should take.
    }
    else if (proximityDetected) {                           // Music just stopped, still have proximity.
      LEDState = LED_ACTIVE_FADE_DOWN;                      // Transition fade to active colour.
      fadeSpeed = activeFadeSpeed;                          // How long the transition should take.
    }
    else {
      LEDState = LED_TRANSITION_TO_IDLE;                    // Transition fade to idle colour.
      fadeRed = idleRed;
      fadeGreen = idleGreen;
      fadeBlue = idleBlue;
      fadeSpeed = transitionSpeed;                          // How long the transition should take.
    }
    fadeStep = 0;
  }
  nSteps = min(fadeSpeed / 50, 255);                        // Don't fade too fast, but also no more than 256 total steps. It takes about 32 ms to transmit data to 100 LEDs!
  uint16_t fadeStepTime;
  if (nSteps == 255) {
    fadeStepTime = fadeSpeed / nSteps;
  }
  else {
    fadeStepTime = 50;
  }
  if (millis() - lastFadeTime > (float)fadeStep * (float)fadeStepTime) {
    fadeStep++;
    switch (LEDState) {
      case LED_IDLE:
        break;

      case LED_TRANSITION_TO_IDLE:
      case LED_TRANSITION_TO_ACTIVE:
      case LED_TRANSITION_TO_MUSIC:
        setLEDs(oldRed, fadeRed,
                oldGreen, fadeGreen,
                oldBlue, fadeBlue,
                fadeStep, nSteps);
        if (fadeStep == nSteps) {                           // We reached the end of the sequence.
          if (LEDState == LED_TRANSITION_TO_IDLE) {
            LEDState = LED_IDLE;
            setLEDs(idleRed, 0,
                    idleGreen, 0,
                    idleBlue, 0,
                    0, 0);
          }
          else if (LEDState == LED_TRANSITION_TO_ACTIVE) {
            LEDState = LED_ACTIVE_FADE_UP;
            fadeSpeed = activeFadeSpeed;                    // How long the transition should take.
          }
          else if (LEDState == LED_TRANSITION_TO_MUSIC) {
            LEDState = LED_MUSIC;
            setLEDs(musicRed, 0,
                    musicGreen, 0,
                    musicBlue, 0,
                    0, 0);
          }
        }
        break;

      case LED_ACTIVE_FADE_UP:
        setLEDs(idleRed, musicRed,
                idleGreen, musicGreen,
                idleBlue, musicBlue,
                fadeStep, nSteps);
        if (fadeStep == nSteps) {
          LEDState = LED_ACTIVE_FADE_DOWN;                // Start fading down (colour 1 to 0).
          fadeSpeed = activeFadeSpeed;
          fadeStep = 0;
          lastFadeTime = millis();
        }
        break;

      case LED_ACTIVE_FADE_DOWN:
        setLEDs(musicRed, idleRed,
                musicGreen, idleGreen,
                musicBlue, idleBlue,
                fadeStep, nSteps);
        if (fadeStep == nSteps) {
          LEDState = LED_ACTIVE_FADE_UP;                    // Start fading up (colour 0 to 1).
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
  if (nSteps == 0) {                                        // No fading; just set the LEDs to a colour.
    red = fromRed;
    green = fromGreen;
    blue = fromBlue;
  }
  else {                                                    // Fade in progress: calculate the new colour.
    red = (float)fromRed + ((float)toRed - (float)fromRed) * (float)fadeStep / (float)nSteps;
    green = (float)fromGreen + ((float)toGreen - (float)fromGreen) * (float)fadeStep / (float)nSteps;
    blue = (float)fromBlue + ((float)toBlue - (float)fromBlue) * (float)fadeStep / (float)nSteps;
  }
  CRGB rgb = CRGB(red, green, blue);
  //#ifdef TEST_PROX_SENSOR
  //  for (uint8_t i = 0; i < NUM_LEDS - 10; i++) {
  //    leds[i] = rgb;
  //  }
  //  if (digitalRead(PROXIMITY_SENSOR_PIN)) {
  //    rgb = CRGB(255, 0, 0);
  //  }
  //  else {
  //    rgb = CRGB(0, 255, 0);
  //  }
  //  for (uint8_t i = NUM_LEDS - 10; i < NUM_LEDS; i++) {
  //    leds[i] = rgb;
  //  }
  //  FastLED.show();
  //#else
  FastLED.showColor(rgb);
  //#endif
}
