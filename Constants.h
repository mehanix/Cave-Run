/******** HARDWARE *********/

/***** LCD *****/

#define LCD_RS 8 
#define LCD_EN 6 
#define LCD_D4 A5 
#define LCD_D5 4 
#define LCD_D6 9
#define LCD_D7 13
#define LCD_V0 5
#define LCD_BACKLIGHT 10
          
/***** MATRIX *****/

#define MATRIX_DIN_PIN 12
#define MATRIX_CLOCK_PIN 11
#define MATRIX_LOAD_PIN A2

#define MATRIX_SIZE 8
#define MATRIX_BRIGHTNESS 2

/***** JOYSTICK *****/

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BTN 2

#define JOYSTICK_MIN_TRESHOLD 400
#define JOYSTICK_MAX_TRESHOLD 600

#define AXIS_IDLE 0
#define AXIS_POSITIVE 1
#define AXIS_NEGATIVE -1

#define WARNING_LED_PIN A3
#define BUZZER_PIN 3
#define RANDOM_PIN A4
#define BOMB_EXPLOSION_INTERVAL 1500
#define LED_FLASHING_INTERVAL 200

#define BAUD 9600
/******** END HARDWARE *********/

/******** GLOBAL STATES *********/

#define SYSTEM_STATE_SPLASH 0
#define SYSTEM_STATE_MENU 1
#define SYSTEM_STATE_MENU_SETTINGS 2
#define SYSTEM_STATE_MENU_HIGHSCORES 3
#define SYSTEM_STATE_MENU_ABOUT 4
#define SYSTEM_STATE_SETTINGS_EDITING 5
#define SYSTEM_STATE_GAME 6

#define SYSTEM_STATE_NAME_EDIT_LOCKED 7
#define SYSTEM_STATE_NAME_EDIT_UNLOCKED 8
#define SYSTEM_STATE_NAME_EDIT 9

#define SYSTEM_STATE_GAME_SETUP 10
#define SYSTEM_STATE_GAME_LOOP 11
#define SYSTEM_STATE_GAME_END 12


/******** END GLOBAL STATES *********/

/************* MENU *****************/

#define MENU_TITLE_WELCOME "Welcome"
#define MENU_TITLE_GAME "Cave Run"
#define MENU_TITLE_SETTINGS "Settings"
#define MENU_TITLE_HIGHSCORES "Hall of Fame"
#define MENU_TITLE_ABOUT "About"


/*********** END MENU ***************/

/******** SETTINGS BOUNDS ***********/
// for contrast, brightness, etc
#define SETTINGS_MIN_ADJUSTMENT_VALUE 1
#define SETTINGS_MAX_ADJUSTMENT_VALUE 9

#define SETTINGS_ADJUSTMENT_INCREMENT 16

#define SETTINGS_MIN_DIFFICULTY 1
#define SETTINGS_MAX_DIFFICULTY 3

#define SETTING_NAME 0
#define SETTING_DIFFICULTY 1
#define SETTING_LCD_CONTRAST 2
#define SETTING LCD_BRIGHTNESS 3
#define SETTING_MATRIX_BRIGHTNESS 4
#define SETTING_BACK_TO_MENU 5

#define SETTING_NUMBERED_ITEM_PADDING 2

#define NAME_LENGTH 5

/******** END SETTINGS BOUNDS *******/

/***************** GAME *********************/

#define ROOM_SIZE_SMALL 8
#define ROOM_SIZE_MEDIUM 12
#define ROOM_SIZE_LARGE 16

#define MAP_SIZE 16
#define MAX_KEY_COUNT 9
#define MAX_BOMB_COUNT 9
#define LIVES_COUNT 3

#define DIFFICULTY_EASY 1
#define DIFFICULTY_MEDIUM 2
#define DIFFICULTY_HARD 3

#define DIRECTION_RIGHT 0
#define DIRECTION_DOWN 1

#define PLAYER_MOVEMENT_DELAY 150
#define PLAYER_BLINK_INTERVAL 100
#define PLAYER_POWERUP_DURATION 1000
#define WALL 1
#define FLOOR 0

#define GAME_DURATION 60

#define GAME_END_TIMEOUT 0
#define GAME_END_NO_LIVES 1
#define GAME_END_EXIT 2

#define KEY_PICKUP 0
#define BOMB_PICKUP 1
#define DOOR_OPEN 2
#define DOOR_ENTER 3

#define DOOR_POS_X 0
#define DOOR_POS_Y 5

/**************** END_GAME **************/

/*********** LCD CONSTANTS **************/

#define LCD_CHARACTER_LENGTH 15
// splash
#define SPLASH_TEXT_POS 4,0
#define SPLASH_TIME_MS 2000
#define SPLASH_TEXT "Welcome!"

#define ABOUT_NAME_TEXT_POS 0,0
#define ABOUT_GITHUB_TEXT_POS 0,1

#define LEVEL_TEXT_POS 4,0
#define LIVES_TEXT_POS 6,0
#define KEYS_LEFT_TEXT_POS 15,0
#define DOOR_OPEN_TEXT_POS 7,1

#define LEVEL_LABEL_TEXT_POS 0,0
#define KEY_LABEL_TEXT_POS 10,0
#define TIME_LABEL_TEXT_POS 0,1
#define SCORE_LABEL_TEXT_POS 7,1

#define TIME_TEXT_POS 5,1
#define SCROLLBAR_TEXT_POS 15,1

#define NAME_TEXT_POS 0,0
#define SAVE_TEXT_POS 11,0
#define SAVE_CURSOR_POS 12, 1
#define SAVE_TEXT "Save"
#define SELECTION_CARET "^"

#define DOOR_TEXT "Door:Open"
#define LEVEL_LABEL_TEXT "Lvl:"
#define KEY_LABEL_TEXT "Keys:"
#define TIME_LABEL_TEXT "Time:"
#define SCORE_LABEL_TEXT " Pts:"

#define DOWN_SYMBOL 0
#define UP_SYMBOL 1
#define BOTH_SYMBOL 2
#define CLICK_SYMBOL 3
#define ENTER_SYMBOL 4
#define HEART_SYMBOL 5

/********** END LCD CONSTANTS ***********/

/********** EEPROM MEMORY ***************/

#define EEPROM_SETTINGS_MEMORY_LOCATION 100
#define EEPROM_HIGHSCORES_MEMORY_LOCATION 0

/********** END EEPROM MEMORY ***********/
