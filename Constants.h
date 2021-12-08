/******** HARDWARE *********/

/***** LCD *****/

#define LCD_RS 8 
#define LCD_EN 6 
#define LCD_D4 A5 
#define LCD_D5 4 
#define LCD_D6 9
#define LCD_D7 13
#define LCD_V0 3
          
/***** MATRIX *****/

#define MATRIX_DIN_PIN 12
#define MATRIX_CLOCK_PIN 11
#define MATRIX_LOAD_PIN 10

#define MATRIX_SIZE 8
#define MATRIX_BRIGHTNESS 2

/***** JOYSTICK *****/

#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BTN 2

#define JOYSTICK_MIN_TRESHOLD 200
#define JOYSTICK_MAX_TRESHOLD 800

#define AXIS_IDLE 0
#define AXIS_POSITIVE 1
#define AXIS_NEGATIVE -1

#define WARNING_LED_PIN 5

/******** END HARDWARE *********/

/******** GLOBAL STATES *********/

#define SYSTEM_STATE_SPLASH 0
#define SYSTEM_STATE_MENU 1
#define SYSTEM_STATE_MENU_SETTINGS 2
#define SYSTEM_STATE_MENU_HIGHSCORES 3
#define SYSTEM_STATE_MENU_ABOUT 4
#define SYSTEM_STATE_SETTINGS_EDITING 5
#define SYSTEM_STATE_GAME 6
#define SYSTEM_STATE_GAME_SETUP 10
#define SYSTEM_STATE_GAME_LOOP 11
#define SYSTEM_STATE_GAME_END 12

/******** END GLOBAL STATES *********/

/******** SETTINGS BOUNDS ***********/
// for contrast, brightness, etc
#define SETTINGS_MIN_ADJUSTMENT_VALUE 1
#define SETTINGS_MAX_ADJUSTMENT_VALUE 9

#define SETTINGS_ADJUSTMENT_INCREMENT 16

#define SETTINGS_MIN_DIFFICULTY 1
#define SETTINGS_MAX_DIFFICULTY 3

#define SETTING_DIFFICULTY 0
#define SETTING_LCD_CONTRAST 1
#define SETTING LCD_BRIGHTNESS 2
#define SETTING_MATRIX_BRIGHTNESS 3

/******** END SETTINGS BOUNDS *******/

/***************** GAME *********************/

#define ROOM_SIZE_SMALL 8
#define ROOM_SIZE_MEDIUM 12
#define ROOM_SIZE_LARGE 16

#define MAP_SIZE 20
#define MAX_KEY_COUNT 5
#define MAX_BOMB_COUNT 5
#define MAX_LEVEL_COUNT 4

#define STARTING_ROOM_X 3
#define STARTING_ROOM_Y 3

#define DIFFICULTY_EASY 1
#define DIFFICULTY_MEDIUM 2
#define DIFFICULTY_HARD 3

#define DIRECTION_RIGHT 0
#define DIRECTION_DOWN 1

#define PLAYER_MOVEMENT_DELAY 150

#define WALL 1
#define FLOOR 0

#define GAME_DURATION_MS 60000
// game states

/**************** END_GAME **************/
