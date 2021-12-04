#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <MPU6050_light.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>  
#include <Chrono.h>
#include <LightChrono.h>
#define SD_ChipSelectPin 10
#include <TMRpcm.h>
using namespace std;

//global variables and inits
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display
MPU6050 mpu(Wire);
TMRpcm audioPlayer;
TMRpcm audioDirection;
int numberCorrectMoves = 0;
double timeForEachMove = 5;
boolean inGame = false;
boolean alreadyInit = false;

void setup() { 
  srand(time(NULL));
  //call function to set up all the components
  setUpComponents();
}

//called when power is turned on
void loop() {
      
    //read yellow button input to see if high
    if (digitalRead(8) == HIGH) {
      //power yellow button down
      digitalWrite(7, LOW);
      delay(2000);
      game();
    }
}

void introduction() {
  // print welcome message on the LCD screen
  lcd.clear();
  lcd.setCursor(2,1);   //Set cursor to character 2 on line 1
  lcd.print("Welcome to BOPIT!");
  lcd.setCursor(4,2);   //Set cursor to character 2 on line 1
  lcd.print("By: The Unit");

  delay(5000);
  
  //power yellow button high
  digitalWrite(7, HIGH);

  //clear screen and display first instruction
  lcd.clear();
  lcd.setCursor(2,1);   //Move cursor to character 0 on line 1
  lcd.print("Press the Yellow");
  lcd.setCursor(3,2);
  lcd.print("Button to play!");

  audioDirection.loop(0);
  audioDirection.play("PY.wav");
  delay(1200);
  audioPlayer.loop(1);
  audioPlayer.play("cash.wav",15);
  
}

void setUpComponents() {
  if (alreadyInit == false) {
  //initalize lcd
  lcd.clear();  
  lcd.init();       
  lcd.backlight();      // Make sure backlight is on

  //pins for yellow button
  pinMode(7, OUTPUT); 
  pinMode(8, INPUT);    

  //pins for blue button
  pinMode(3, OUTPUT); 
  pinMode(4, INPUT);

  //pins for Micro SD card
  pinMode(12, OUTPUT); 
  pinMode(11, INPUT);

  //initalize Gyro
  Wire.begin();
  byte status = mpu.begin();
  while(status!=0){ }
  mpu.upsideDownMounting = false;
  mpu.calcOffsets(true,true);

  //initialize audio
    alreadyInit = true;
    audioPlayer.speakerPin = 9;
    audioDirection.speakerPin = 9;

    while(!SD.begin(SD_ChipSelectPin)){
      lcd.setCursor(0,2);
      lcd.print("Plug in SD card!");
    }

    audioDirection.setVolume(7);
    audioDirection.loop(0);
    audioPlayer.setVolume(5);
    audioPlayer.loop(1);
    audioPlayer.play("cash.wav"); 
  }

  //introduction to the game
  introduction();
}

void setNumberCorrectMoves(int input) {
  numberCorrectMoves = input;
}

void setTimeForEachMove(double input) {
  timeForEachMove = input;
}
  
void game() {
  //Initialize game
  setNumberCorrectMoves(0);
  setTimeForEachMove(5);

  boolean isCorrect = true;

  while (isCorrect) {
    //start random game levels
    //randomNumber generator from 1-3 for number of action components
    int action = 1 + (rand()%3);

    lcd.clear();

    //check number of correct moves
    if(numberCorrectMoves == 100) {
      isCorrect = false;
    }
    
    
    //Based off number call the action for the corresponding component
    else if (action == 1) {
      isCorrect = executeJoystick();
    }
    else if(action == 2) {
      isCorrect = executeButtons();
    }
    else if(action == 3) {
      isCorrect = executeGyro();
    }
  }

  //check number of correct moves
  if(numberCorrectMoves == 10) {
    youWin();
  }
  else {
    endGame();  
  }
}

