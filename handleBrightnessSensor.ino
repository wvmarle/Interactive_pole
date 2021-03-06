const uint8_t MINIMUM_BRIGHTNESS = 35;                      // Minimum LED brightness. Absolute minimum 25 to keep the power banks on (>65 mA current).
const uint8_t MAXIMUM_BRIGHTNESS = 220;                     // Maximum LED brightness (100% = 255; have to limit this a bit due to available current).

const uint16_t MINIMUM_READING = 10;                        // Brightness level where LED brightness should reach minimum.
const uint16_t MAXIMUM_READING = 400;                       // Brightness level where LED brightness should reach maximum.

const uint16_t READING_FREQUENCY = 100;                     // Time in milliseconds between brightness readings.
const uint16_t READINGS = 64;                               // Number of readings over which to average.

void initBrightnessSensor() {
  FastLED.setBrightness(200);
}

void handleBrightnessSensor() {
  static uint32_t lastBrightnessReading;
  static uint16_t brightnessReadings[READINGS];
  static uint8_t readingIndex;
  uint32_t totalReadings = 0;
  if (millis() - lastBrightnessReading > READING_FREQUENCY) {
    lastBrightnessReading += READING_FREQUENCY;
    brightnessReadings[readingIndex] = analogRead(BRIGHTNESS_SENSOR_PIN);
    for (uint8_t i = 0; i < READINGS; i++) {
      totalReadings += brightnessReadings[i];
    }

    // Calculate the average brightness over the 64 measurements, then make sure
    // this value is within the target brightness range, and finally map it to the
    // actual brightness the LEDs should have.
    uint16_t averageBrightness = constrain(totalReadings / READINGS, MINIMUM_READING, MAXIMUM_READING);
    uint8_t LEDBrightness = map(averageBrightness, MINIMUM_READING, MAXIMUM_READING, MINIMUM_BRIGHTNESS, MAXIMUM_BRIGHTNESS);
    FastLED.setBrightness(LEDBrightness);
    readingIndex++;
    if (readingIndex == READINGS) {
      readingIndex = 0;
    }
  }
}
