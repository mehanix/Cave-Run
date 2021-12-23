/**
 * SystemModule.h - Areas of concern: global system state, interpreting raw data from hardware, saving/loading from memory
 */

/****** HARDWARE ******/
LedControl matrix = LedControl(MATRIX_DIN_PIN, MATRIX_CLOCK_PIN, MATRIX_LOAD_PIN, 1);

/***** SYSTEM *****/
volatile int systemState = SYSTEM_STATE_SPLASH;

/***** JOYSTICK *****/
int joystickX = 0;
int joystickY = 0;
bool joystickBtn = LOW;
bool didReadJoystick = false;

int rawJoystickX = 0;
int rawJoystickY = 0;
bool rawJoystickBtn = HIGH;

volatile unsigned long long lastDebounceTime = 0;

/***** LCD *****/
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

byte downArrowSymbol[8] = {
    B00000,
    B00000,
    B00100,
    B10101,
    B01110,
    B00100,
    B00000,
    B00000};

byte upArrowSymbol[8] = {
    B00000,
    B00000,
    B00100,
    B01110,
    B10101,
    B00100,
    B00000,
    B00000};

byte triangleSymbol[8] = {
    B00100,
    B01110,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};

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

byte heartSymbol[8] = {
    B00000,
    B01010,
    B11111,
    B11111,
    B01110,
    B00100,
    B00000,
    B00000,
};

/***** MATRIX ******/
byte happyMatrixSymbol[8] = {
    B11111111,
    B10000001,
    B10100101,
    B10000001,
    B10100101,
    B10111101,
    B10000001,
    B11111111,
};

byte sadMatrixSymbol[8] = {
    B11111111,
    B10000001,
    B10100101,
    B10000001,
    B10111101,
    B10100101,
    B10000001,
    B11111111,
};

byte menuMatrixSymbol[8] = {
    B11111111,
    B11000011,
    B10111101,
    B10100101,
    B10100101,
    B10111101,
    B11000011,
    B11111111};

byte aboutMatrixSymbol[8] = {
    B00000000,
    B00100010,
    B01110111,
    B01111111,
    B01111111,
    B00111110,
    B00011100,
    B00001000};

byte settingsMatrixSymbol[8] = {
    B00000000,
    B00000010,
    B00000010,
    B00001010,
    B00001010,
    B00101010,
    B00101010,
    B10101010};

byte highscoreMatrixSymbol[8] = {
    B01111110,
    B01111110,
    B01111110,
    B01111110,
    B00111100,
    B00011000,
    B00011000,
    B01111110};

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

void setWarningLed(short value) {
  analogWrite(WARNING_LED_PIN, value);
}

void setMatrixImage(byte image[]) {
  for (int i = 0; i < MATRIX_SIZE; i++) {
    matrix.setRow(0, i, image[i]);
  }
}
