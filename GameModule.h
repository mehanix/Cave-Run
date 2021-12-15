#include <assert.h>

struct Coordinate {
  short x, y;
    
  bool operator == (const Coordinate& a) const
  {
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

struct Player {
  Coordinate pos = {1, 1};
  Coordinate previousPos = {1, 1};
  byte lives = 3;
  long long lastMoveTime = 0;
  bool shouldRedraw = true;
  byte keysLeft;
  short score = 0;
  bool blinkStatus = true;
  long long lastBlinkTime = 0;
} player;

struct RoomData {
    short size;
    short keyCount = 3;
    short bombCount = 1;
    Coordinate keys[MAX_KEY_COUNT];
    Coordinate bombs[MAX_BOMB_COUNT];
    Coordinate doorPos;
    bool isDoorOpen = false;
} level;

const byte difficultyItemCountRanges[4][2] = {
  {0,0},
  {3,5},
  {4,7},
  {5,8}
};
const byte configRoomCountsTilChange[][2] = {{0,0}, {3,3}, {2,2}, {1,1}};
byte roomCountsTilChange[4][2];
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
  for (auto &key: level.keys) {
    key = {};  
  }

  for (auto &bomb: level.bombs) {
    bomb = {};  
  }
  level.doorPos = {0,0};
  level.isDoorOpen = false;

  // player-related resets
  
  player.pos = {1, 1};
  player.previousPos = {1, 1};
  
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
  for (int i = 0; i < sizeof(configRoomCountsTilChange)/sizeof(Coordinate); i++) {
    roomCountsTilChange[i][0] = configRoomCountsTilChange[i][0];
    roomCountsTilChange[i][1] = configRoomCountsTilChange[i][1];

  }
  currentLevel = 0;
  gameState = SYSTEM_STATE_GAME_SETUP;
  
  elapsedTime = 0;
  lcdTime = 0;
}

Coordinate getObjectRegion(Coordinate obj) {
  return {obj.x / MATRIX_SIZE, obj.y / MATRIX_SIZE }; 
}


void drawLcdPlayerStats() {
  
  // level
  lcd.setCursor(4,0);
  lcd.print(currentLevel);
  
  // lives
  lcd.setCursor(6,0);
  lcd.print("   ");
  lcd.setCursor(6,0);
  for (int i = 0; i < player.lives; i++) {
    lcd.write(byte(5));
  }
  
  // keys left or door. not enough lcd space for both
  lcd.setCursor(15,0);
  lcd.print(player.keysLeft);
 
  if (player.keysLeft == 0) {
      lcd.setCursor(7,1);
      lcd.print("Door Open!");
  }
  
  
}

void drawLcdText() {

  lcd.setCursor(0,0);
  lcd.print("Lvl:");
  
  lcd.setCursor(10,0);
  lcd.print("Keys:");
  
  lcd.setCursor(0,1);
  lcd.print("Time:");
  lcd.print(GAME_DURATION);

  lcd.setCursor(7,1);
  lcd.print(" Pts:");

  drawLcdPlayerStats();
}

void drawLcdTime(byte lcdTime) {
  lcd.setCursor(5, 1);
  if (lcdTime < 10) {
    lcd.print("  ");
    lcd.setCursor(5, 1);
  }
  lcd.print(lcdTime);
}


void drawMap() {

  short regionX = displayedRegion.x * MATRIX_SIZE;
  short regionY = displayedRegion.y * MATRIX_SIZE;
  if(!shouldRedrawMap) {
    return;
  }

  // walls
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      matrix.setLed(0, i, j, gameMap[regionX + i][regionY + j]);
    }
  }

  //objects 
  for (auto &key: level.keys) {
    Coordinate keyRegion = getObjectRegion(key);
    if (keyRegion == displayedRegion) {
      matrix.setLed(0, key.x % MATRIX_SIZE, key.y % MATRIX_SIZE, true);
    }
  }

  player.previousPos = player.pos;
  shouldRedrawMap = false;
  
  
}

void updatePowerup() {
  if (!isPowerupActive) {
    return;
  }

  
}
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
  shouldRedrawScore = true;
}


/**
 * Draws room size based on difficulty and currentLevel.
 * How game difficulty increases room size:
 *  - Easy: 3 small rooms, 3 medium rooms, then large rooms until time runs out 
 *  - Medium: 2 small room, 2 medium rooms, then large rooms until time runs out
 *  - Hard: 1 of each then large til time runs out
 */
