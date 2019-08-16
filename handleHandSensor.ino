#include <VL53L0X.h>                                        // https://github.com/ScruffR/VL53L0X
VL53L0X handSensor;                                         // The constructor.

void initHandSensor() {
  handSensor.init();                                        // Initialise the sensor.
  handSensor.setTimeout(0);                                 // Sets a timeout period in milliseconds after which read operations will abort if the sensor is not ready. A value of 0 disables the timeout.
  handSensor.startContinuous();                             // Take measurements continuously, as fast as possible.
  pinMode(2, INPUT_PULLUP);                                 // Make sure we don't leave it floating.
  attachInterrupt(digitalPinToInterrupt(2), sensorInterrupt, FALLING); // When measurement is done, an interrupt is fired (active low).
}

volatile bool haveReading;
void sensorInterrupt() {
  haveReading = true;
}

void handleHandSensor() {
  if (haveReading) {                                        // We got an interrupt, a new reading is complete.
    haveReading = false;
    getReading();                                           // Read the sensor's result. This resets the interrupt state.
    analyseMotion(motionState);
  }
//  static uint32_t lastPrint;
//  if (millis() - lastPrint > 500) {
//    lastPrint  = millis();
//    Serial.print(F("Distance: "));
//    Serial.print(readings[0]);
//    Serial.println(F(" mm."));
//  }
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
