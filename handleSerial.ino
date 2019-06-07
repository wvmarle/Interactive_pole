////////////////////////////////////////////////////////////////////
// Serial console menu.
enum menu {
  MENU_MAIN,
  MENU_SETCOLOURS,
  MENU_SETMUSIC,
};
uint8_t menuState;

enum mainMenuStates {
  MAINMENU_SHOW,
  MAINMENU_WAITING
};
uint8_t mainMenuState;

enum ColourMenuStates {
  COLOURMENU_SHOW_IDLE_FADE_SPEED,
  COLOURMENU_ENTER_IDLE_FADE_SPEED,
  COLOURMENU_CONFIRM_IDLE_FADE_SPEED,

  COLOURMENU_SHOW_IDLE_RGB_1,
  COLOURMENU_ENTER_IDLE_RED_1,
  COLOURMENU_ENTER_IDLE_GREEN_1,
  COLOURMENU_ENTER_IDLE_BLUE_1,
  COLOURMENU_CONFIRM_IDLE_RGB_1,

  COLOURMENU_SHOW_IDLE_RGB_2,
  COLOURMENU_ENTER_IDLE_RED_2,
  COLOURMENU_ENTER_IDLE_GREEN_2,
  COLOURMENU_ENTER_IDLE_BLUE_2,
  COLOURMENU_CONFIRM_IDLE_RGB_2,

  COLOURMENU_SHOW_ACTIVE_FADE_SPEED,
  COLOURMENU_ENTER_ACTIVE_FADE_SPEED,
  COLOURMENU_CONFIRM_ACTIVE_FADE_SPEED,

  COLOURMENU_SHOW_ACTIVE_RGB_1,
  COLOURMENU_ENTER_ACTIVE_RED_1,
  COLOURMENU_ENTER_ACTIVE_GREEN_1,
  COLOURMENU_ENTER_ACTIVE_BLUE_1,
  COLOURMENU_CONFIRM_ACTIVE_RGB_1,

  COLOURMENU_SHOW_ACTIVE_RGB_2,
  COLOURMENU_ENTER_ACTIVE_RED_2,
  COLOURMENU_ENTER_ACTIVE_GREEN_2,
  COLOURMENU_ENTER_ACTIVE_BLUE_2,
  COLOURMENU_CONFIRM_ACTIVE_RGB_2,

  COLOURMENU_SHOW_TRANSITION_SPEED,
  COLOURMENU_ENTER_TRANSITION_SPEED,
  COLOURMENU_CONFIRM_TRANSITION_SPEED,

  COLOURMENU_COMPLETE
};
ColourMenuStates colourMenuState;

enum MusicMenuStates {
  MUSICMENU_SHOW_VOLUME,
  MUSICMENU_ENTER_VOLUME,
  MUSICMENU_CONFIRM_VOLUME,

  MUSICMENU_COMPLETE
};
MusicMenuStates musicMenuState;
////////////////////////////////////////////////////////////////////
void handleSerialUI() {
  char c;
  if (Serial.available()) {
    c = Serial.read();
  }
  else {
    c = 0;
  }
  switch (menuState) {
    case MENU_MAIN:
      menuMain(c);
      break;

    case MENU_SETCOLOURS:
      menuSetColours(c);
      break;

    case MENU_SETMUSIC:
      menuSetMusic(c);
      break;
  }
}