void endGame() {
  //Display losing message on LCD
  lcd.clear();
  lcd.setCursor(6,0);
  lcd.print("You lose!");
  lcd.setCursor(2,1);
  lcd.print("Please try again.");
  lcd.setCursor(0,3);
  lcd.print("Final Score: " + (String)numberCorrectMoves);

  audioDirection.loop(0);
  audioDirection.play("L.wav");
  delay(4000);
  audioPlayer.play("cash.wav",15);

  delay(5000);
  setNumberCorrectMoves(0);
  setTimeForEachMove(5);
  setUpComponents();
}

void youWin() {
  //Display winning message on LCD
  lcd.clear();
  lcd.setCursor(6,0);  
  lcd.print("You WIN!");
  lcd.setCursor(1,1);  
  lcd.print("*Congratulations*");
  lcd.setCursor(0,3);
  lcd.print("Final Score: " + (String)numberCorrectMoves);

  audioDirection.loop(0);
  audioDirection.play("W.wav");
  delay(4000);
  audioPlayer.play("cash.wav",15);

  delay(10000);
  setNumberCorrectMoves(0);
  setTimeForEachMove(5);
  setUpComponents();
}
    
boolean executeJoystick() {
  //initialize movePin
  int incorrectOtherMax = 1023;
  int incorrectOtherMin = 0;
  String moveDirectionString = "";
  char instruction[] = "  .wav";
  
  int xValue = 524;
  int yValue = 524;
  int correctDirection = 1023;
  int incorrectDirection = 0;
  //decide what move to do by random generation
  int moveDirection = 1 + (rand()%4);

  if (moveDirection == 1) {
    moveDirectionString = "UP";
    instruction[0] = 'J';
    instruction[1] = 'U';
  }
  else if (moveDirection == 2) {
    moveDirectionString = "DOWN";
    correctDirection = 0;
    incorrectDirection = 1023;
    instruction[0] = 'J';
    instruction[1] = 'D';
  }
  else if (moveDirection == 3) {
    moveDirectionString = "LEFT";
    correctDirection = 0;
    incorrectDirection = 1023;
    instruction[0] = 'J';
    instruction[1] = 'L';
  }
  else if (moveDirection == 4) {
    moveDirectionString = "RIGHT";
    instruction[0] = 'J';
    instruction[1] = 'R';
  }
  
  //announce joystick command
  String message = "Joystick: " + moveDirectionString;
  lcd.setCursor(round((20 - message.length())/2),1);
  lcd.print("Joystick: " + moveDirectionString);
  lcd.setCursor(0,3);
  lcd.print("Score: " + (String)numberCorrectMoves);

  audioDirection.loop(0);
  audioDirection.play(instruction);
  delay(1500);
  audioPlayer.play("cash.wav",15);

  // start measuring time
  Chrono timer;
  timer.restart();

  mpu.update();
  double x = mpu.getGyroX();
  double y = mpu.getGyroY();
  double newx = mpu.getGyroX();
  double newy = mpu.getGyroY();

  boolean correct = false;
  //loop while elasped time is less than time for each move
  while(timer.elapsed()/1000 <= timeForEachMove && ( xValue <= 1022 && xValue >= 1 && yValue <= 1022 && yValue >= 1)) {

    //gyro read
    mpu.update();
    newx = mpu.getGyroX();
    newy = mpu.getGyroY();
  
    if (abs(newx) - abs(x) > 5 && abs(newy) - abs(y) > 5) {
      return false;
    }
  
    //button read
    if (digitalRead(4) == HIGH || digitalRead(8) == HIGH) {
      return false;
    }
    
    // Read analog port values A0 and A1  
    xValue = analogRead(A0);  
    yValue = analogRead(A1);
    if (moveDirection == 1) {
      if (yValue == correctDirection && yValue != incorrectDirection && xValue != incorrectOtherMax && xValue != incorrectOtherMin) {
        correct = true;
      } else {
        correct = false;
      }
    } else if (moveDirection == 2) {
      if (yValue == correctDirection && yValue != incorrectDirection && xValue != incorrectOtherMax && xValue != incorrectOtherMin) {
        correct = true;
      } else {
        correct = false;
      }
    } else if (moveDirection == 3) {
      if (xValue == correctDirection && xValue != incorrectDirection && yValue != incorrectOtherMax && yValue != incorrectOtherMin) {
        correct = true;
      } else {
        correct = false;
      }
    } else if (moveDirection == 4) {
      if (xValue == correctDirection && xValue != incorrectDirection && yValue != incorrectOtherMax && yValue != incorrectOtherMin) {
        correct = true;
      } else {
        correct = false;
      }
    }
  }

  if (correct == true) {
    //announce correct
    lcd.setCursor(0,1);   //Set cursor to character 0 on line 0
    lcd.print("    Correct Move!   ");
    delay(1000);
  }
  else if (correct == false) {
    return false;
  }
  setNumberCorrectMoves(numberCorrectMoves + 1);
  setTimeForEachMove(timeForEachMove - 0.05);
  return true;
}

