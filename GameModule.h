/**
 * GameModule.h - Game loop, scoring, level generation, item generation, player movement
 */

struct Coordinate {
  short x, y;

  bool operator==(const Coordinate &a) const {
    return (x == a.x && y == a.y);
  }
};

bool gameMap[MAP_SIZE][MAP_SIZE];

/**
 * Represents the top-left corner of the currently viewable area of the map on the LED Matrix.
 */
Coordinate displayedRegion = {0, 0};

bool shouldRedrawMap = true;
bool shouldRedrawScore = true;
bool bombWentOff = false;

bool ledExplosionState = 0;

long long lastExplosionTime;
long long lastLedChangeTime;

byte currentLevel = 0;
byte gameState = SYSTEM_STATE_GAME_SETUP;
byte difficulty;

long long startTime = 0;
long long elapsedTime = 0;
byte lcdTime = 0;

long long powerupStartTime = 0;
bool isPowerupActive = false;
bool isPowerupAvailable = true;
const Coordinate playerStartingPos = {1, 1};  // can't define an entire object; i tried; so const it is

struct Player {
  Coordinate pos = playerStartingPos;
  Coordinate previousPos = playerStartingPos;
  byte lives = LIVES_COUNT;
  long long lastMoveTime = 0;
  bool shouldRedraw = true;
  byte keysLeft;
  short score = 0;
  bool blinkStatus = true;
  long long lastBlinkTime = 0;
} player;

struct RoomData {
  short size;
  short keyCount = 0;
  short bombCount = 0;
  Coordinate keys[MAX_KEY_COUNT];
  Coordinate bombs[MAX_BOMB_COUNT];
  Coordinate doorPos;
  bool isDoorOpen = false;
} level;

const byte difficultyItemCountRanges[4][2] = {
    {0, 0},  // padding
    {3, 5},  // item count range for EASY difficulty - 3/5 keys/bombs per level
    {3, 6},  // same but for medium
    {3, 7}   // same but for hard
};
const byte configRoomCountsTilChange[4][2] = {{0, 0}, {3, 3}, {1, 2}, {0, 2}};
byte roomCountsTilChange[2];
byte roomTypeIndex = 0;

/**
 * Resets the object which holds the level that is being played - preparing for level generation
 */
void resetLevel() {
  // gameMap reset
  for (int i = 0; i < MAP_SIZE; i++) {
    for (int j = 0; j < MAP_SIZE; j++) {
      gameMap[i][j] = 0;
    }
  }

  // level reset
  level.size = 0;
  level.keyCount = 0;
  level.bombCount = 0;
  for (auto &key : level.keys) {
    key = {};
  }

  for (auto &bomb : level.bombs) {
    bomb = {};
  }
  level.doorPos = {DOOR_POS_X, DOOR_POS_Y};
  level.isDoorOpen = false;

  // player-related resets
  player.pos = playerStartingPos;
  player.previousPos = playerStartingPos;
}

/**
 * Resets the global variabiles specific to the game state.
 */
void resetGame() {
  resetLevel();

  player.lives = 3;
  player.lastMoveTime = 0;
  player.lastBlinkTime = 0;
  player.shouldRedraw = true;
  player.keysLeft;
  player.score = 0;

  isPowerupAvailable = true;

  level.isDoorOpen = false;
  difficulty = systemSettings.difficulty;

  displayedRegion = {0, 0};
  shouldRedrawMap = true;
  shouldRedrawScore = true;
  bombWentOff = false;

  ledExplosionState = 0;

  lastExplosionTime = 0;
  lastLedChangeTime = 0;

  roomTypeIndex = 0;
  //  for (int i = 0; i < 4; i++) {
  //    roomCountsTilChange[i][0] = configRoomCountsTilChange[i][0];
  //    roomCountsTilChange[i][1] = configRoomCountsTilChange[i][1];
  //
  //  }
  roomCountsTilChange[0] = configRoomCountsTilChange[difficulty][0];
  roomCountsTilChange[1] = configRoomCountsTilChange[difficulty][1];

  currentLevel = 0;
  gameState = SYSTEM_STATE_GAME_SETUP;

  elapsedTime = 0;
  lcdTime = 0;
}

