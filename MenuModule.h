/**
 * MenuModule.h - Everything concerning the menu and its logic 
 */

int menuDirection = AXIS_IDLE;

String menuItems[] = {"Start Game", "Settings", "Highscores", "About"};
int menuItemsCount = 4;
int highscoreItemsCount = 3;
volatile int selectedItem = 0;
volatile bool shouldRedrawMenu = false;

// settings menu
String settingsItems[] = {"Name:", "Difficulty:", "Contrast:", "Brightness:", "Matr. Light:", "Back to Menu"};
const int settingsItemsCount = 6;
volatile bool isEditingSetting = false;
volatile bool nameEditSetup = true;

byte index = 0;
volatile int nameEditState = SYSTEM_STATE_NAME_EDIT_LOCKED;
bool shouldRedrawNameEdit = true;

// i chose to hold the brightness settings in a short array as they're all numbers (except the name) in order to write more generic control functions
// consult the settings constants section for the corresponding indices
struct Settings {
  char name[6];
  short difficulty;
  short brightnessArray[3];
} systemSettings;

// high score menu

struct PlayerScore {
  short score;
  char name[6];
} currentPlayer;

PlayerScore scores[3];

// about menu
String aboutText[] = {"Nicoleta Ciausu", "git.io/JMQEj"};

/**
 * Interprets user joystick input in the way needed for the menu.
 * Only read off one axis, and disable scrolling-through-items by holding the joystick.
 */
short getMenuUserInput(int axis) {
  short reading = axis;
  if (reading == AXIS_IDLE) {
    didReadJoystick = false;
    return reading;
  }

  if (!didReadJoystick) {
    didReadJoystick = true;
    return reading;
  }

  // do nothing - we don't want to cycle through the menu on joystick hold
  return AXIS_IDLE;
}

void saveSettings() {
  EEPROM.put(EEPROM_SETTINGS_MEMORY_LOCATION, systemSettings);
}

/**
 * Figures out if the currently acheived score is a high score and if so saves it to the eeprom.
 */
void saveScore() {
  bool shouldUpdateEEPROM = false;
  for (int i = 0; i < 3; i++) {
    if (currentPlayer.score > scores[i].score) {
      strcpy(scores[i].name, systemSettings.name);
      scores[i].score = currentPlayer.score;
      shouldUpdateEEPROM = true;
      break;
    }
  }

  if (shouldUpdateEEPROM) {
    EEPROM.put(EEPROM_HIGHSCORES_MEMORY_LOCATION, scores);
  }
}

// change system state depending on selected setting to edit
void doSettingsAction() {
  if (isEditingSetting) {
    isEditingSetting = false;
    saveSettings();
    return;
  }

  switch (selectedItem) {
    case settingsItemsCount - 1:
      selectedItem = 0;
      setMatrixImage(menuMatrixSymbol);
      systemState = SYSTEM_STATE_MENU;
      return;

    case 0:

      systemState = SYSTEM_STATE_NAME_EDIT;
      if (nameEditState == SYSTEM_STATE_NAME_EDIT_UNLOCKED) {
        nameEditState = SYSTEM_STATE_NAME_EDIT_LOCKED;
      } else {
        nameEditState = SYSTEM_STATE_NAME_EDIT_UNLOCKED;
      }
      shouldRedrawNameEdit = true;
      return;

    default:
      isEditingSetting = true;
      return;
  }
}
/**
 * Updates user menu selection, making sure not to go out of the item count bounds.
 */
void updateLcdMenu(short userInput, short itemCount) {
  if (userInput == AXIS_NEGATIVE && selectedItem < (itemCount - 1)) {
    shouldRedrawMenu = true;
    selectedItem += 1;
  }

  if (userInput == AXIS_POSITIVE && selectedItem > 0) {
    shouldRedrawMenu = true;
    selectedItem -= 1;
  }
}

/**
 * Does a bit of math which figures out how to center text on a lcd.
 */
byte getCenteredTextPosition(String text) {
  return (LCD_CHARACTER_LENGTH - text.length()) / 2 + 1;
}

/**
 * Draws the menu according the current system state.
 */