void menuMain(char c) {
  switch (mainMenuState) {
    case (MAINMENU_SHOW):
      Serial.println(F("Welcome to the interactive music pole."));
      Serial.println();
      sprintf_P(buf, PSTR("Idle fading: %u milliseconds per cycle."), idleFadeSpeed);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Idle colour 1 (r, g, b) = (%u, %u, %u)"), idleRed[0], idleGreen[0], idleBlue[0]);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Idle colour 2 (r, g, b) = (%u, %u, %u)"), idleRed[1], idleGreen[1], idleBlue[1]);
      Serial.println(buf);
      Serial.println();
      sprintf_P(buf, PSTR("Transition fading: %u milliseconds to complete transition."), transitionSpeed);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Active fading: %u milliseconds per cycle."), activeFadeSpeed);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Active colour 1 (r, g, b) = (%u, %u, %u)"), activeRed[0], activeGreen[0], activeBlue[0]);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Active colour 2 (r, g, b) = (%u, %u, %u)"), activeRed[1], activeGreen[1], activeBlue[1]);
      Serial.println(buf);
      Serial.println();
      sprintf_P(buf, PSTR("Music volume = %u"), musicVolume);
      Serial.println(buf);
      Serial.println();
      Serial.println();
      Serial.println(F("1    Set up colours."));
      Serial.println(F("2    Set music volume."));
      Serial.println(F("3    Check brightness level."));
      Serial.println();
      Serial.println(F("Key in the desired option number."));
      mainMenuState = MAINMENU_WAITING;
      break;

    case (MAINMENU_WAITING):
      switch (c) {
        case '1':
          menuState = MENU_SETCOLOURS;
          mainMenuState = MAINMENU_SHOW;
          colourMenuState = COLOURMENU_SHOW_IDLE_FADE_SPEED;
          delay(10);                                        // Allow for stray characters to enter the Serial buffer.
          clearBuffer();
          break;

        case '2':
          menuState = MENU_SETMUSIC;
          mainMenuState = MAINMENU_SHOW;
          musicMenuState = MUSICMENU_SHOW_VOLUME;
          delay(10);                                        // Allow for stray characters to enter the Serial buffer.
          clearBuffer();
          break;

        case '3':
          Serial.print(F("Current brightness level: "));
          Serial.println(analogRead(BRIGHTNESS_SENSOR_PIN));
          delay(10);                                        // Allow for stray characters to enter the Serial buffer.
          clearBuffer();
          break;

        default:
          break;
      }
      break;
  }
}

const char PROGMEM confirm[] = "Press 'y' to confirm, any other key to re-enter.";