boolean executeButtons() {
  //initialize movePin
  int pressPin = 0;
  int movePin = 0;
  int otherPin;
  String pressButtonString = "YELLOW";
  char instruction[] = "  .wav";
 
  //decide what move to do by random generation
  int pressButton = 1 + (rand()%2);

  if (pressButton == 1) {
    pressButtonString = "YELLOW";
    //power yellow and blue button
    digitalWrite(7, HIGH);
    digitalWrite(3, HIGH);
    movePin = 8;
    otherPin = 4;
    instruction[0] = 'P';
    instruction[1] = 'Y';
  }
  else if (pressButton == 2) {
    pressButtonString = "BLUE";
    //power yellow and blue button
    digitalWrite(7, HIGH);
    digitalWrite(3, HIGH);
    movePin = 4;
    otherPin = 8;
    instruction[0] = 'P';
    instruction[1] = 'B';
  }
  
  //announce button command
  String message = "Press: " + pressButtonString;
  lcd.setCursor(round((20 - message.length())/2),1);
  lcd.print("Press: " + pressButtonString);
  lcd.setCursor(0,3);
  lcd.print("Score: " + (String)numberCorrectMoves);

  audioDirection.loop(0);
  audioDirection.play(instruction);
  delay(1200);
  audioPlayer.play("cash.wav",15);

  // start measuring time
  Chrono timer;
  timer.restart();

  //joystick read
  int xValue = analogRead(A0);  
  int yValue = analogRead(A1);
  int newxValue = analogRead(A0);  
  int newyValue = analogRead(A1);

  //gyro read
  mpu.update();
  double x = mpu.getGyroX();
  double y = mpu.getGyroY();
  double newx = mpu.getGyroX();
  double newy = mpu.getGyroY();

  boolean answeredInTime = false;

  boolean movePinPressed = false;
  boolean otherPinPressed = false;
  
  //loop while elasped time is less than time for each move
  while(timer.elapsed()/1000 <= timeForEachMove && (movePinPressed == false && otherPinPressed == false)) {

    //joystick read
    newxValue = analogRead(A0);  
    newyValue = analogRead(A1);
    if (abs(newxValue) - abs(xValue) > 10 && abs(newyValue) - abs(yValue) > 10) {
      return false;
    }
  
    //gyro read
    mpu.update();
    newx = mpu.getGyroX();
    newy = mpu.getGyroY();
    if (abs(newx) - abs(x) > 5 && abs(newy) - abs(y) > 5) {
      return false;
    }

    if (digitalRead(movePin) == HIGH) {
      movePinPressed = true;
    }

    if (digitalRead(otherPin) == HIGH) {
      otherPinPressed = true;
    }
  }

  if (movePinPressed == true) {
    answeredInTime = true;
    //announce correct
    lcd.setCursor(0,1);   //Set cursor to character 0 on line 0
    lcd.print("    Correct Move!   ");
    delay(1000);
    //RESET buttons to low
    digitalWrite(7, LOW);
    digitalWrite(3, LOW);
  }
  
  if (otherPinPressed == true) {
    //RESET buttons to low
    digitalWrite(7, LOW);
    digitalWrite(3, LOW);
    return false;
  }
  
  //if you did not answer in time return false
  if (!answeredInTime) {
    //RESET buttons to low
    digitalWrite(7, LOW);
    digitalWrite(3, LOW);
    return false;
  }
  setNumberCorrectMoves(numberCorrectMoves + 1);
  setTimeForEachMove(timeForEachMove - 0.05);
  return true;
}

