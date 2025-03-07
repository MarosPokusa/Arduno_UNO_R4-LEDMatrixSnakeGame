#include "Arduino_LED_Matrix.h"
ArduinoLEDMatrix matrix;

#define MAX_Y 8
#define MAX_X 12
#define stickX A0
#define stickY A1

short turnDelay = 300;
unsigned long lastMillis = 0;
unsigned long currentMillis;
uint8_t grid[MAX_Y][MAX_X] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

class Input{
  public:
    short x;
    short y;
    //gets joystick input
    void GetInput(){
      if(analogRead(stickX) < 250) x = -1;
        else if (analogRead(stickX) > 750) x = 1;
        else x = 0;
      if(analogRead(stickY) < 250) y = -1;
        else if (analogRead(stickY) > 750) y = 1;
        else y = 0;
    }
};

class Apple{
  public:
    short x = 3;
    short y = 3;
    unsigned long startMillis;
    unsigned long endMillis;
    unsigned long timeToGenerate;
    //generates an apple on a random position until it finds a spot that is free
    //random takes too long so i messure the time to find a spot and later i subtract it from the turn delay
    void GenerateApple(){
      startMillis=millis();
      do {
        x = random(0, MAX_X);
        y = random(0, MAX_Y);       
      } while(grid[y][x]!=0);
      endMillis = millis();
      timeToGenerate = endMillis - startMillis;
      Serial.println(timeToGenerate);
    }
};

class Snake{
  public:
    short snake[MAX_X*MAX_Y][2]={
      {7,3},
      {8,3},
      {9,3}
    };
    short length = 3;
    short lastX = -1, lastY = 0;
    //moves the snake in the requested direction
    void MoveSnake(short moveX, short moveY){
      for(int i = length; i > 0; i--){
        snake[i][0] = snake[i-1][0];
        snake[i][1] = snake[i-1][1];
      }
      //the second part of the statement makes sure you cant kill yourself by hitting the second square of the snake
      if (moveX != 0 && snake[0][0] + moveX != snake[2][0]){
        snake[0][0] += moveX;
        lastX = moveX;
        lastY = 0;
      } else if (moveY != 0 && snake[0][1] + moveY != snake[2][1]){
        snake[0][1] += moveY;
        lastY = moveY;
        lastX = 0;
      } else {
        snake[0][0] += lastX;
        snake[0][1] += lastY;
      }
      //teleports the snake if you go to the edge
      if (snake[0][0]<0) snake[0][0] = MAX_X - 1;
      else if (snake[0][0]>MAX_X-1) snake[0][0] = 0;
      else if (snake[0][1]<0) snake[0][1] = MAX_Y - 1;
      else if (snake[0][1]>MAX_Y-1) snake[0][1] = 0;
    }
};

Apple apple;
Input input;
Snake snake;

void setup() {
  //arduino r4 has a pin that can output an analog signal, so i defined that the used pins are input
  pinMode(stickX, INPUT);
  pinMode(stickY, INPUT);
  Serial.begin(19200);
  matrix.begin();
  delay(1000);
  play();
}

void loop() {
  currentMillis = millis();
  //turnDelay - apple.timeToGenerate is very likely to owerflow so i put in a failsafe
  if (currentMillis-lastMillis > turnDelay - apple.timeToGenerate || apple.timeToGenerate > turnDelay){
    play();
    lastMillis = currentMillis;
    apple.timeToGenerate = 0;
  }
}

void play(){
  boolean newGrid[MAX_Y][MAX_X] = {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };
  //puts the apple on the new grid
  newGrid[apple.y][apple.x] = 1;
  
  input.GetInput();
  snake.MoveSnake(input.x, input.y);

  //checks if the snake colided with a lit up square
  if (grid[snake.snake[0][1]][snake.snake[0][0]]==1){   
    //checks if the square was an apple if yes it adds to the snake length
    //if it wasnt an apple it means it was a snake so it resets the game  
    if ((snake.snake[0][0]==apple.x && snake.snake[0][1]==apple.y)){
      snake.length++;
      apple.GenerateApple();
      if (turnDelay > 100) turnDelay-=10;
    } else reset();
  }
  //prints the snake
  for (int i = 0; i < snake.length; i++){
    newGrid[snake.snake[i][1]][snake.snake[i][0]] = 1;
  }

  //changes the old grid to the new one that was created in this function
  for(int y = 0; y < MAX_Y; y++){
    for(int x = 0; x < MAX_X; x++){
      grid[y][x] = newGrid[y][x];
    }
  }

  display();
}

void reset(){
  //prints the length as a score
  Serial.print("You died! Score: ");
  Serial.println(snake.length);

  //resets applle to the default coordinates
  apple.x = 3;
  apple.y = 3;

  //resets the snake to its default values
  snake.length = 3;
  snake.lastX = -1;
  snake.lastY = 0;
  for (int i = 0; i<3; i++){
    snake.snake[i][0] = i + 7;
    snake.snake[i][1] = 3;
  }

  //resets the speed
  turnDelay = 300;

  delay(1000);
}

void display(){
  matrix.renderBitmap(grid, MAX_Y, MAX_X);
}