void menuSetColours(char c) {                               // Handle input of colour values.
  static char input[11];
  static uint8_t nCharacters = 0;
  static bool entryChanged;
  static bool lastWasCRNL;
  static uint16_t entry;
  static uint8_t rgbEntry[3];

  if (nCharacters < 10 && (c >= '0' && c <= '9')) {
    input[nCharacters] = c;
    Serial.print(c);
    nCharacters++;
  }
  if (c == 13 || c == 10 ||                                 // CR or NL received, or
      c == 'y' || c == 'Y') {                               // y or Y confirmation.
    delay(10);                                              // Wait for other characters to come in,
    clearBuffer();                                          // and ignore what came after.
  }

  switch (colourMenuState) {

    /*********************************************************************************************************
       Idle fading speed and steps.
    */
    case COLOURMENU_SHOW_IDLE_FADE_SPEED:
      Serial.println();
      sprintf_P(buf, PSTR("Enter the idle fade speed in milliseconds per cycle (%u): "), idleFadeSpeed);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_IDLE_FADE_SPEED;
      break;

    case COLOURMENU_ENTER_IDLE_FADE_SPEED:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          entry = atoi(input);
          nCharacters = 0;
          Serial.println();
          sprintf_P(buf, PSTR("New value for idle fade speed: %u ms per cycle."), entry);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_IDLE_FADE_SPEED;
        }
        else {                                                // No new value entered; continue to the next.
          colourMenuState = COLOURMENU_SHOW_IDLE_RGB_1;
        }
      }
      break;

    case COLOURMENU_CONFIRM_IDLE_FADE_SPEED:
      if (c == 'y' || c == 'Y') {
        idleFadeSpeed = entry;
        colourMenuState = COLOURMENU_SHOW_IDLE_RGB_1;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_IDLE_FADE_SPEED;
      }
      break;

    /*********************************************************************************************************
       Idle fading colour 1
    */
    case COLOURMENU_SHOW_IDLE_RGB_1:
      Serial.println();
      sprintf_P(buf, PSTR("Current idle base colour 1 (r, g, b) = (%u, %u, %u)."), idleRed[0], idleGreen[0], idleBlue[0]);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Enter new red 1 value (%u): "), idleRed[0]);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_IDLE_RED_1;
      rgbEntry[0] = idleRed[0];
      rgbEntry[1] = idleGreen[0];
      rgbEntry[2] = idleBlue[0];
      entryChanged = false;
      break;

    case COLOURMENU_ENTER_IDLE_RED_1:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[0] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new green 1 value (%u): "), idleGreen[0]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_IDLE_GREEN_1;
      }
      break;

    case COLOURMENU_ENTER_IDLE_GREEN_1:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[1] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new value 1 value (%u): "), idleBlue[0]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_IDLE_BLUE_1;
      }
      break;

    case COLOURMENU_ENTER_IDLE_BLUE_1:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[2] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        if (entryChanged) {
          Serial.println();
          sprintf_P(buf, PSTR("New idle base colour 1 (r, g, b) = (%u, %u, %u)."), rgbEntry[0], rgbEntry[1], rgbEntry[2]);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_IDLE_RGB_1;
        }
        else {
          colourMenuState = COLOURMENU_SHOW_IDLE_RGB_2;
        }
      }
      break;

    case COLOURMENU_CONFIRM_IDLE_RGB_1:
      if (c == 'y' || c == 'Y') {
        idleRed[0] = rgbEntry[0];
        idleGreen[0] = rgbEntry[1];
        idleBlue[0] = rgbEntry[2];
        colourMenuState = COLOURMENU_SHOW_IDLE_RGB_2;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_IDLE_RGB_1;
      }
      break;

    /*********************************************************************************************************
       Idle fading colour 2.
    */

    case COLOURMENU_SHOW_IDLE_RGB_2:
      Serial.println();
      sprintf_P(buf, PSTR("Current idle base colour 2 (r, g, b) = (%u, %u, %u)."), idleRed[1], idleGreen[1], idleBlue[1]);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Enter new red 2 value (%u): "), idleRed[1]);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_IDLE_RED_2;
      rgbEntry[0] = idleRed[1];
      rgbEntry[1] = idleGreen[1];
      rgbEntry[2] = idleBlue[1];
      entryChanged = false;
      break;

    case COLOURMENU_ENTER_IDLE_RED_2:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[0] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new green 2 value (%u): "), idleGreen[1]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_IDLE_GREEN_2;
      }
      break;

    case COLOURMENU_ENTER_IDLE_GREEN_2:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[1] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new value 2 value (%u): "), idleBlue[1]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_IDLE_BLUE_2;
      }
      break;

    case COLOURMENU_ENTER_IDLE_BLUE_2:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[2] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        if (entryChanged) {
          Serial.println();
          sprintf_P(buf, PSTR("New idle base colour 2 (r, g, b) = (%u, %u, %u)."), rgbEntry[0], rgbEntry[1], rgbEntry[2]);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_IDLE_RGB_2;
        }
        else {
          colourMenuState = COLOURMENU_SHOW_ACTIVE_FADE_SPEED;
        }
      }
      break;

    case COLOURMENU_CONFIRM_IDLE_RGB_2:
      if (c == 'y' || c == 'Y') {
        idleRed[1] = rgbEntry[0];
        idleGreen[1] = rgbEntry[1];
        idleBlue[1] = rgbEntry[2];
        colourMenuState = COLOURMENU_SHOW_ACTIVE_FADE_SPEED;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_IDLE_RGB_2;
      }
      break;

    /*********************************************************************************************************
       Active fading speed and steps.
    */
    case COLOURMENU_SHOW_ACTIVE_FADE_SPEED:
      Serial.println();
      sprintf_P(buf, PSTR("Enter the active fade speed in milliseconds per cycle (%u): "), activeFadeSpeed);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_ACTIVE_FADE_SPEED;
      break;

    case COLOURMENU_ENTER_ACTIVE_FADE_SPEED:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          entry = atoi(input);
          nCharacters = 0;
          Serial.println();
          sprintf_P(buf, PSTR("New value for active fade speed: %u ms per cycle."), entry);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_ACTIVE_FADE_SPEED;
        }
        else {                                                // No new value entered; continue to the next.
          colourMenuState = COLOURMENU_SHOW_ACTIVE_RGB_1;
        }
      }
      break;

    case COLOURMENU_CONFIRM_ACTIVE_FADE_SPEED:
      if (c == 'y' || c == 'Y') {
        activeFadeSpeed = entry;
        colourMenuState = COLOURMENU_SHOW_ACTIVE_RGB_1;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_ACTIVE_FADE_SPEED;
      }
      break;

    /*********************************************************************************************************
       Active colour 1.
    */
    case COLOURMENU_SHOW_ACTIVE_RGB_1:
      Serial.println();
      sprintf_P(buf, PSTR("Current active base colour 1 (r, g, b) = (%u, %u, %u)."), activeRed[0], activeGreen[0], activeBlue[0]);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Enter new red value (%u): "), activeRed[0]);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_ACTIVE_RED_1;
      rgbEntry[0] = activeRed[0];
      rgbEntry[1] = activeGreen[0];
      rgbEntry[2] = activeBlue[0];
      entryChanged = false;
      break;

    case COLOURMENU_ENTER_ACTIVE_RED_1:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[0] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new green value (%u): "), activeGreen[0]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_ACTIVE_GREEN_1;
      }
      break;

    case COLOURMENU_ENTER_ACTIVE_GREEN_1:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[1] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new value value (%u): "), activeBlue[0]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_ACTIVE_BLUE_1;
      }
      break;

    case COLOURMENU_ENTER_ACTIVE_BLUE_1:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[2] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        if (entryChanged) {
          Serial.println();
          sprintf_P(buf, PSTR("New active base colour 1 (r, g, b) = (%u, %u, %u)."), rgbEntry[0], rgbEntry[1], rgbEntry[2]);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_ACTIVE_RGB_1;
        }
        else {
          colourMenuState = COLOURMENU_SHOW_ACTIVE_RGB_2;
        }
      }
      break;

    case COLOURMENU_CONFIRM_ACTIVE_RGB_1:
      if (c == 'y' || c == 'Y') {
        activeRed[0] = rgbEntry[0];
        activeGreen[0] = rgbEntry[1];
        activeBlue[0] = rgbEntry[2];
        colourMenuState = COLOURMENU_SHOW_ACTIVE_RGB_2;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_ACTIVE_RGB_1;
      }
      break;

    /*********************************************************************************************************
       Active colour 2.
    */
    case COLOURMENU_SHOW_ACTIVE_RGB_2:
      Serial.println();
      sprintf_P(buf, PSTR("Current active base colour 2 (r, g, b) = (%u, %u, %u)."), activeRed[1], activeGreen[1], activeBlue[1]);
      Serial.println(buf);
      sprintf_P(buf, PSTR("Enter new red 2 value (%u): "), activeRed[1]);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_ACTIVE_RED_2;
      rgbEntry[0] = activeRed[1];
      rgbEntry[1] = activeGreen[1];
      rgbEntry[2] = activeBlue[1];
      entryChanged = false;
      break;

    case COLOURMENU_ENTER_ACTIVE_RED_2:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[0] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new green 2 value (%u): "), activeGreen[1]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_ACTIVE_GREEN_2;
      }
      break;

    case COLOURMENU_ENTER_ACTIVE_GREEN_2:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[1] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        sprintf_P(buf, PSTR("Enter new value 2 value (%u): "), activeBlue[1]);
        Serial.println(buf);
        colourMenuState = COLOURMENU_ENTER_ACTIVE_BLUE_2;
      }
      break;

    case COLOURMENU_ENTER_ACTIVE_BLUE_2:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          rgbEntry[2] = atoi(input);
          nCharacters = 0;
          entryChanged = true;
          Serial.println();
        }
        if (entryChanged) {
          Serial.println();
          sprintf_P(buf, PSTR("New active base colour 2 (r, g, b) = (%u, %u, %u)."), rgbEntry[0], rgbEntry[1], rgbEntry[2]);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_ACTIVE_RGB_2;
        }
        else {
          colourMenuState = COLOURMENU_SHOW_TRANSITION_SPEED;
        }
      }
      break;

    case COLOURMENU_CONFIRM_ACTIVE_RGB_2:
      if (c == 'y' || c == 'Y') {
        activeRed[1] = rgbEntry[0];
        activeGreen[1] = rgbEntry[1];
        activeBlue[1] = rgbEntry[2];
        colourMenuState = COLOURMENU_SHOW_TRANSITION_SPEED;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_ACTIVE_RGB_2;
      }
      break;


    /*********************************************************************************************************
       Transition speed and steps.
    */
    case COLOURMENU_SHOW_TRANSITION_SPEED:
      Serial.println();
      Serial.println(F("Upon detection of movement in the proximity of the pole, transition to the active colour."));
      sprintf_P(buf, PSTR("Enter the time it takes for this transition in milliseconds (%u): "), transitionSpeed);
      Serial.println(buf);
      colourMenuState = COLOURMENU_ENTER_TRANSITION_SPEED;
      break;

    case COLOURMENU_ENTER_TRANSITION_SPEED:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          entry = atoi(input);
          nCharacters = 0;
          Serial.println();
          sprintf_P(buf, PSTR("New value for transition time: %u ms."), entry);
          Serial.println(buf);
          strcpy_P(buf, confirm);
          Serial.println(buf);
          colourMenuState = COLOURMENU_CONFIRM_TRANSITION_SPEED;
        }
        else {                                                // No new value entered; continue to the next.
          colourMenuState = COLOURMENU_COMPLETE;
        }
      }
      break;

    case COLOURMENU_CONFIRM_TRANSITION_SPEED:
      if (c == 'y' || c == 'Y') {
        transitionSpeed = entry;
        colourMenuState = COLOURMENU_COMPLETE;
      }
      else if (c != 0) {
        colourMenuState = COLOURMENU_SHOW_TRANSITION_SPEED;
      }
      break;

    case COLOURMENU_COMPLETE:
      updateEEPROM();
      menuState = MENU_MAIN;
      Serial.println();
      Serial.println();
      Serial.println();
      break;
  }
}

