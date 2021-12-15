void stateChangerSetup() {
    attachInterrupt(digitalPinToInterrupt(JOYSTICK_BTN), joystickButtonClick, FALLING);
}

void joystickSetup() {
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
}

void memorySetup() {
//  strcpy(scores[0].name, "AAAAA");
//  strcpy(scores[1].name, "AAAAA");
//  strcpy(scores[2].name, "AAAAA");
  EEPROM.get(0, scores);
  EEPROM.get(100, systemSettings);
}

void buzzerSetup() {
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
}

void lcdSetup() {
   
  applySettings();
  lcd.createChar(0, downArrowSymbol);
  lcd.createChar(1, upArrowSymbol);
  lcd.createChar(2, bothArrowSymbol);
  lcd.createChar(3, clickSymbol);
  lcd.createChar(4, enterSymbol);
  lcd.createChar(5, heartSymbol);

  lcd.begin(16, 2);

  // end lcd setup
  drawMenu(menuTitles[1], true, menuItems[selectedItem], menuItemsCount, false);
  pinMode(WARNING_LED_PIN, OUTPUT);
  analogWrite(WARNING_LED_PIN, 1000);

}

void matrixSetup() {
  
  matrix.shutdown(0, false);
  matrix.setIntensity(0, 2);
  matrix.clearDisplay(0);
}