void generateMap(byte roomSize) {
  
  for(int i = 0; i < roomSize; i++) {
     gameMap[i][0] = 1;
     gameMap[i][roomSize - 1] = 1;

  }

  for(int j = 0; j < roomSize; j++) {
      gameMap[0][j] = 1;
      gameMap[roomSize - 1][j] = 1;
  }

  level.doorPos.x = 0;
  level.doorPos.y = 5;

}

bool checkIfColliding(Coordinate point) {

  Coordinate playerPos {1, 1};
  // player start
  if (point == playerPos) {
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

  // todo door
  // walls are already accounted for

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

void generateObjects() {
  
  player.keysLeft = level.keyCount;

  Serial.println(level.keyCount);

  for (int i = 0; i < level.keyCount; i++) {
    level.keys[i] = generateNonCollidingPoint();
    Serial.print(level.keys[i].x);
    Serial.print(" ");
    Serial.println(level.keys[i].y);
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
  if (roomCountsTilChange[difficulty][roomTypeIndex] == 0) {
      roomTypeIndex += 1;
  }
  // mark the room as generated
  roomCountsTilChange[difficulty][roomTypeIndex] -= 1;

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

//  Serial.println(level.size);
//  Serial.println(int(difficulty));
//  
  Serial.println("ranges:");
  Serial.println(difficultyItemCountRanges[difficulty][0] - roomSizeModifier);
  Serial.println(difficultyItemCountRanges[difficulty][1] - roomSizeModifier);

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
  lcdTime = 60;
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
    
    matrix.setLed(0, player.previousPos.x % 8, player.previousPos.y % 8, false);
    
    matrix.setLed(0, player.pos.x % 8, player.pos.y % 8, player.blinkStatus);
    player.shouldRedraw = false;
  }
}

Coordinate computePosition() {
  Coordinate answer = player.pos;

  if (joystickX != AXIS_IDLE || joystickY != AXIS_IDLE) {
      player.shouldRedraw = true;
  }
  answer.x -= joystickY;
  answer.y += joystickX;

  

  if (gameMap[answer.x][answer.y] == WALL) {
    return player.pos;
  }
  return answer;
}

void updatePlayer() {
  long long currentTime = millis();

  if (bombWentOff) {
    return; // time penalty for setting bomb off, cannot move.
  }

  if (currentTime - player.lastMoveTime < PLAYER_MOVEMENT_DELAY) {
    return;
  }
  
  player.lastMoveTime = currentTime;
  player.previousPos = player.pos;

  player.pos = computePosition();

  
  Coordinate newDisplayRegion = {0,0};
  newDisplayRegion.x = player.pos.x / 8;
  newDisplayRegion.y = player.pos.y / 8;
  
  if (!(newDisplayRegion == displayedRegion)) {
    displayedRegion = newDisplayRegion;  
    shouldRedrawMap = true;
  }

  if (currentTime - player.lastBlinkTime < PLAYER_BLINK_INTERVAL) {
    return;
  }

  player.lastBlinkTime = currentTime;
  player.blinkStatus = !player.blinkStatus;
  player.shouldRedraw = true;
  
}

float getDistance(Coordinate a, Coordinate b) {

  return sqrt(pow((a.x-b.x),2) + pow((a.y - b.y), 2));
}

short getNearestBombDistance() {
  float shortestDistance = MAP_SIZE*2;
  
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
      setWarningLed(max(240 - 60*nearestBomb, 0));
  }
}

void endGameDisplay(byte image[], String message) {
  
  setMatrixImage(image);
  lcd.setCursor((15 - message.length())/2, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);
  lcd.print("Your score:");
  lcd.print(player.score);

}

void endGame(byte reason) {
  systemState = SYSTEM_STATE_GAME_END;
  lcd.clear();

  switch (reason) {
    case GAME_END_TIMEOUT:
      endGameDisplay(happyMatrixSymbol, "== Time's up!  ==");
      break;

    case GAME_END_NO_LIVES:
      endGameDisplay(sadMatrixSymbol, "==  You died!  ==");
      break; 
  }

  currentPlayer.score = player.score;
  saveScore();
}


void updateTime() {
  
  elapsedTime = (millis() - startTime)/1000;
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

  lcd.setCursor(12,1);
  lcd.print("    ");
  lcd.setCursor(12,1);
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
    nextLevel(); // POC only; we'd switch to the next level here normally
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
