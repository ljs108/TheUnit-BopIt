#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <MPU6050_light.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>
#include <math.h>
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
int numberCorrectMoves = 0;
double timeForEachMove = 5;

void setup() {
  //call function to set up all the components
  setUpComponents();

  //introduction to the game
  introduction();
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
  
}

void setUpComponents() {
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
  audioPlayer.speakerPin = 9;
  while(!SD.begin(SD_ChipSelectPin)){
  }

  audioPlayer.setVolume(5);
  audioPlayer.loop(1);
  audioPlayer.play("test.wav");
  
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

  boolean isCorrect = true;

  while (isCorrect) {
    //start random game levels
    //randomNumber generator from 1-3 for number of action components
    int action = 1 + (rand()%3);

    lcd.clear();
    
    //Based off number call the action for the corresponding component
    if (action == 1) {
      isCorrect = executeJoystick();
    }
    else if(action == 2) {
      isCorrect = executeButtons();
    }
    else if(action == 3) {
      isCorrect = executeGyro();
    }

    //check number of correct moves
    if(numberCorrectMoves == 99) {
      youWin();
    }
  }
  
  endGame();
}

void endGame() {
  //Display losing message on LCD
  lcd.clear();
  lcd.setCursor(5,0);   //Set cursor to character 2 on line 0
  lcd.print("Incorrect!");
  lcd.setCursor(6,1);
  lcd.print("You lose!");
  lcd.setCursor(2,2);
  lcd.print("Please try again.");

  delay(5000);
  setup();
  
}

void youWin() {
  //Display winning message on LCD
  lcd.clear();
  lcd.setCursor(6,1);  
  lcd.print("You WIN!");
  lcd.setCursor(1,2);  
  lcd.print("*Congratulations*");
}
    
boolean executeJoystick() {
  //initialize movePin
  int incorrectOtherMax = 1023;
  int incorrectOtherMin = 0;
  String moveDirectionString = "";
  
  int xValue = 524;
  int yValue = 524;
  int correctDirection = 1023;
  int incorrectDirection = 0;
  //decide what move to do by random generation
  int moveDirection = 1 + (rand()%4);

  if (moveDirection == 1) {
    moveDirectionString = "UP";
  }
  else if (moveDirection == 2) {
    moveDirectionString = "DOWN";
    correctDirection = 0;
    incorrectDirection = 1023;
  }
  else if (moveDirection == 3) {
    moveDirectionString = "LEFT";
    correctDirection = 0;
    incorrectDirection = 1023;
  }
  else if (moveDirection == 4) {
    moveDirectionString = "RIGHT";
  }
  
  //announce joystick command
  lcd.setCursor(round((20 - moveDirectionString.length())/2),1);
  lcd.print("Joystick: " + moveDirectionString);
  lcd.setCursor(0,3);
  lcd.print("Count: " + (String)numberCorrectMoves);

  // start measuring time
  Chrono timer;
  timer.restart();

  boolean correct = false;
  //loop while elasped time is less than time for each move
  while(timer.elapsed()/1000 <= timeForEachMove && ( xValue <= 1022 && xValue >= 1 && yValue <= 1022 && yValue >= 1)) {
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
    delay(2000);
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
 
  //decide what move to do by random generation
  int pressButton = 1 + (rand()%2);

  if (pressButton == 1) {
    pressButtonString = "YELLOW";
    //power yellow and blue button
    digitalWrite(7, HIGH);
    digitalWrite(3, HIGH);
    movePin = 8;
    otherPin = 4;
  }
  else if (pressButton == 2) {
    pressButtonString = "BLUE";
    //power yellow and blue button
    digitalWrite(7, HIGH);
    digitalWrite(3, HIGH);
    movePin = 4;
    otherPin = 8;
  }
  
  //announce button command
  lcd.setCursor(round((20 - pressButtonString.length())/2),1);
  lcd.print("Press: " + pressButtonString);
  lcd.setCursor(0,3);
  lcd.print("Correct: " + (String)numberCorrectMoves);

  // start measuring time
  Chrono timer;
  timer.restart();

  boolean answeredInTime = false;
  
  //loop while elasped time is less than time for each move
  while(timer.elapsed()/1000 <= timeForEachMove) {
    if (digitalRead(movePin) == HIGH) {
      answeredInTime = true;
      //announce correct
      lcd.setCursor(0,1);   //Set cursor to character 0 on line 0
      lcd.print("    Correct Move!   ");
      delay(2000);
    }
    if (digitalRead(otherPin) == HIGH) {
      //RESET buttons to low
      digitalWrite(7, LOW);
      digitalWrite(3, LOW);
      return false;
    }
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

  //RESET buttons to low
  digitalWrite(7, LOW);
  digitalWrite(3, LOW);
  return true;
}

boolean executeGyro() {
  
  //initialize movePin
  String tiltDirectionString = "";

  int threshold = 0;

  boolean correct = false;
  int x = 0;
  int y = 0;
  
  //decide what move to do by random generation
  int moveDirection = 1 + (rand()%4);

  if (moveDirection == 1) {
    tiltDirectionString = "TILT UP";
    threshold = -150;
  }
  else if (moveDirection == 2) {
    tiltDirectionString = "TILT DOWN";
    threshold = 150;
  }
  else if (moveDirection == 3) {
    tiltDirectionString = "TILT LEFT";
    threshold = -150;
  }
  else if (moveDirection == 4) {
    tiltDirectionString = "TILT RIGHT";
    threshold = 150;
  }
  
  //announce gyro command
  lcd.setCursor(round((20 - tiltDirectionString.length())/2),1);
  lcd.print("Gyro: " + tiltDirectionString);
  lcd.setCursor(0,3);
  lcd.print("Correct: " + (String)numberCorrectMoves);

  // start measuring time
  Chrono timer;
  timer.restart();

  while(timer.elapsed()/1000 <= timeForEachMove && (x <= 150 && x >= -150 && y <= 150 && y >= -150)) {
    mpu.update();
    x = mpu.getGyroX();
    y = mpu.getGyroY();
    if (moveDirection == 1) {
      if(x <= threshold && x <= 150 && y <= 150 && y >= -150) {
        correct = true;
      }
    } else if (moveDirection == 2) {
      if(x >= threshold && x >= -150 && y <= 150 && y >= -150) {
        correct = true;
      }
    } else if (moveDirection == 3) {
      if(y <= threshold && y <= 150 && x <= 150 && x >= -150) {
        correct = true;
      }
    } else if (moveDirection == 4) {
      if(y >= threshold && y >= -150 && x <= 150 && x >= -150) {
        correct = true;
      } 
    }
  }

  if (correct == true) {
    //announce correct
    lcd.setCursor(0,1);   //Set cursor to character 0 on line 0
    lcd.print("    Correct Move!   ");
    delay(2000);
  }
  else if (correct == false) {
    return false;
  }
  setNumberCorrectMoves(numberCorrectMoves + 1);
  setTimeForEachMove(timeForEachMove - 0.05);
  return true;
}
