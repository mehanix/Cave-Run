#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>

#include "Constants.h"
#include "SystemModule.h"
#include "MenuModule.h"
#include "GameModule.h"
#include "StateChanger.h"
#include "SetupModule.h"

void setup() {

//  strcpy(systemSettings.name, "AAAAA");
//  systemSettings.difficulty = 1;
//  systemSettings.brightnessArray[0] = 6;
//  systemSettings.brightnessArray[1] = 9;
//  systemSettings.brightnessArray[2] = 6;

  joystickSetup();
  stateChangerSetup();
  memorySetup();
  lcdSetup();
  matrixSetup();
  Serial.begin(9600);
  randomSeed(analogRead(A4));
}

void loop() {
  
  computeJoystickValues();

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
      
  }
}
