#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C

// Pin Definitions
#define BTN_UP    1
#define BTN_DOWN  2
#define BTN_LEFT  3
#define BTN_RIGHT 4
#define BTN_A     5
#define MOTOR     6

#define OLED_SDA  8
#define OLED_SCL  9

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =========================================================================
// DATA STRUCTURES & TYPES
// =========================================================================

struct Point { 
  int8_t x, y; 
};

struct Button {
  uint8_t pin;
  bool state;
  bool lastState;
  unsigned long lastDebounceTime;
};

struct Tetromino {
  uint8_t shape[4][4];
  int8_t size;
};

struct Bullet {
  int x;
  int y;
  bool active;
};

// =========================================================================
// STATE MACHINE & SYSTEM VARIABLES
// =========================================================================

enum SystemState { STATE_HOME, STATE_MENU, STATE_SNAKE, STATE_PONG, STATE_TETRIS, STATE_DODGE, STATE_FLAPPY, STATE_GAMEOVER };
SystemState currentState = STATE_HOME;
SystemState activeGame   = STATE_HOME;

int menuIndex = 0;
const int TOTAL_GAMES = 5;
const char* gameNames[] = {
  "1. Snake",
  "2. Pong",
  "3. Tetris",
  "4. Space Dodge",
  "5. Flappy Bird"
};

int score = 0;
int highScoreSnake = 0;
int highScorePong = 0;
int highScoreTetris = 0;
int highScoreDodge = 0;
int highScoreFlappy = 0;

Button btnUp    = {BTN_UP, HIGH, HIGH, 0};
Button btnDown  = {BTN_DOWN, HIGH, HIGH, 0};
Button btnLeft  = {BTN_LEFT, HIGH, HIGH, 0};
Button btnRight = {BTN_RIGHT, HIGH, HIGH, 0};
Button btnA     = {BTN_A, HIGH, HIGH, 0};

const unsigned long DEBOUNCE_DELAY = 25;
unsigned long exitHoldStartTime = 0;

// Forward Declarations
void initSnake();  void updateSnake();  void drawSnake();
void initPong();   void updatePong();   void drawPong();
void initTetris(); void updateTetris(); void drawTetris();
void initDodge();  void updateDodge();  void drawDodge();
void initFlappy(); void updateFlappy(); void drawFlappy();

// =========================================================================
// HARDWARE HELPERS & UTILITIES
// =========================================================================

void vibrate(int durationMs) {
  digitalWrite(MOTOR, HIGH);
  delay(durationMs);
  digitalWrite(MOTOR, LOW);
}

bool isPressed(Button &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastState) {
    b.lastDebounceTime = millis();
  }
  b.lastState = reading;
  if ((millis() - b.lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != b.state) {
      b.state = reading;
      if (b.state == LOW) return true;
    }
  }
  return false;
}

void checkExitCombination() {
  if (currentState != STATE_HOME && currentState != STATE_MENU && currentState != STATE_GAMEOVER) {
    if (digitalRead(BTN_LEFT) == LOW && digitalRead(BTN_RIGHT) == LOW) {
      if (exitHoldStartTime == 0) {
        exitHoldStartTime = millis();
      } else if (millis() - exitHoldStartTime > 1000) {
        vibrate(150);
        currentState = STATE_MENU;
        exitHoldStartTime = 0;
      }
    } else {
      exitHoldStartTime = 0;
    }
  }
}

void triggerGameOver() {
  vibrate(300);
  if (activeGame == STATE_SNAKE  && score > highScoreSnake)  highScoreSnake = score;
  if (activeGame == STATE_PONG   && score > highScorePong)   highScorePong = score;
  if (activeGame == STATE_TETRIS && score > highScoreTetris) highScoreTetris = score;
  if (activeGame == STATE_DODGE  && score > highScoreDodge)  highScoreDodge = score;
  if (activeGame == STATE_FLAPPY && score > highScoreFlappy) highScoreFlappy = score;
  currentState = STATE_GAMEOVER;
}

// =========================================================================
// RETRO BOOTUP ANIMATION
// =========================================================================

