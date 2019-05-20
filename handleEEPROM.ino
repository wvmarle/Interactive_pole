#include <EEPROM.h>

void readEEPROM() {
  EEPROM.get(0, activeFadeSpeed);                           // uint16_t
  EEPROM.get(2, idleFadeSpeed);                             // uint16_t
  EEPROM.get(4, idleRed);                                   // uint8_t [2]
  EEPROM.get(6, idleGreen);                                 // uint8_t [2]
  EEPROM.get(8, idleBlue);                                  // uint8_t [2]
  EEPROM.get(10, transitionSpeed);                          // uint16_t
  EEPROM.get(14, activeRed);                                // uint8_t [2]
  EEPROM.get(16, activeGreen);                              // uint8_t [2]
  EEPROM.get(18, activeBlue);                               // uint8_t [2]
  EEPROM.get(20, musicVolume);                              // uint8_t
  if (musicVolume > 30) {                                   // Fresh EEPROM, or just messed up settings. Set defaults.
    activeFadeSpeed = 6000;
    idleFadeSpeed = 8000;
    transitionSpeed = 2000;
    idleRed[0] = 255;
    idleGreen[0] = 255;
    idleBlue[0] = 255;
    idleRed[1] = 221;
    idleGreen[1] = 127;
    idleBlue[1] = 255;
    activeRed[0] = 255;
    activeGreen[0] = 84;
    activeBlue[0] = 28;
    activeRed[1] = 39;
    activeGreen[1] = 28;
    activeBlue[1] = 255;
    musicVolume = 20;
    updateEEPROM();
  }
}

void updateEEPROM() {
  EEPROM.put(0, activeFadeSpeed);                           // uint16_t
  EEPROM.put(2, idleFadeSpeed);                             // uint16_t
  EEPROM.put(4, idleRed);                                   // uint8_t [2]
  EEPROM.put(6, idleGreen);                                 // uint8_t [2]
  EEPROM.put(8, idleBlue);                                  // uint8_t [2]
  EEPROM.put(10, transitionSpeed);                          // uint16_t
  EEPROM.put(14, activeRed);                                // uint8_t [2]
  EEPROM.put(16, activeGreen);                              // uint8_t [2]
  EEPROM.put(18, activeBlue);                               // uint8_t [2]
  EEPROM.put(20, musicVolume);                              // uint8_t
}
