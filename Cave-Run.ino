#include <LiquidCrystal.h>
#include <LedControl.h>

/******** HARDWARE *********/

#include "Menu.h"
/***** LCD *****/

#define LCD_RS 7 
#define LCD_EN 6 
#define LCD_D4 5 
#define LCD_D5 4 
#define LCD_D6 9
#define LCD_D7 2 
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
#define JOYSTICK_BTN 1

#define JOYSTICK_MIN_TRESHOLD 200
#define JOYSTICK_MAX_TRESHOLD 800

/******** END HARDWARE *********/

/******** GLOBAL STATES *********/

#define SYSTEM_STATE_SPLASH 0
#define SYSTEM_STATE_MENU 1
#define SYSTEM_STATE_MENU_SETTINGS 2
#define SYSTEM_STATE_MENU_HIGHSCORE 3
#define SYSTEM_STATE_MENU_ABOUT 4
#define SYSTEM_STATE_GAME 10
#define SYSTEM_STATE_GAME_END 11

/******** END GLOBAL STATES *********/

int systemState = SYSTEM_STATE_MENU;

int joystickX = 0;
int joystickY = 0;

#define JOYSTICK_IDLE 0
#define JOYSTICK_UP 1
#define JOYSTICK_DOWN 2

int menuDirection = JOYSTICK_IDLE;

String menuTitle = "    Cave Run";
String menuItems[] = {"Start Game", "Highscores", "Settings", "About"};
String displayedMenuItems[2];
const int menuItemsCount = 4;

byte downArrow[8] = {
  B00000,
  B00000,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000,
  B00000
};

byte upArrow[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00000,
  B00000
};

byte bothArrow[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};


int selectedMenuItem = 0;
bool isJoystickMovedMenu = false;

void setup() {
  // pin setup

  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_BTN, INPUT_PULLUP);
  
  // end pin setup

  Serial.begin(9600);

  // lcd setup
  
  analogWrite(LCD_V0, 150);
  lcd.createChar(0,downArrow);
  lcd.createChar(1,upArrow);
  lcd.createChar(2,bothArrow);
  lcd.begin(16, 2);

  // end lcd setup

  drawMenu();
  
}

void updateMenu(short menuAction) {
  if (menuAction == JOYSTICK_DOWN && selectedMenuItem < (menuItemsCount-1)) {
    selectedMenuItem += 1;
  }

  if (menuAction == JOYSTICK_UP && selectedMenuItem > 0) {
    selectedMenuItem -= 1;
  }

  
  
}

void drawMenu() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(menuTitle);
  lcd.setCursor(0,1);
  lcd.write(">");
  lcd.print(menuItems[selectedMenuItem]);

  short caretCharacter;
  switch (selectedMenuItem) {
    case 0:
      caretCharacter = 0;
      break;
    case (menuItemsCount-1):
      caretCharacter = 1;
      break;
    default:
      caretCharacter = 2;
      break;
  }
  
  lcd.setCursor(15,1);
  lcd.write(byte(caretCharacter));
  
}

void menu() {
  
    short menuAction = getMenuAction();

    if (menuAction) {
      updateMenu(menuAction);
      drawMenu();
    }
}

short getMenuAction() {

  short reading = getMenuReading();

  if (!reading) {
    isJoystickMovedMenu = false;
    return JOYSTICK_IDLE;
  }

  if (!isJoystickMovedMenu) {
    isJoystickMovedMenu = true;
    return reading;
  }

  return JOYSTICK_IDLE;
}

short getMenuReading() {
  if (joystickY >= JOYSTICK_MIN_TRESHOLD && joystickY <= JOYSTICK_MAX_TRESHOLD) {
    return JOYSTICK_IDLE;
  }

  if (joystickY < JOYSTICK_MIN_TRESHOLD) {
    return JOYSTICK_UP;
  }
  
  return JOYSTICK_DOWN;
}

void getRawJoystickValues() {
  joystickX = analogRead(JOYSTICK_X);
  joystickY = analogRead(JOYSTICK_Y);
}

void loop() {
  
  getRawJoystickValues();
  switch (systemState) {
    case SYSTEM_STATE_MENU:
      menu();
      break;
  }
}