Coordinate getObjectRegion(Coordinate obj) {
  return {obj.x / MATRIX_SIZE, obj.y / MATRIX_SIZE};
}

void drawLcdPlayerStats() {
  // level
  lcd.setCursor(LEVEL_TEXT_POS);
  lcd.print(currentLevel);

  // lives
  lcd.setCursor(LIVES_TEXT_POS);
  lcd.print("   ");
  lcd.setCursor(LIVES_TEXT_POS);
  for (int i = 0; i < player.lives; i++) {
    lcd.write(byte(HEART_SYMBOL));
  }

  // keys left or door. not enough lcd space for both
  lcd.setCursor(KEYS_LEFT_TEXT_POS);
  lcd.print(player.keysLeft);

  if (player.keysLeft == 0) {
    lcd.setCursor(DOOR_OPEN_TEXT_POS);
    lcd.print(DOOR_TEXT);
  }
}

void drawLcdText() {
  lcd.setCursor(LEVEL_LABEL_TEXT_POS);
  lcd.print(LEVEL_LABEL_TEXT);

  lcd.setCursor(KEY_LABEL_TEXT_POS);
  lcd.print(KEY_LABEL_TEXT);

  lcd.setCursor(TIME_LABEL_TEXT_POS);
  lcd.print(TIME_LABEL_TEXT);
  lcd.print(GAME_DURATION);

  lcd.setCursor(SCORE_LABEL_TEXT_POS);
  lcd.print(SCORE_LABEL_TEXT);

  drawLcdPlayerStats();
}

void drawLcdTime(byte lcdTime) {
  lcd.setCursor(TIME_TEXT_POS);

  if (lcdTime < 10) {  // single digits need padding
    lcd.print("  ");
    lcd.setCursor(TIME_TEXT_POS);
  }
  lcd.print(lcdTime);
}

void drawMap() {
  short regionX = displayedRegion.x * MATRIX_SIZE;
  short regionY = displayedRegion.y * MATRIX_SIZE;
  if (!shouldRedrawMap) {
    return;
  }

  // walls
  for (int i = 0; i < MATRIX_SIZE; i++) {
    for (int j = 0; j < MATRIX_SIZE; j++) {
      matrix.setLed(0, i, j, gameMap[regionX + i][regionY + j]);
    }
  }

  //objects
  for (auto &key : level.keys) {
    Coordinate keyRegion = getObjectRegion(key);
    if (keyRegion == displayedRegion) {
      matrix.setLed(0, key.x % MATRIX_SIZE, key.y % MATRIX_SIZE, true);
    }
  }

  player.previousPos = player.pos;
  shouldRedrawMap = false;
}

void drawBombs() {
  // draw remaining bombs on screen. if the bomb went off (its index is < 0) don't draw it
  for (int i = 0; i < level.bombCount; i++) {
    Coordinate bomb = level.bombs[i];
    Coordinate bombRegion = getObjectRegion(bomb);
    if (bombRegion == displayedRegion && bomb.x >= 0) {
      matrix.setLed(0, bomb.x % MATRIX_SIZE, bomb.y % MATRIX_SIZE, true);
    }
  }
}

/*
 * Clear the entire game and show bombs visible on this map section. 
 */
void updatePowerup() {
  if (!isPowerupActive) {
    return;
  }
  long long currentTime = millis();
  matrix.clearDisplay(0);

  drawBombs();
  if (currentTime - powerupStartTime < PLAYER_POWERUP_DURATION) {
    return;
  }
  shouldRedrawMap = true;
  player.shouldRedraw = true;
  isPowerupActive = false;
}
/**
 * Handles scoreable events by adding their respective formula values to the score. Most are time-based.
 * Penalizes stepping on bombs when the timer has a lot of time (don't rush through the game!)
 * Rewards opening and entering doors early (u got skill!)
 * 
 */
