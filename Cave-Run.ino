ea#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>

/******** HARDWARE *********/

/***** LCD *****/

#define LCD_RS 8 
#define LCD_EN 6 
#define LCD_D4 5 
#define LCD_D5 4 
#define LCD_D6 9
#define LCD_D7 13
#define LCD_V0 3
          
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/***** MATRIX *****/

#define MATRIX_DIN_PIN 12
#define MATRIX_CLOCK_PIN 11
#define MATRIX_LOAD_PIN 10

#define MATRIX_SIZE 8
#define MATRIX_BRIGHTNESS 2

LedControl matrix = LedControl(MATRIX_DIN_PIN, MATRIX_CLOCK_PIN, MATRIX_LOAD_PIN, 1);

/***** JOYSTICK *****/

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BTN 2

#define JOYSTICK_MIN_TRESHOLD 200
#define JOYSTICK_MAX_TRESHOLD 800

#define AXIS_IDLE 0
#define AXIS_POSITIVE 1
#define AXIS_NEGATIVE -1

/******** END HARDWARE *********/

/******** GLOBAL STATES *********/

#define SYSTEM_STATE_SPLASH 0
#define SYSTEM_STATE_MENU 1
#define SYSTEM_STATE_MENU_SETTINGS 2
#define SYSTEM_STATE_MENU_HIGHSCORES 3
#define SYSTEM_STATE_MENU_ABOUT 4
#define SYSTEM_STATE_SETTINGS_EDITING 5
#define SYSTEM_STATE_GAME 10
#define SYSTEM_STATE_GAME_END 11

/******** END GLOBAL STATES *********/

/******** SETTINGS BOUNDS ***********/
// for contrast, brightness, etc
#define SETTINGS_MIN_ADJUSTMENT_VALUE 1
#define SETTINGS_MAX_ADJUSTMENT_VALUE 9

#define SETTINGS_ADJUSTMENT_INCREMENT 25

#define SETTINGS_MIN_DIFFICULTY 1
#define SETTINGS_MAX_DIFFICULTY 3

#define SETTING_DIFFICULTY 0
#define SETTING_LCD_CONTRAST 1
#define SETTING LCD_BRIGHTNESS 2
#define SETTING_MATRIX_BRIGHTNESS 3

/******** END SETTINGS BOUNDS *******/

/*********** VARIABLES **************/

/***** RAW INPUT / HARDWARE-RELATED *****/
volatile int systemState = SYSTEM_STATE_MENU;

/***** JOYSTICK *****/
int joystickX = 0;
int joystickY = 0;
bool joystickBtn = LOW;
bool didReadJoystick = false;

int rawJoystickX = 0;
int rawJoystickY = 0;
bool rawJoystickBtn = HIGH;

volatile unsigned long long lastDebounceTime = 0;
unsigned long long debounceDelay = 300;

/***** END JOYSTICK *****/

/***** LCD *****/

byte downArrowSymbol[8] = {
  B00000,
  B00000,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000,
  B00000
};

byte upArrowSymbol[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00000,
  B00000
};

byte bothArrowSymbol[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};

byte clickSymbol[8] = {
  B00000,
  B00000,
  B00000,
  B01110,
  B01110,
  B00000,
  B00000,
  B00000,
};

byte enterSymbol[8] = {
  B00000,
  B00000,
  B00001,
  B00101,
  B01001,
  B11111,
  B01000,
  B00100,
};


/***** END LCD *****/

/***** MENU *****/

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

// i chose to hold the brighness settings in a short array as they're all numbers (except the name) in order to write more generic control functions
// consult the settings define section for the corresponding indices 
struct Settings {
  char name[6];
  short difficulty;
  short brightnessArray[3];
} systemSettings;

// high score menu

struct Player {
  short score;
  char name[6];
};

Player scores[3];

// about menu
String aboutText[] = {"My Github:", "git.io/JMQEj"};



/***** END MENU *****/

void joystickButtonClick() {

  Serial.println("Nu");
  volatile int currentTime = millis();
  if (currentTime - lastDebounceTime < debounceDelay) { 
    return;
  }
  Serial.println("Clk");

  lastDebounceTime = currentTime;
  shouldRedrawMenu = true;

  switch (systemState){
    case SYSTEM_STATE_MENU:
      switch (selectedItem) {
        case 0:
          systemState = SYSTEM_STATE_GAME;
          break;
        case 1:
          systemState = SYSTEM_STATE_MENU_SETTINGS;
          isEditingSetting = false;
          break;
        case 2:
          systemState = SYSTEM_STATE_MENU_HIGHSCORES;
          break;
        case 3:
          systemState = SYSTEM_STATE_MENU_ABOUT;
          break;
      }
      selectedItem = 0;
      return;

    case SYSTEM_STATE_MENU_ABOUT:
      systemState = SYSTEM_STATE_MENU;
      return;

    case SYSTEM_STATE_MENU_HIGHSCORES:
      systemState = SYSTEM_STATE_MENU;
      return;

    case SYSTEM_STATE_MENU_SETTINGS:
      doSettingsAction();
      return;
    
  }
}

