/**
 * SetupModule.h - Areas of concern: Holds setup functions for the hardware
 */
void stateChangerSetup() {
    attachInterrupt(digitalPinToInterrupt(JOYSTICK_BTN), joystickButtonClick, FALLING);
}

void joystickSetup() {
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
}

void memorySetup() {
  
  EEPROM.get(EEPROM_HIGHSCORES_MEMORY_LOCATION, scores);
  EEPROM.get(EEPROM_SETTINGS_MEMORY_LOCATION, systemSettings);
}

void buzzerSetup() {
  
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
}

void lcdSetup() {
   
  applySettings();
  lcd.createChar(DOWN_SYMBOL, downArrowSymbol);
  lcd.createChar(UP_SYMBOL, upArrowSymbol);
  lcd.createChar(BOTH_SYMBOL, bothArrowSymbol);
  lcd.createChar(CLICK_SYMBOL, clickSymbol);
  lcd.createChar(ENTER_SYMBOL, enterSymbol);
  lcd.createChar(HEART_SYMBOL, heartSymbol);

  lcd.begin(16, 2);

  // end lcd setup
  drawMenu(MENU_TITLE_GAME, true, menuItems[selectedItem], menuItemsCount, false);
  pinMode(WARNING_LED_PIN, OUTPUT);

}

void matrixSetup() {
  
  matrix.shutdown(0, false);
  matrix.clearDisplay(0);
}