void updateScoreEvent(byte event) {
  switch (event) {
    case KEY_PICKUP:
      player.score += 100 - elapsedTime;
      break;
    case BOMB_PICKUP:
      player.score -= lcdTime;
      break;
    case DOOR_OPEN:
      player.score += 2 * lcdTime;
      break;
    case DOOR_ENTER:
      player.score += 2 * lcdTime;
      break;
  }

  if (level.isDoorOpen == false) {
    shouldRedrawScore = true;
  }
}

/**
 * Draws room size based on difficulty and currentLevel.
 * How game difficulty increases room size:
 *  - Easy: 3 small rooms, 3 medium rooms, then large rooms until time runs out 
 *  - Medium: 2 small room, 2 medium rooms, then large rooms until time runs out
 *  - Hard: 1 of each then large til time runs out
 */
void generateMap(byte roomSize) {
  for (int i = 0; i < roomSize; i++) {
    gameMap[i][0] = WALL;
    gameMap[i][roomSize - 1] = WALL;
  }

  for (int j = 0; j < roomSize; j++) {
    gameMap[0][j] = WALL;
    gameMap[roomSize - 1][j] = WALL;
  }

  level.doorPos.x = DOOR_POS_X;
  level.doorPos.y = DOOR_POS_Y;
}

/**
 * Checks if colliding with any other collectable or area-of-interest (no bomb in front of the door)
 */
bool checkIfColliding(Coordinate point) {
  // player start
  if (point == playerStartingPos) {
    return false;
  }

  // other objects
  // keys
  for (int i = 0; i < level.keyCount; i++) {
    if (point == level.keys[i]) {
      return false;
    }
  }

  // bombs
  for (int i = 0; i < level.bombCount; i++) {
    if (point == level.bombs[i]) {
      return false;
    }
  }

  // door
  if (point.x == level.doorPos.x + 1 && point.y == level.doorPos.y) {
    return false;
  }

  return true;
}

/**
 * Function which returns a valid coordinate for an object.
 * Valid coordinates are those which:
 *  - do not overlap the player
 *  - do not overlap the walls
 *  - are not placed in front of the exit
 *  - do not overlap other objects
 */
Coordinate generateNonCollidingPoint() {
  bool valid = false;
  Coordinate point;

  while (!valid) {
    point = {random(1, level.size - 1), random(1, level.size - 1)};
    valid = checkIfColliding(point);
  }

  return point;
}

/**
 * Handles collectable object generation - both keys and bombs
 */
void generateObjects() {
  player.keysLeft = level.keyCount;

  for (int i = 0; i < level.keyCount; i++) {
    level.keys[i] = generateNonCollidingPoint();
  }

  for (int i = 0; i < level.bombCount; i++) {
    level.bombs[i] = generateNonCollidingPoint();
  }
}

byte getRoomSizeModifier() {
  return 3 - level.size / 4;
}

/**
 * Depending on the current level and selected difficulty, generate a room of a certain size and certain item counts.
 */
void generateLevelParams() {
  // room size => a difficulty has a number of small/mid sized rooms, followed by large rooms
  // if i already generated enough types of this room, go to the next type of room
  if (roomCountsTilChange[roomTypeIndex] == 0) {
    roomTypeIndex += 1;
  }
  // mark the room as generated
  roomCountsTilChange[roomTypeIndex] -= 1;

  switch (roomTypeIndex) {
    case (0):
      level.size = ROOM_SIZE_SMALL;
      break;
    case (1):
      level.size = ROOM_SIZE_MEDIUM;
      break;
    default:
      level.size = ROOM_SIZE_LARGE;
      break;
  }

  // make key count also scale with room size
  byte roomSizeModifier = getRoomSizeModifier();

  level.keyCount = random(difficultyItemCountRanges[difficulty][0] - roomSizeModifier, difficultyItemCountRanges[difficulty][1] - roomSizeModifier);
  level.bombCount = random(difficultyItemCountRanges[difficulty][0] - roomSizeModifier, difficultyItemCountRanges[difficulty][1] - roomSizeModifier);
}