void saveSettings() {
  EEPROM.put(100, systemSettings);
}

// change system state depending on selected setting to edit
void doSettingsAction() {

  if (isEditingSetting) {
    isEditingSetting = false;
    Serial.println("pa");
    saveSettings();
    return;
  }

  switch (selectedItem) {
    case settingsItemsCount - 1:
      selectedItem = 0;
      systemState = SYSTEM_STATE_MENU;
      return;

    case 0:
      // name edit fun
      break;
      return;
      
    default:
      isEditingSetting = true;
      Serial.println("why2");
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

//void drawMenuScrollbarCharacter(

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
void menu() {
  
    short userInput = getMenuUserInput(joystickY);

    if (userInput) {
      updateLcdMenu(userInput, menuItemsCount);
    }

    if (shouldRedrawMenu) {
      drawMenu(menuTitles[1] , true, menuItems[selectedItem], menuItemsCount);
    }
}

void about() {

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
  Serial.println(str);
  return str;
}

void highscores() {
  short userInput = getMenuUserInput(joystickY);

  if (userInput) {
      updateLcdMenu(userInput, highscoreItemsCount);
  }

  if (shouldRedrawMenu) {
     drawMenu(menuTitles[3], false, getScoreText(selectedItem), sizeof(scores)/sizeof(Player));
  }

}

/**
 * Interprets user joystick input in the way needed for the menu.
 * Only read off Y axis, and disable scrolling-through-items by holding the joystick.
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


/**
 * Handles hardware joystick input and turns it into easier-to-use values.
 * 
 * Updates global variables joystickX/Y with axis values representing:
 *   * 1/-1 for up/down on joystickY
 *   * 1/-1 for right/left on joystickX
 *   * 0 for idle
 *   
 * Using this system we can also express diagonal movement in an easy to use format.
 */
void computeJoystickValues() {
  
  rawJoystickX = analogRead(JOYSTICK_X);
  rawJoystickY = analogRead(JOYSTICK_Y);
  rawJoystickBtn = digitalRead(JOYSTICK_BTN);
  
    if (rawJoystickY >= JOYSTICK_MIN_TRESHOLD && rawJoystickY <= JOYSTICK_MAX_TRESHOLD) {
      joystickY = AXIS_IDLE;
    } else if (rawJoystickY < JOYSTICK_MIN_TRESHOLD) {
      joystickY = AXIS_POSITIVE;
    } else {
      joystickY = AXIS_NEGATIVE;
    }

    if (rawJoystickX >= JOYSTICK_MIN_TRESHOLD && rawJoystickX <= JOYSTICK_MAX_TRESHOLD) {
      joystickX = AXIS_IDLE;
    } else if (rawJoystickX < JOYSTICK_MIN_TRESHOLD) {
      joystickX = AXIS_POSITIVE;
    } else {
      joystickX = AXIS_NEGATIVE;
    } 
}

String getSettingText(short item) {

  String message = "";

  message.concat(settingsItems[item]);

  Serial.println(selectedItem);
  Serial.println(selectedItem);


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

/**
 * Applies settings values to the actual hardware.
 */
void applySettings() {
  
    analogWrite(LCD_V0, systemSettings.brightnessArray[0] * SETTINGS_ADJUSTMENT_INCREMENT);
    //todo others
}

void settings() {
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

void setup() {
  // pin setup

//  strcpy(systemSettings.name, "AAAAA");
//  systemSettings.difficulty = 1;
//  systemSettings.brightnessArray[0] = 6;
//  systemSettings.brightnessArray[1] = 9;
//  systemSettings.brightnessArray[2] = 6;


  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
  
  EEPROM.get(0, scores);
  EEPROM.get(100, systemSettings);
  attachInterrupt(digitalPinToInterrupt(JOYSTICK_BTN), joystickButtonClick, FALLING);
  // end pin setup

  Serial.begin(9600);
  
  // lcd setup
  applySettings();
  lcd.createChar(0,downArrowSymbol);
  lcd.createChar(1,upArrowSymbol);
  lcd.createChar(2,bothArrowSymbol);
  lcd.createChar(3,clickSymbol);
  lcd.createChar(4,enterSymbol);

  lcd.begin(16, 2);

  // end lcd setup
  drawMenu(menuTitles[1], true, menuItems[selectedItem], menuItemsCount, false);
  
}

void loop() {
  
  computeJoystickValues();

  switch (systemState) {
    case SYSTEM_STATE_GAME_LOOP:
      gameLoop();
      break;
    case SYSTEM_STATE_MENU:
      menu();
      break;
      
    case SYSTEM_STATE_MENU_ABOUT:
      about();
      break;

    case SYSTEM_STATE_MENU_HIGHSCORES:
      highscores();
      break;

    case SYSTEM_STATE_MENU_SETTINGS:
      settings();
      break;
  }
}