void drawMenu(String title, bool showCaret, String option, short optionsCount, bool isEditingSetting = false) {
  lcd.clear();
  short titleCursorPos = getCenteredTextPosition(title);

  lcd.setCursor(titleCursorPos, 0);
  lcd.print(title);
  lcd.setCursor(0, 1);

  if (showCaret) {
    lcd.print(">");
  }
  lcd.print(option);

  if (isEditingSetting) {
    lcd.setCursor(SCROLLBAR_TEXT_POS);
    lcd.write(byte(ENTER_SYMBOL));
    shouldRedrawMenu = false;
    return;
  }

  short scrollbarCharacter;
  if (selectedItem == 0) {
    scrollbarCharacter = DOWN_SYMBOL;
  } else if (selectedItem == optionsCount - 1) {
    scrollbarCharacter = UP_SYMBOL;
  } else {
    scrollbarCharacter = BOTH_SYMBOL;
  }

  lcd.setCursor(SCROLLBAR_TEXT_POS);
  lcd.write(byte(scrollbarCharacter));
  shouldRedrawMenu = false;
}

/**
  * Loop for the SYSTEM_STATE_MENU state.
  * Redraw on LCD only on user action.
  */
void menuLoop() {
  short userInput = getMenuUserInput(joystickY);

  if (userInput) {
    updateLcdMenu(userInput, menuItemsCount);
  }

  if (shouldRedrawMenu) {
    drawMenu(MENU_TITLE_GAME, true, menuItems[selectedItem], menuItemsCount);
  }
}

/**
 * Changes selected letter in SYSTEM_STATE_EDIT name
 */
void changeSelection(short input) {
  // keep in bounds, no wrapping back to the beginning of the name
  if (input == AXIS_POSITIVE && index < NAME_LENGTH) {
    index += 1;
  }

  if (input == AXIS_NEGATIVE && index > 0) {
    index -= 1;
  }
  shouldRedrawNameEdit = true;
}

void scrollThroughLetter(short input) {
  if (input == AXIS_POSITIVE && systemSettings.name[index] < 'Z') {
    systemSettings.name[index] += 1;
  }

  if (input == AXIS_NEGATIVE && systemSettings.name[index] > 'A') {
    systemSettings.name[index] -= 1;
  }

  shouldRedrawNameEdit = true;
}

void drawNameEdit() {
  lcd.clear();
  lcd.setCursor(NAME_TEXT_POS);
  lcd.print(systemSettings.name);
  lcd.setCursor(SAVE_TEXT_POS);
  lcd.print(SAVE_TEXT);

  // show selection: either an editable character or the Save button
  if (index == NAME_LENGTH) {
    lcd.setCursor(SAVE_CURSOR_POS);
  } else {
    lcd.setCursor(index, 1);
  }

  if (nameEditState == SYSTEM_STATE_NAME_EDIT_UNLOCKED) {
    lcd.print(SELECTION_CARET);
  } else {
    lcd.write(byte(TRIANGLE_SYMBOL));
  }

  shouldRedrawNameEdit = false;
}

void nameEditLoop() {
  short userInput = getMenuUserInput(joystickX);

  if (userInput) {
    if (nameEditState == SYSTEM_STATE_NAME_EDIT_UNLOCKED) {
      changeSelection(userInput);
    } else {
      scrollThroughLetter(userInput);
    }
  }

  if (shouldRedrawNameEdit) {
    drawNameEdit();
  }
}

void aboutLoop() {
  if (shouldRedrawMenu) {
    lcd.clear();
    lcd.setCursor(ABOUT_NAME_TEXT_POS);
    lcd.print(aboutText[0]);
    lcd.setCursor(ABOUT_GITHUB_TEXT_POS);
    lcd.print(aboutText[1]);
    lcd.setCursor(SCROLLBAR_TEXT_POS);
    lcd.write(byte(CLICK_SYMBOL));
    shouldRedrawMenu = false;
  }
}

String getScoreText(short selectedItem) {
  String str = String(selectedItem + 1);
  str.concat(" ");
  str.concat(String(scores[selectedItem].name));
  str.concat(" ");
  str.concat(String(scores[selectedItem].score));

  return str;
}