void generateLevel() {
  generateLevelParams();
  generateMap(level.size);
  generateObjects();
  shouldRedrawMap = true;
  shouldRedrawScore = true;
}
void gameSetup() {
  resetGame();
  generateLevel();
  lcdTime = GAME_DURATION;
}

void nextLevel() {
  resetLevel();
  currentLevel += 1;
  generateLevel();
  player.shouldRedraw = true;
  drawLcdText();
}

void drawPlayer() {
  // should redraw => player moved, so redraw what is needed
  if (player.shouldRedraw) {
    matrix.setLed(0, player.previousPos.x % MATRIX_SIZE, player.previousPos.y % MATRIX_SIZE, false);

    matrix.setLed(0, player.pos.x % MATRIX_SIZE, player.pos.y % MATRIX_SIZE, player.blinkStatus);
    player.shouldRedraw = false;
  }
}

/**
 * Collision detection. Don't pass through walls.
 */
Coordinate computePosition() {
  Coordinate answer = player.pos;

  if (joystickX != AXIS_IDLE || joystickY != AXIS_IDLE) {
    player.shouldRedraw = true;
  }
  answer.x -= joystickY;
  answer.y += joystickX;

  // if about to pass through wall, don't
  if (gameMap[answer.x][answer.y] == WALL) {
    return player.pos;
  }
  return answer;
}

/**
 * Player loop. Checks various areas of interest for updates - bombs, position, and makes sure to change the viewable map area if the player is exiting the 
 * visible space.
 */
void updatePlayer() {
  long long currentTime = millis();

  if (bombWentOff) {
    return;  // time penalty for setting bomb off, cannot move.
  }

  if (currentTime - player.lastMoveTime < PLAYER_MOVEMENT_DELAY) {
    return;
  }

  player.lastMoveTime = currentTime;
  player.previousPos = player.pos;

  player.pos = computePosition();

  // the map is split into areas of 8x8. its correspondent region id is a Coordinate object that counts how many multiples of 8 you're away from the starting zone
  Coordinate newDisplayRegion = {0, 0};
  newDisplayRegion.x = player.pos.x / MATRIX_SIZE;
  newDisplayRegion.y = player.pos.y / MATRIX_SIZE;

  if (!(newDisplayRegion == displayedRegion)) {
    displayedRegion = newDisplayRegion;
    shouldRedrawMap = true;
  }

  if (currentTime - player.lastBlinkTime < PLAYER_BLINK_INTERVAL) {
    return;
  }

  player.lastBlinkTime = currentTime;
  player.blinkStatus = !player.blinkStatus;
  
  if (!isPowerupActive) {
    player.shouldRedraw = true;
  }
}

/**
 * Euclidean distance
 */