void menuSetMusic(char c) {
  /*********************************************************************************************************
     Set up the music player.
  */
  static char input[11];
  static uint8_t nCharacters = 0;
  static bool entryChanged;
  static bool lastWasCRNL;
  static uint16_t entry;
  static uint8_t rgbEntry[3];

  if (nCharacters < 10 && (c >= '0' && c <= '9')) {
    input[nCharacters] = c;
    Serial.print(c);
    nCharacters++;
  }
  if (c == 13 || c == 10 ||                                 // CR or NL received, or
      c == 'y' || c == 'Y') {                               // y or Y confirmation.
    delay(10);                                              // Wait for other characters to come in,
    clearBuffer();                                          // and ignore what came after.
  }

  switch (musicMenuState) {
    case MUSICMENU_SHOW_VOLUME:
      Serial.println();
      sprintf_P(buf, PSTR("Set the music volume (0-30) (%u): "), musicVolume);
      Serial.println(buf);
      musicMenuState = MUSICMENU_ENTER_VOLUME;
      break;

    case MUSICMENU_ENTER_VOLUME:
      if (c == 13 || c == 10) {                                // CR or NL received. Handle input based on what it is.
        if (nCharacters > 0) {
          input[nCharacters] = 0;
          entry = atoi(input);
          if (entry < 0 || entry > 30) {
            Serial.println(F("Invalid input. Please try again."));
            nCharacters = 0;
          }
          else {
            nCharacters = 0;
            Serial.println();
            sprintf_P(buf, PSTR("New volume: %u."), entry);
            Serial.println(buf);
            strcpy_P(buf, confirm);
            Serial.println(buf);
            musicMenuState = MUSICMENU_CONFIRM_VOLUME;
          }
        }
        else {                                                // No new value entered; continue to the next.
          musicMenuState = MUSICMENU_COMPLETE;
        }
      }
      break;

    case MUSICMENU_CONFIRM_VOLUME:
      if (c == 'y' || c == 'Y') {
        musicVolume = entry;
        musicMenuState = MUSICMENU_COMPLETE;
      }
      else if (c != 0) {
        musicMenuState = MUSICMENU_SHOW_VOLUME;
      }
      break;

    case MUSICMENU_COMPLETE:
      updateEEPROM();
      myDFPlayer.volume(musicVolume);
      menuState = MENU_MAIN;
      Serial.println();
      Serial.println();
      Serial.println();
      break;
  }
}

void clearBuffer() {
  while (Serial.available()) {                // Clear the buffer of any stray input.
    Serial.read();
  }
}
