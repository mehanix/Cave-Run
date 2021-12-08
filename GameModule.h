#include <assert.h>

struct Coordinate {
  short x, y;
    
  bool operator==(const Coordinate& a) const
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
bool shouldRedrawLcd = true;
byte currentLevel = 0;
byte gameState = SYSTEM_STATE_GAME_SETUP;

short startTime = 0;

struct Player {
  Coordinate pos = {1, 1};
  Coordinate previousPos = {1, 1};
  byte lives = 3;
  long long lastMoveTime = 0;
  bool shouldRedraw = true;
} player;

struct RoomData {
    Coordinate pos; //upper right corner
    short size;
    short keyCount = 0;
    short bombCount = 0;
    Coordinate keys[MAX_KEY_COUNT];
    Coordinate bombs[MAX_BOMB_COUNT];
    Coordinate exitPosition;
} level;



void reset() {
    for (int i = 0; i < MAP_SIZE; i++) {
      for (int j = 0; j < MAP_SIZE; j++) {
        gameMap[i][j] = 0;
      }
    }
}

Coordinate getObjectRegion(Coordinate obj) {
  return {obj.x / MATRIX_SIZE, obj.y / MATRIX_SIZE }; 
}

void drawLcdText() {

  lcd.setCursor(0,0);
  lcd.print("Lives:");
  
//  for (int i = 0; i < player.lives; i++) {
//    lcd.write(byte(5));
//  }

  lcd.setCursor(0,1);
  lcd.print("Time:");
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
  shouldRedrawMap = false;
  
}
/**
 * Draws room size based on difficulty and currentLevel.
 * How game difficulty increases room size:
 *  - Easy: 2 small rooms, 2 medium rooms, then large rooms until time runs out 
 *  - Medium: 1 small room, 2 medium rooms, then large rooms until time runs out
 *  - Hard: 2 medium rooms then large til time runs out
 */
void generateMap(int difficulty) {
  
  byte roomSize = (difficulty + 1) * 4;

  for(int i = 0; i < roomSize; i++) {
     gameMap[i][0] = 1;
     gameMap[i][roomSize - 1] = 1;

  }

  for(int j = 0; j < roomSize; j++) {
      gameMap[0][j] = 1;
      gameMap[roomSize - 1][j] = 1;
  }

}

void generateObjects(short difficulty) {
  level.keyCount = 3;
  level.bombCount = 3;
  level.keys[0] = {2,2};
  level.keys[1] = {5,6};
  level.keys[2] = {10,9};  
  level.bombs[0] = {5,5};

}

void generateLevel(short difficulty) {
  // POC, first level. 3 keys, 3 bombs, spawned at fixed positions for now that fit, 8x8 room.

  generateMap(difficulty);
  generateObjects(difficulty);
  
}
void gameSetup() {
  reset();
  generateLevel(2);
}

void drawPlayer() {
  
  // should redraw => player moved, so redraw what is needed
  if (player.shouldRedraw) {
    
    matrix.setLed(0, player.previousPos.x % 8, player.previousPos.y % 8, false);
    
    matrix.setLed(0, player.pos.x % 8, player.pos.y % 8, true);
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
}
void gameLoop() {
  
  switch (gameState) {
    case SYSTEM_STATE_GAME_SETUP:
      startTime = millis();
      lcd.clear();
      gameSetup();
      drawMap();    
      drawPlayer();
      drawLcdText();
      gameState = SYSTEM_STATE_GAME_LOOP;
      break;
      
    case SYSTEM_STATE_GAME_LOOP:
      updatePlayer();
      drawMap();
      drawPlayer();
      break;
  }
}
