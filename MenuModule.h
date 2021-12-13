// main menu
int menuDirection = AXIS_IDLE;
String menuTitles[] = {"Welcome", "Cave Run", "Settings", "Hall of Fame","About"};

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

// i chose to hold the brighness settings in a short array as they're all numbers (except the name) in order to write more generic control functions
// consult the settings define section for the corresponding indices 
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
String aboutText[] = {"My Github:", "git.io/JMQEj"};

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
  EEPROM.put(100, systemSettings);
}

void saveScore() {
  bool shouldUpdateEEPROM = false;
  for(int i = 0; i < 3; i++) {
    if (currentPlayer.score > scores[i].score) {
      strcpy(scores[i].name, systemSettings.name);
      scores[i].score = currentPlayer.score;
      shouldUpdateEEPROM = true;
      break;
    }
  }

  if (shouldUpdateEEPROM) {
    EEPROM.put(0, scores);
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
  if (userInput == AXIS_NEGATIVE && selectedItem < (itemCount-1)) {
    shouldRedrawMenu = true;
    selectedItem += 1;
  }

  if (userInput == AXIS_POSITIVE && selectedItem > 0) {
    shouldRedrawMenu = true;
    selectedItem -= 1;
  }
}

/**
 * Draws the menu according the current system state.
 */

void drawMenu(String title, bool showCaret, String option, short optionsCount, bool isEditingSetting = false) {
  lcd.clear();
  short titleCursorPos = (15 - title.length()) / 2 + 1;
  
  lcd.setCursor(titleCursorPos,0);
  lcd.print(title);
  lcd.setCursor(0,1);

  if (showCaret) {
    lcd.print(">");
  }
  lcd.print(option);

  if (isEditingSetting) {

    lcd.setCursor(15,1);
    lcd.write(byte(4));
    shouldRedrawMenu = false;
    return;  
  }

  short scrollbarCharacter;
  if (selectedItem == 0) {
      scrollbarCharacter = 0;
  } else if (selectedItem == optionsCount -1) {
      scrollbarCharacter = 1;
  } else {
      scrollbarCharacter = 2;
  }
  
  lcd.setCursor(15,1);
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
    drawMenu(menuTitles[1] , true, menuItems[selectedItem], menuItemsCount);
  }
}


void changeSelection(short input) {
  Serial.println("hi");
  if (input == AXIS_POSITIVE && index < 5) {
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
  lcd.setCursor(0,0);
  lcd.print(systemSettings.name);
  lcd.setCursor(11,0);
  lcd.print("Save");
  if (index == 5) {
    lcd.setCursor(12, 1);
  } else {
    lcd.setCursor(index, 1);
  }
  lcd.print("^");
  
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
    lcd.setCursor(0,0);
    lcd.print(aboutText[0]);
    lcd.setCursor(0,1);
    lcd.print(aboutText[1]);
    lcd.setCursor(15,1);
    lcd.write(byte(3));
    shouldRedrawMenu = false;
  }
}

String getScoreText(short selectedItem) {
  String str = String(selectedItem+1);
  str.concat(" ");
  str.concat(String(scores[selectedItem].name));
  str.concat(" ");
  str.concat(String(scores[selectedItem].score));
//  Serial.println(str);
  return str;
}

void highscoresLoop() {
  short userInput = getMenuUserInput(joystickY);

  if (userInput) {
      updateLcdMenu(userInput, highscoreItemsCount);
  }

  if (shouldRedrawMenu) {
     drawMenu(menuTitles[3], false, getScoreText(selectedItem), sizeof(scores)/sizeof(PlayerScore));
  }

}

String getSettingText(short item) {

  String message = "";

  message.concat(settingsItems[item]);

//  Serial.println(selectedItem);
//  Serial.println(selectedItem);


  switch (item) {
    case 0:
      message.concat(systemSettings.name);
      break;
    case 1:
      message.concat(systemSettings.difficulty);
      break;
    case 5:
      break;
    default:
      message.concat(systemSettings.brightnessArray[item-2]);
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


void updateSetting(short userInput) {
  short lowerBound, upperBound, currentValue;

  if (selectedItem == SETTING_DIFFICULTY) {
    lowerBound = SETTINGS_MIN_DIFFICULTY;
    upperBound = SETTINGS_MAX_DIFFICULTY;
    currentValue = systemSettings.difficulty;
  } else {
    lowerBound = SETTINGS_MIN_ADJUSTMENT_VALUE;
    upperBound = SETTINGS_MAX_ADJUSTMENT_VALUE;
    currentValue = systemSettings.brightnessArray[selectedItem-2];
  }


  if (userInput == AXIS_NEGATIVE && currentValue < upperBound) {
    shouldRedrawMenu = true;
    currentValue += 1;
  }

  if (userInput == AXIS_POSITIVE && currentValue > 0) {
    shouldRedrawMenu = true;
    currentValue -= 1;
  }

  if (selectedItem == SETTING_DIFFICULTY) {
    systemSettings.difficulty = currentValue;
  } else {
    systemSettings.brightnessArray[selectedItem-2] = currentValue;
  }

  applySettings();

  
}


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
     drawMenu(menuTitles[2], true, getSettingText(selectedItem), settingsItemsCount, isEditingSetting);
  }
}
