#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>
/******** HARDWARE *********/

#include "Menu.h"
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

/******** END HARDWARE *********/

/******** GLOBAL STATES *********/

#define SYSTEM_STATE_SPLASH 0
#define SYSTEM_STATE_MENU 1
#define SYSTEM_STATE_MENU_SETTINGS 2
#define SYSTEM_STATE_MENU_HIGHSCORES 3
#define SYSTEM_STATE_MENU_ABOUT 4
#define SYSTEM_STATE_GAME 10
#define SYSTEM_STATE_GAME_END 11

/******** END GLOBAL STATES *********/

volatile int systemState = SYSTEM_STATE_MENU;

// joystick
int joystickX = 0;
int joystickY = 0;
bool joystickBtn = LOW;
bool didReadJoystick = false;

int rawJoystickX = 0;
int rawJoystickY = 0;
bool rawJoystickBtn = HIGH;

#define AXIS_IDLE 0
#define AXIS_POSITIVE 1
#define AXIS_NEGATIVE -1

volatile long lastDebounceTime = 0;
short debounceDelay = 300;

// end joystick

// menu
int menuDirection = AXIS_IDLE;
String menuTitles[] = {"Welcome", "Cave Run", "Settings", "Hall of Fame","About"};
String menuItems[] = {"Start Game", "Settings", "Highscores", "About"};
const int menuItemsCount = 4;
const int highscoreItemsCount = 3;
const int settingsItemsCount = 3;

String settingsItems[] = {"Your Name", "Difficulty", "Contrast"};
String aboutText[] = {"My Github:", "git.io/JMQEj"};

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

int selectedItem = 0;
volatile bool shouldRedrawMenu = false;
// end menu

// high score

short savedScores[3];
char savedNames[3][6];

struct Player {
  short score;
  char name[6];
};
Player scores[3];
// end high score

void setup() {
  // pin setup

  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
  
  EEPROM.get(0, scores);

  attachInterrupt(digitalPinToInterrupt(JOYSTICK_BTN), joystickButtonClick, FALLING);
  // end pin setup

  Serial.begin(9600);
  // lcd setup
   Serial.println(scores[0].name);  
  analogWrite(LCD_V0, 150);
  lcd.createChar(0,downArrowSymbol);
  lcd.createChar(1,upArrowSymbol);
  lcd.createChar(2,bothArrowSymbol);
  lcd.createChar(3,clickSymbol);
  lcd.begin(16, 2);

  // end lcd setup
  drawMenu(menuTitles[1], true, menuItems[selectedItem], menuItemsCount);
  
}

void joystickButtonClick() {

  volatile int currentTime = millis();
  if (currentTime - lastDebounceTime < debounceDelay) { 
    return;
  }

   
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
    
  }

 
//  if (systemState == SYSTEM_STATE_UNLOCKED) {
//    systemState = SYSTEM_STATE_LOCKED;
//    
//    return;
//  }
//
//  if (systemState == SYSTEM_STATE_LOCKED) {
//    systemState = SYSTEM_STATE_UNLOCKED;
//    EEPROM.put(eeAddressDigitMemory, digitMemory);
//    return;
//  }
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

void drawMenu(String title, bool showCaret, String option, short optionsCount) {
  lcd.clear();
  short titleCursorPos = (15 - title.length()) / 2 + 1;
  
  lcd.setCursor(titleCursorPos,0);
  lcd.print(title);
  lcd.setCursor(0,1);

  if (showCaret) {
    lcd.print("> ");
  }
  lcd.print(option);

   int const lastOption = optionsCount - 1;
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
  
    short userInput = getMenuUserInput();

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
  short userInput = getMenuUserInput();

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
short getMenuUserInput() {

  short reading = joystickY;
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

void loop() {
  
  computeJoystickValues();
  
  switch (systemState) {
    case SYSTEM_STATE_MENU:
      menu();
      break;
    case SYSTEM_STATE_MENU_ABOUT:
      about();
      break;

    case SYSTEM_STATE_MENU_HIGHSCORES:
      highscores();
      break;
  }
}
