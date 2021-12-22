#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>

#include "Constants.h"
#include "SystemModule.h"
#include "MenuModule.h"
#include "GameModule.h"
#include "StateChanger.h"
#include "SetupModule.h"
#include "ThemeSongModule.h"

void setup() {
  
  joystickSetup();
  stateChangerSetup();
  memorySetup();
  lcdSetup();
  matrixSetup();
  buzzerSetup();
  
  Serial.begin(BAUD);
  randomSeed(analogRead(RANDOM_PIN));
}

void loop() {
  
  computeJoystickValues();
  themeSongLoop();

  switch (systemState) {

    case SYSTEM_STATE_SPLASH:
      doSplash();
      break;

    case SYSTEM_STATE_MENU:
      menuLoop();
      break;
      
    case SYSTEM_STATE_MENU_ABOUT:
      aboutLoop();
      break;

    case SYSTEM_STATE_MENU_HIGHSCORES:
      highscoresLoop();
      break;

    case SYSTEM_STATE_MENU_SETTINGS:
      settingsLoop();
      break;

    case SYSTEM_STATE_NAME_EDIT:
      nameEditLoop();
      break;
      
    case SYSTEM_STATE_GAME:
      gameLoop();
      break;

    case SYSTEM_STATE_GAME_END:
      gameEndLoop();
      break;
      
  }
  delay(2);
}