void highscoresLoop() {
  short userInput = getMenuUserInput(joystickY);

  if (userInput) {
    updateLcdMenu(userInput, highscoreItemsCount);
  }

  if (shouldRedrawMenu) {
    drawMenu(MENU_TITLE_HIGHSCORES, false, getScoreText(selectedItem), sizeof(scores) / sizeof(PlayerScore));
  }
}

/**
 * Composes the setting text with its value and returns it for displaying.
 */
String getSettingText(short item) {
  String message = "";

  message.concat(settingsItems[item]);

  switch (item) {
    case SETTING_NAME:
      message.concat(systemSettings.name);
      break;
    case SETTING_DIFFICULTY:
      message.concat(systemSettings.difficulty);
      break;
    case SETTING_BACK_TO_MENU:
      break;
    default:
      // first 2 settings aren't related to brightness array; to avoid padding in the vector (2 0 positions) i subtract from the selection index
      // the value 2 - so the first brightness setting, which is setting number 3, has a corresponding value of 1 in the brightness array.
      message.concat(systemSettings.brightnessArray[item - SETTING_NUMBERED_ITEM_PADDING]);
      break;
  }

  return message;
}

/**
 * Applies settings values to the actual hardware.
 */
void applySettings() {
  analogWrite(LCD_V0, systemSettings.brightnessArray[0] * SETTINGS_ADJUSTMENT_INCREMENT);
  analogWrite(LCD_BACKLIGHT, systemSettings.brightnessArray[1] * SETTINGS_ADJUSTMENT_INCREMENT);
  matrix.setIntensity(0, systemSettings.brightnessArray[2]);
}

/**
 * For the selected setting, if user has interacted with it, update its value, and make sure not to exit bounds.
 */
void updateSetting(short userInput) {
  short lowerBound, upperBound, currentValue;

  // all numbered settings are bound from 1-9 except for difficulty which is 1-3. handling that here
  if (selectedItem == SETTING_DIFFICULTY) {
    lowerBound = SETTINGS_MIN_DIFFICULTY;
    upperBound = SETTINGS_MAX_DIFFICULTY;
    currentValue = systemSettings.difficulty;
  } else {
    lowerBound = SETTINGS_MIN_ADJUSTMENT_VALUE;
    upperBound = SETTINGS_MAX_ADJUSTMENT_VALUE;
    currentValue = systemSettings.brightnessArray[selectedItem - 2];
  }

  // don't escape bounds
  if (userInput == AXIS_NEGATIVE && currentValue < upperBound) {
    shouldRedrawMenu = true;
    currentValue += 1;
  }

  if (userInput == AXIS_POSITIVE && currentValue > lowerBound) {
    shouldRedrawMenu = true;
    currentValue -= 1;
  }

  if (selectedItem == SETTING_DIFFICULTY) {
    systemSettings.difficulty = currentValue;
  } else {
    systemSettings.brightnessArray[selectedItem - 2] = currentValue;
  }

  // make the settings take effect immediately
  applySettings();
}

/**
 * Loop for SYSTEM_STATE_MENU_SETTINGS state.
 */
void settingsLoop() {
  short userInput = getMenuUserInput(joystickY);

  if (userInput) {
    if (isEditingSetting) {
      // if adjusting setting, update its value
      updateSetting(userInput);
    } else {
      // if scrolling through settings, change it
      updateLcdMenu(userInput, settingsItemsCount);
    }
  }

  if (shouldRedrawMenu) {
    drawMenu(MENU_TITLE_SETTINGS, true, getSettingText(selectedItem), settingsItemsCount, isEditingSetting);
  }
}

/**
 * Game splash function, draw the welcome text and wait.
 */
void doSplash() {
  setMatrixImage(happyMatrixSymbol);
  lcd.clear();
  lcd.setCursor(SPLASH_TEXT_POS);
  lcd.print(SPLASH_TEXT);

  delay(SPLASH_TIME_MS);
  systemState = SYSTEM_STATE_MENU;
  shouldRedrawMenu = true;
  setMatrixImage(menuMatrixSymbol);
}