void runBootAnimation() {
  for (int r = 0; r <= 28; r += 4) {
    display.clearDisplay();
    display.drawRect(64 - r * 2, 32 - r, r * 4, r * 2, SSD1306_WHITE);
    display.display();
    delay(15);
  }
  
  vibrate(40);
  
  for (int i = 0; i < 128; i += 8) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(30, 20);
    display.print(F("RETRO MINI"));
    display.drawFastHLine(0, 32, i, SSD1306_WHITE);
    display.display();
    delay(10);
  }

  for (int w = 0; w <= 100; w += 10) {
    display.clearDisplay();
    display.setCursor(30, 15);
    display.print(F("RETRO MINI"));
    display.drawRect(14, 38, 102, 10, SSD1306_WHITE);
    display.fillRect(15, 39, w, 8, SSD1306_WHITE);
    display.setCursor(30, 52);
    display.print(F("BOOTING SYSTEM"));
    display.display();
    vibrate(10);
    delay(25);
  }
  
  vibrate(80);
  delay(200);
}

// =========================================================================
// HOME ATTRACT SCREEN ANIMATION
// =========================================================================

float animAngle = 0;
int ballXAnim = 10, ballYAnim = 45, ballDirX = 2, ballDirY = -1;

void updateAndDrawHomeScreen() {
  animAngle += 0.15;
  
  // Bounce decorative game elements
  ballXAnim += ballDirX;
  ballYAnim += ballDirY;
  if (ballXAnim <= 5 || ballXAnim >= 120) ballDirX = -ballDirX;
  if (ballYAnim <= 38 || ballYAnim >= 58) ballDirY = -ballDirY;

  display.clearDisplay();
  
  // Outer Banner
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  
  // Title Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(34, 4);
  display.print(F("RETRO MINI"));
  display.drawFastHLine(4, 14, 120, SSD1306_WHITE);

  // Animated Retro Controller Graphic (Center)
  int padX = 46;
  int padY = 19 + (sin(animAngle) * 2);
  display.drawRoundRect(padX, padY, 36, 18, 4, SSD1306_WHITE);
  // D-Pad
  display.fillRect(padX + 4, padY + 7, 8, 4, SSD1306_WHITE);
  display.fillRect(padX + 6, padY + 5, 4, 8, SSD1306_WHITE);
  // Buttons A/B
  display.fillCircle(padX + 24, padY + 11, 2, SSD1306_WHITE);
  display.fillCircle(padX + 30, padY + 7, 2, SSD1306_WHITE);

  // Background Bouncing Pixel Elements
  display.fillRect(ballXAnim, ballYAnim, 3, 3, SSD1306_WHITE);
  display.drawRect(128 - ballXAnim, 96 - ballYAnim, 4, 4, SSD1306_WHITE);

  // Flashing "PRESS A TO START" Prompt
  if ((millis() / 400) % 2 == 0) {
    display.fillRect(16, 48, 96, 11, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(19, 50);
    display.print(F("PRESS A TO START"));
  }

  display.display();
}

// =========================================================================
// MAIN SETUP & LOOP
// =========================================================================

void setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);

  pinMode(MOTOR, OUTPUT);
  digitalWrite(MOTOR, LOW);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    for (;;);
  }

  runBootAnimation();
}

