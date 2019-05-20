#include <VL53L0X.h>                                        // https://github.com/ScruffR/VL53L0X
VL53L0X handSensor;                                         // The constructor.

void initHandSensor() {
  handSensor.init();                                        // Initialise the sensor.
  handSensor.setTimeout(0);                                 // Sets a timeout period in milliseconds after which read operations will abort if the sensor is not ready. A value of 0 disables the timeout.
  handSensor.startContinuous();                             // Take measurements continuously, as fast as possible.
  attachInterrupt(digitalPinToInterrupt(2), sensorInterrupt, FALLING); // When measurement is done, an interrupt is fired (active low).
}

volatile bool haveReading;
void sensorInterrupt() {
  haveReading = true;
}

void handleHandSensor() {
  static uint32_t latestReading;
  if (haveReading) {                                        // We got an interrupt, a new reading is complete.
    haveReading = false;
    getReading();                                           // Read the sensor's result. This resets the interrupt state.
    analyseMotion(motionState);
    latestReading = millis();                               // Record when we had the reading - indicating the sensor is alive and well.
  }
  if (millis() - latestReading > 1000) {                    // If one second no readings sensor is probably not connected: 
    initHandSensor();                                       // Try to restart the sensor, maybe it's been (re)connected already.
    latestReading = millis();                               // Keep track of when this happened, so a second later we try again.
  }
}

void analyseMotion(uint8_t &state) {

  // Hand presented/removed.
  bool handPresented = true;

  // Check whether a hand was present or not during the latest three readings.
  for (uint8_t i = 0; i < 3; i++) {
    if (readings[i] > MAX_DISTANCE) {                       // No hand present.
      handPresented = false;
    }
  }

  // Check whether the opposite is true for the previous three readings.
  for (uint8_t i = 3; i < 6; i++) {
    if (handPresented) {
      if (readings[i] < MAX_DISTANCE) {                     // It was there already; not presented now.
        handPresented = false;
      }
    }
  }
  bitWrite(state, HAND_PRESENTED, handPresented);
}

void getReading() {
  uint16_t reading = handSensor.readRangeContinuousMillimeters();
  for (uint8_t i = N_READINGS - 1; i > 0; i--) {
    readings[i] = readings [i - 1];                         // Move all values one up.
  }
  readings[0] = reading;
}
