#include <Wire.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <FastLED.h>

//#define TEST_PROX_SENSOR

const uint8_t LED_PIN = 5;
const uint8_t PROXIMITY_SENSOR_PIN = A1;
const uint8_t BRIGHTNESS_SENSOR_PIN = A0;

SoftwareSerial mySoftwareSerial(8, 7); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// Bits to set when a specific movement is detected.
#define HAND_PRESENTED  2                                   // Hand presented above the sensor.
uint8_t motionState;
bool musicPlaying;

const uint8_t N_READINGS = 20;                              // We keep the latest 20 readings for hand gesture detection.
uint16_t readings[N_READINGS];
const uint16_t MAX_DISTANCE = 750;                          // If distance greater than this number (in mm), no hand present.
const uint16_t MIN_DISTANCE = 50;                           // If distance less than this number (in mm), sensor is covered.

const uint32_t PROXIMITY_TIMEOUT = 15000;                   // How long to remain "active" after motion sensor says there's no movement nearby.
const uint32_t MUSIC_PLAYING_TIMEOUT = 30000;               // How long to remain "active" after motion sensor says there's no movement nearby, and we have music playing.
bool proximityDetected = false;

uint8_t musicVolume;                                        // The volume of the music: 0-30.

uint16_t activeFadeSpeed, transitionSpeed;

// Colour when the pole is idle - not fading. Same for all poles.
uint8_t idleRed = 255;
uint8_t idleGreen = 255;
uint8_t idleBlue = 255;

// Colours when music plays - not fading. Set through the menu pole by pole.
uint8_t musicRed, musicGreen, musicBlue;

char buf[80];

void setup() {
  Serial.begin(115200);
  Wire.begin();
  readEEPROM();
  initLEDs();
  initProximitySensor();
  initHandSensor();
  initBrightnessSensor();
  initMusic();
}

void loop() {
  handleHandSensor();
  handleMusic();
  handleProximitySensor();
  handleBrightnessSensor();
  handleLEDs(); 
  handleSerialUI();
}