void loop() {
  checkExitCombination();

  switch (currentState) {
    case STATE_HOME:
      updateAndDrawHomeScreen();
      if (isPressed(btnA)) {
        vibrate(50);
        currentState = STATE_MENU;
      }
      break;

    case STATE_MENU:
      if (isPressed(btnDown)) {
        menuIndex = (menuIndex + 1) % TOTAL_GAMES;
        vibrate(20);
      }
      if (isPressed(btnUp)) {
        menuIndex = (menuIndex - 1 + TOTAL_GAMES) % TOTAL_GAMES;
        vibrate(20);
      }
      if (isPressed(btnA)) {
        vibrate(60);
        score = 0;
        if (menuIndex == 0) { activeGame = STATE_SNAKE;  initSnake();  currentState = STATE_SNAKE; }
        if (menuIndex == 1) { activeGame = STATE_PONG;   initPong();   currentState = STATE_PONG; }
        if (menuIndex == 2) { activeGame = STATE_TETRIS; initTetris(); currentState = STATE_TETRIS; }
        if (menuIndex == 3) { activeGame = STATE_DODGE;  initDodge();  currentState = STATE_DODGE; }
        if (menuIndex == 4) { activeGame = STATE_FLAPPY; initFlappy(); currentState = STATE_FLAPPY; }
      }

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(30, 0);
      display.print(F("= RETRO MINI ="));
      display.drawFastHLine(0, 10, 128, SSD1306_WHITE);

      for (int i = 0; i < 4; i++) {
        int itemIdx = (menuIndex < 4) ? i : (menuIndex - 3 + i);
        if (itemIdx >= TOTAL_GAMES) break;

        if (itemIdx == menuIndex) {
          display.fillRect(5, 14 + (i * 12), 118, 11, SSD1306_WHITE);
          display.setTextColor(SSD1306_BLACK);
        } else {
          display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(10, 16 + (i * 12));
        display.print(gameNames[itemIdx]);
      }
      display.display();
      break;

    case STATE_SNAKE:  updateSnake();  drawSnake();  break;
    case STATE_PONG:   updatePong();   drawPong();   break;
    case STATE_TETRIS: updateTetris(); drawTetris(); break;
    case STATE_DODGE:  updateDodge();  drawDodge();  break;
    case STATE_FLAPPY: updateFlappy(); drawFlappy(); break;

    case STATE_GAMEOVER:
      if (isPressed(btnA) || isPressed(btnUp) || isPressed(btnDown)) {
        vibrate(40);
        currentState = STATE_MENU;
      }
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(12, 5);
      display.println(F("GAME OVER"));
      
      display.setTextSize(1);
      display.setCursor(25, 30);
      display.print(F("Score: ")); display.println(score);
      
      display.setCursor(10, 50);
      display.println(F("Press A to Return"));
      display.display();
      break;
  }
  delay(10);
}

// =========================================================================
// 1. SNAKE GAME ENGINE
// =========================================================================
#define MAX_SNAKE 64
Point snake[MAX_SNAKE];
int snakeLen = 4;
int8_t dirX = 1, dirY = 0;
Point food;
unsigned long lastSnakeMove = 0;

void initSnake() {
  snakeLen = 4; dirX = 1; dirY = 0;
  for (int i = 0; i < snakeLen; i++) snake[i] = { (int8_t)(8 - i), 8 };
  food = { 15, 8 };
}

void updateSnake() {
  if (isPressed(btnUp)    && dirY == 0) { dirX = 0;  dirY = -1; }
  if (isPressed(btnDown)  && dirY == 0) { dirX = 0;  dirY = 1;  }
  if (isPressed(btnLeft)  && dirX == 0) { dirX = -1; dirY = 0;  }
  if (isPressed(btnRight) && dirX == 0) { dirX = 1;  dirY = 0;  }

  if (millis() - lastSnakeMove > 110) {
    lastSnakeMove = millis();
    Point nextHead = { (int8_t)(snake[0].x + dirX), (int8_t)(snake[0].y + dirY) };

    if (nextHead.x < 0 || nextHead.x >= 32 || nextHead.y < 3 || nextHead.y >= 16) {
      triggerGameOver(); return;
    }
    for (int i = 0; i < snakeLen; i++) {
      if (snake[i].x == nextHead.x && snake[i].y == nextHead.y) {
        triggerGameOver(); return;
      }
    }
    for (int i = snakeLen - 1; i > 0; i--) snake[i] = snake[i - 1];
    snake[0] = nextHead;

    if (snake[0].x == food.x && snake[0].y == food.y) {
      vibrate(25); score += 10;
      if (snakeLen < MAX_SNAKE) snakeLen++;
      food = { (int8_t)random(0, 32), (int8_t)random(3, 16) };
    }
  }
}

void drawSnake() {
  display.clearDisplay();
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  display.setCursor(0, 1); display.print(F("SNAKE"));
  display.setCursor(75, 1); display.print(F("SCR:")); display.print(score);

  display.fillRect(food.x * 4, food.y * 4, 4, 4, SSD1306_WHITE);
  for (int i = 0; i < snakeLen; i++) {
    display.drawRect(snake[i].x * 4, snake[i].y * 4, 4, 4, SSD1306_WHITE);
  }
  display.display();
}

// =========================================================================
// 2. PONG GAME ENGINE
// =========================================================================
int paddlePlayerY = 24, paddleCpuY = 24;
float ballX = 64, ballY = 32, ballVx = 2.5, ballVy = 1.5;

void initPong() {
  paddlePlayerY = 24; paddleCpuY = 24;
  ballX = 64; ballY = 32; ballVx = 2.5; ballVy = 1.5;
}

void updatePong() {
  if (digitalRead(BTN_UP) == LOW && paddlePlayerY > 12) paddlePlayerY -= 3;
  if (digitalRead(BTN_DOWN) == LOW && paddlePlayerY < 48) paddlePlayerY += 3;

  if (ballY > paddleCpuY + 8 && paddleCpuY < 48) paddleCpuY += 2;
  if (ballY < paddleCpuY + 8 && paddleCpuY > 12) paddleCpuY -= 2;

  ballX += ballVx; ballY += ballVy;
  if (ballY <= 12 || ballY >= 60) ballVy = -ballVy;

  if (ballX <= 6 && ballY >= paddlePlayerY && ballY <= paddlePlayerY + 16) {
    ballVx = -ballVx * 1.05; vibrate(20); score += 5;
  }
  if (ballX >= 120 && ballY >= paddleCpuY && ballY <= paddleCpuY + 16) {
    ballVx = -ballVx;
  }
  if (ballX < 0) triggerGameOver();
  if (ballX > 128) { ballX = 64; ballY = 32; ballVx = -2.5; }
}

void drawPong() {
  display.clearDisplay();
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  display.setCursor(0, 1); display.print(F("PONG"));
  display.setCursor(75, 1); display.print(F("SCR:")); display.print(score);

  display.fillRect(2, paddlePlayerY, 4, 16, SSD1306_WHITE);
  display.fillRect(122, paddleCpuY, 4, 16, SSD1306_WHITE);
  display.fillCircle((int)ballX, (int)ballY, 2, SSD1306_WHITE);
  display.display();
}

// =========================================================================
// 3. FIXED TETRIS ENGINE (Grid Matrix, Calibrated Boundaries, Compact UI)
// =========================================================================
#define BOARD_W 10
#define BOARD_H 20
uint8_t board[BOARD_H][BOARD_W] = {0};

const Tetromino SHAPES[4] = {
  { {{1,1},{1,1}}, 2 },               // O-Block
  { {{0,1,0},{1,1,1},{0,0,0}}, 3 },   // T-Block
  { {{1,1,0},{0,1,1},{0,0,0}}, 3 },   // Z-Block
  { {{1,0,0},{1,0,0},{1,1,0}}, 3 }    // L-Block
};

Tetromino currentPiece;
int piecePx = 3, piecePy = 0;
unsigned long lastDropTime = 0;

bool checkCollision(int nx, int ny, Tetromino p) {
  for (int r = 0; r < p.size; r++) {
    for (int c = 0; c < p.size; c++) {
      if (p.shape[r][c]) {
        int targetX = nx + c;
        int targetY = ny + r;
        if (targetX < 0 || targetX >= BOARD_W || targetY >= BOARD_H) return true;
        if (targetY >= 0 && board[targetY][targetX]) return true;
      }
    }
  }
  return false;
}

void rotatePiece() {
  Tetromino rotated = currentPiece;
  for (int r = 0; r < currentPiece.size; r++) {
    for (int c = 0; c < currentPiece.size; c++) {
      rotated.shape[c][currentPiece.size - 1 - r] = currentPiece.shape[r][c];
    }
  }
  if (!checkCollision(piecePx, piecePy, rotated)) {
    currentPiece = rotated;
  }
}

void spawnPiece() {
  currentPiece = SHAPES[random(0, 4)];
  piecePx = 3;
  piecePy = 0;
  if (checkCollision(piecePx, piecePy, currentPiece)) {
    triggerGameOver();
  }
}

void lockPiece() {
  for (int r = 0; r < currentPiece.size; r++) {
    for (int c = 0; c < currentPiece.size; c++) {
      if (currentPiece.shape[r][c]) {
        int boardY = piecePy + r;
        int boardX = piecePx + c;
        if (boardY >= 0 && boardY < BOARD_H && boardX >= 0 && boardX < BOARD_W) {
          board[boardY][boardX] = 1;
        }
      }
    }
  }
  vibrate(30);

  for (int r = BOARD_H - 1; r >= 0; r--) {
    bool fullLine = true;
    for (int c = 0; c < BOARD_W; c++) {
      if (!board[r][c]) { fullLine = false; break; }
    }
    if (fullLine) {
      score += 20;
      vibrate(50);
      for (int y = r; y > 0; y--) {
        for (int x = 0; x < BOARD_W; x++) {
          board[y][x] = board[y - 1][x];
        }
      }
      for (int x = 0; x < BOARD_W; x++) board[0][x] = 0;
      r++;
    }
  }
  spawnPiece();
}

void initTetris() {
  memset(board, 0, sizeof(board));
  spawnPiece();
}

void updateTetris() {
  if (isPressed(btnLeft)  && !checkCollision(piecePx - 1, piecePy, currentPiece)) piecePx--;
  if (isPressed(btnRight) && !checkCollision(piecePx + 1, piecePy, currentPiece)) piecePx++;
  if (isPressed(btnA)) rotatePiece();

  int speedDelay = (digitalRead(BTN_DOWN) == LOW) ? 60 : 350;

  if (millis() - lastDropTime > speedDelay) {
    lastDropTime = millis();
    if (!checkCollision(piecePx, piecePy + 1, currentPiece)) {
      piecePy++;
    } else {
      lockPiece();
    }
  }
}

void drawTetris() {
  display.clearDisplay();
  
  // Well Framing (Adjusted 32px width perfectly aligned to 0-63 Y coordinates)
  int offsetX = 2;
  int blockSz = 3;
  display.drawRect(offsetX, 1, (BOARD_W * blockSz) + 3, (BOARD_H * blockSz) + 3, SSD1306_WHITE);

  // Render Grid Blocks
  for (int r = 0; r < BOARD_H; r++) {
    for (int c = 0; c < BOARD_W; c++) {
      if (board[r][c]) {
        display.fillRect(offsetX + 2 + (c * blockSz), 3 + (r * blockSz), blockSz - 1, blockSz - 1, SSD1306_WHITE);
      }
    }
  }

  // Render Active Piece
  for (int r = 0; r < currentPiece.size; r++) {
    for (int c = 0; c < currentPiece.size; c++) {
      if (currentPiece.shape[r][c]) {
        display.fillRect(offsetX + 2 + ((piecePx + c) * blockSz), 3 + ((piecePy + r) * blockSz), blockSz - 1, blockSz - 1, SSD1306_WHITE);
      }
    }
  }

  // Compact Right Side Dashboard
  display.setCursor(42, 4);  display.print(F("TETRIS"));
  display.drawFastHLine(42, 14, 82, SSD1306_WHITE);
  
  display.setCursor(42, 22); display.print(F("SCORE"));
  display.setCursor(42, 32); display.print(score);

  display.setCursor(42, 48); display.print(F("A:Rotate"));
  display.display();
}

// =========================================================================
// 4. SPACE DODGE GAME ENGINE (With Fire Action Support)
// =========================================================================
int playerX = 60;
int astX[3], astY[3];
#define MAX_BULLETS 4
Bullet bullets[MAX_BULLETS];
unsigned long lastDodgeUpdate = 0;

void initDodge() {
  playerX = 60;
  for (int i = 0; i < 3; i++) {
    astX[i] = random(5, 120);
    astY[i] = -random(10, 50);
  }
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
}

void updateDodge() {
  if (digitalRead(BTN_LEFT) == LOW && playerX > 2) playerX -= 3;
  if (digitalRead(BTN_RIGHT) == LOW && playerX < 120) playerX += 3;

  // Fire Action Button (BTN A)
  if (isPressed(btnA)) {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = playerX;
        bullets[i].y = 50;
        bullets[i].active = true;
        vibrate(15);
        break;
      }
    }
  }

  if (millis() - lastDodgeUpdate > 30) {
    lastDodgeUpdate = millis();
    
    // Advance Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (bullets[i].active) {
        bullets[i].y -= 4;
        if (bullets[i].y < 10) bullets[i].active = false;
      }
    }

    // Advance Asteroids & Handle Collision
    for (int i = 0; i < 3; i++) {
      astY[i] += 2;
      
      // Check Laser Hit on Asteroid
      for (int b = 0; b < MAX_BULLETS; b++) {
        if (bullets[b].active && astY[i] > 10) {
          if (abs(bullets[b].x - astX[i]) < 6 && abs(bullets[b].y - astY[i]) < 6) {
            bullets[b].active = false;
            astY[i] = -10;
            astX[i] = random(5, 120);
            score += 15;
            vibrate(30);
          }
        }
      }

      // Check Player Collision
      if (astY[i] >= 50 && astY[i] <= 58 && abs(astX[i] - playerX) < 7) {
        triggerGameOver(); return;
      }
      
      // Reset Screen Bounds
      if (astY[i] > 64) {
        astY[i] = -10;
        astX[i] = random(5, 120);
        score += 5;
      }
    }
  }
}

