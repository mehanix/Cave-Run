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
  Serial.begin(9600);
  randomSeed(analogRead(RANDOM_PIN));
}

void loop() {
  
  computeJoystickValues();
  themeSongLoop();

  switch (systemState) {

    case SYSTEM_STATE_SPLASH:
      setMatrixImage(happyMatrixSymbol);
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Welcome!");
      delay(2000);
      systemState = SYSTEM_STATE_MENU;
      shouldRedrawMenu = true;
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