float getDistance(Coordinate a, Coordinate b) {
  return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

/**
 * Iterate through all bombs and choose the one which is closest to you in order to update the radar accordingly.
 * May sound inefficient but it's fine since bomb count is such a small number 
 */
short getNearestBombDistance() {
  float shortestDistance = MAP_SIZE * 2;

  for (int i = 0; i < level.bombCount; i++) {
    if (player.pos == level.bombs[i])
      return 0;

    float distance = getDistance(player.pos, level.bombs[i]);
    if (distance < shortestDistance) {
      shortestDistance = distance;
    }
  }
  return shortestDistance;
}

/**
 * Removes by marking to an impossible coordinate. Redraws LCD stats as player lost a life.
 */
void removeBomb() {
  int index;
  player.lives -= 1;
  drawLcdPlayerStats();

  for (int i = 0; i < level.bombCount; i++) {
    if (player.pos == level.bombs[i]) {
      level.bombs[i] = {-100, -100};
      return;
    }
  }
}

void updateKeys() {
  // check if player is touching key, if he is, count the key
  int index;
  for (int i = 0; i < level.keyCount; i++) {
    if (player.pos == level.keys[i]) {
      level.keys[i] = {-1, -1};
      player.keysLeft -= 1;

      updateScoreEvent(KEY_PICKUP);
      drawLcdPlayerStats();
      return;
    }
  }
}

/**
 * Door behavior loop. If no keys left, open the door.
 */
void updateDoor() {
  if (player.keysLeft == 0 && gameMap[level.doorPos.x][level.doorPos.y]) {
    gameMap[level.doorPos.x][level.doorPos.y] = 0;

    drawLcdPlayerStats();
    shouldRedrawMap = true;
    updateScoreEvent(DOOR_OPEN);
    shouldRedrawScore = false;
  }
}

void updateBombRadar() {
  if (bombWentOff) {
    tone(BUZZER_PIN, 500);
    long long currentTime = millis();
    if (currentTime - lastExplosionTime > BOMB_EXPLOSION_INTERVAL) {
      bombWentOff = false;
      return;
    }

    if (currentTime - lastLedChangeTime > LED_FLASHING_INTERVAL) {
      ledExplosionState = !ledExplosionState;
      lastLedChangeTime = currentTime;
    }
    digitalWrite(WARNING_LED_PIN, ledExplosionState);
    return;
  }
  // get closest bomb and show on the radar a percentage corresponding to how close you are to it
  float nearestBomb = getNearestBombDistance();

  noTone(BUZZER_PIN);
  if (nearestBomb == 0) {
    bombWentOff = true;
    lastExplosionTime = millis();
    updateScoreEvent(BOMB_PICKUP);

    if (player.lives > 0) {
      removeBomb();
    }

  } else {
    setWarningLed(max(240 - 60 * nearestBomb, 0));
  }
}

void endGameDisplay(byte image[]) {
  String firstRow = "Game| ";
  String secondRow = "Over| score:";
  firstRow.concat(String(systemSettings.name));
  firstRow.concat("'s");
  secondRow.concat(String(player.score));

  setMatrixImage(image);
  lcd.setCursor(0, 0);
  lcd.print(firstRow);
  lcd.setCursor(0, 1);
  lcd.print(secondRow);
}

void endGame(byte reason) {
  systemState = SYSTEM_STATE_GAME_END;
  lcd.clear();

  switch (reason) {
    case GAME_END_TIMEOUT:
      endGameDisplay(happyMatrixSymbol);
      break;

    case GAME_END_NO_LIVES:
      endGameDisplay(sadMatrixSymbol);
      break;
  }

  currentPlayer.score = player.score;
  saveScore();
}

void updateTime() {
  elapsedTime = (millis() - startTime) / 1000;
  short timeLeft = GAME_DURATION - elapsedTime;
  if (lcdTime != timeLeft) {
    lcdTime = timeLeft;
    drawLcdTime(lcdTime);
  }
}

void updateScore() {
  if (!shouldRedrawScore) {
    return;
  }

  lcd.setCursor(12, 1);
  lcd.print("    ");
  lcd.setCursor(12, 1);
  lcd.print(player.score);
  shouldRedrawScore = false;
}

void checkEndConditions() {
  if (elapsedTime == GAME_DURATION) {
    endGame(GAME_END_TIMEOUT);
    return;
  }

  if (player.lives == 0) {
    endGame(GAME_END_NO_LIVES);
  }

  if (player.pos == level.doorPos) {
    updateScoreEvent(DOOR_ENTER);
    nextLevel();
    return;
  }
}

void gameLoop() {
  switch (gameState) {
    case SYSTEM_STATE_GAME_SETUP:
      elapsedTime = 0;

      startTime = millis();
      lcd.clear();
      gameSetup();
      drawMap();
      drawPlayer();
      drawLcdText();
      gameState = SYSTEM_STATE_GAME_LOOP;
      break;

    case SYSTEM_STATE_GAME_LOOP:
      updateTime();
      updateScore();
      updatePlayer();
      updateBombRadar();
      updateKeys();
      updateDoor();
      updatePowerup();

      drawMap();
      drawPlayer();

      checkEndConditions();
      break;
  }
}

void gameEndLoop() {
  // bomb radar should continue going off even after game end screen appears
  updateBombRadar();
}