void drawDodge() {
  display.clearDisplay();
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  display.setCursor(0, 1); display.print(F("SPACE DODGE"));
  display.setCursor(85, 1); display.print(F("S:")); display.print(score);

  // Ship
  display.drawTriangle(playerX, 52, playerX - 4, 60, playerX + 4, 60, SSD1306_WHITE);
  
  // Bullets
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      display.drawFastVLine(bullets[i].x, bullets[i].y, 3, SSD1306_WHITE);
    }
  }

  // Asteroids
  for (int i = 0; i < 3; i++) {
    if (astY[i] > 10) display.fillCircle(astX[i], astY[i], 3, SSD1306_WHITE);
  }
  display.display();
}

// =========================================================================
// 5. FLAPPY BIRD GAME ENGINE
// =========================================================================
float birdY = 32, birdVelocity = 0;
int pipeX = 128, pipeGapY = 25;
const int GAP_HEIGHT = 22;

void initFlappy() {
  birdY = 32; birdVelocity = 0;
  pipeX = 128; pipeGapY = random(15, 35);
}

void updateFlappy() {
  if (isPressed(btnA) || isPressed(btnUp)) {
    birdVelocity = -2.8;
    vibrate(15);
  }
  birdVelocity += 0.25;
  birdY += birdVelocity;

  pipeX -= 2;
  if (pipeX < -12) {
    pipeX = 128;
    pipeGapY = random(15, 35);
    score += 10;
  }

  if (birdY < 12 || birdY > 60) { triggerGameOver(); return; }

  if (pipeX < 24 && pipeX > 12) {
    if (birdY < pipeGapY || birdY > pipeGapY + GAP_HEIGHT) {
      triggerGameOver(); return;
    }
  }
}

void drawFlappy() {
  display.clearDisplay();
  display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
  display.setCursor(0, 1); display.print(F("FLAPPY BIRD"));
  display.setCursor(85, 1); display.print(F("SCR:")); display.print(score);

  display.fillRect(16, (int)birdY, 6, 5, SSD1306_WHITE);
  display.fillRect(pipeX, 10, 10, pipeGapY - 10, SSD1306_WHITE);
  display.fillRect(pipeX, pipeGapY + GAP_HEIGHT, 10, 64 - (pipeGapY + GAP_HEIGHT), SSD1306_WHITE);

  display.display();
}