boolean executeGyro() {
  
  //initialize movePin
  String tiltDirectionString = "";
  char instruction[] = "  .wav";

  int threshold = 0;

  boolean correct = false;
  int x = 0;
  int y = 0;
  
  //decide what move to do by random generation
  int moveDirection = 1 + (rand()%4);

  if (moveDirection == 1) {
    tiltDirectionString = "UP";
    threshold = -50;

    instruction[0] = 'T';
    instruction[1] = 'U';
  }
  else if (moveDirection == 2) {
    tiltDirectionString = "DOWN";
    threshold = 50;
    
    instruction[0] = 'T';
    instruction[1] = 'D';
  }
  else if (moveDirection == 3) {
    tiltDirectionString = "LEFT";
    threshold = -50;
    
    instruction[0] = 'T';
    instruction[1] = 'L';
  }
  else if (moveDirection == 4) {
    tiltDirectionString = "RIGHT";
    threshold = 50;

    instruction[0] = 'T';
    instruction[1] = 'R';
  }
  
  //announce gyro command
  String message = "Tilt: " + tiltDirectionString;
  lcd.setCursor(round((20 - message.length())/2),1);
  lcd.print("Tilt: " + tiltDirectionString);
  lcd.setCursor(0,3);
  lcd.print("Score: " + (String)numberCorrectMoves);

  audioDirection.loop(0);
  audioDirection.play(instruction);
  delay(1000);
  audioPlayer.play("cash.wav",15);

  // start measuring time
  Chrono timer;
  timer.restart();

  //joystick read
  int xValue = analogRead(A0);  
  int yValue = analogRead(A1);
  int newxValue = analogRead(A0);  
  int newyValue = analogRead(A1);

  while(timer.elapsed()/1000 <= timeForEachMove && (x <= 50 && x >= -50 && y <= 50 && y >= -50)) {

    //joystick read
    newxValue = analogRead(A0);  
    newyValue = analogRead(A1);
    if (abs(newxValue) - abs(xValue) > 10 && abs(newyValue) - abs(yValue) > 10) {
      return false;
    }
  
    //button read
    if (digitalRead(4) == HIGH || digitalRead(8) == HIGH) {
      return false;
    }
    
    mpu.update();
    x = mpu.getGyroX();
    y = mpu.getGyroY();
    if (moveDirection == 1) {
      if(x <= threshold && x <= 50 && y <= 50 && y >= -50) {
        correct = true;
      }
    } else if (moveDirection == 2) {
      if(x >= threshold && x >= -50 && y <= 50 && y >= -50) {
        correct = true;
      }
    } else if (moveDirection == 3) {
      if(y <= threshold && y <= 50 && x <= 50 && x >= -50) {
        correct = true;
      }
    } else if (moveDirection == 4) {
      if(y >= threshold && y >= -50 && x <= 50 && x >= -50) {
        correct = true;
      } 
    }
  }

  if (correct == true) {
    //announce correct
    lcd.setCursor(0,1);   //Set cursor to character 0 on line 0
    lcd.print("    Correct Move!   ");
    delay(1000);
  }
  else if (correct == false) {
    return false;
  }
  setNumberCorrectMoves(numberCorrectMoves + 1);
  setTimeForEachMove(timeForEachMove - 0.05);
  return true;
}
