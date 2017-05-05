#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>

#define VERSION     "0.0.1"

SoftwareSerial btSerial(10, 11); // RX, TX

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,16,2);  

// for the Arduino Uno + CNC shield V3
#define MOTOR_A_ENABLE_PIN 8
#define MOTOR_A_STEP_PIN 2
#define MOTOR_A_DIR_PIN 5

// not used 
#define MOTOR_B_ENABLE_PIN 8
#define MOTOR_B_STEP_PIN 3
#define MOTOR_B_DIR_PIN 6

#define RELAY_PIN 9

AccelStepper motorA(1, MOTOR_A_STEP_PIN, MOTOR_A_DIR_PIN); 
boolean enabledA = false;
//AccelStepper motorB(1, MOTOR_B_STEP_PIN, MOTOR_B_DIR_PIN); 

#define STEPS_PER_MM 500 //400pasi/0.8mm
int steps_dir = -1;


// serial data
char inData[81]; // Allocate some space for the string
bool newInput;
int index = 0; // Index into array; where to store the character

const char* delimiters = " "; // whatever characters delimit your input string
char* tokens [4];
enum indexName {motor, command, value};


void setup(){
  
  Serial.begin(9600);
  Serial.write(VERSION);


  // set the data rate for the SoftwareSerial port
  btSerial.begin(4800);
  btSerial.println("Hello, world?");

  // relay 
  pinMode(RELAY_PIN, OUTPUT); 
  digitalWrite(RELAY_PIN, LOW);

  // motor 1 initial setup
  pinMode(MOTOR_A_ENABLE_PIN, OUTPUT); 
  motorA.setEnablePin(MOTOR_A_ENABLE_PIN);
  motorA.setPinsInverted(false, false, true);
  //
  motorA.setAcceleration(1000);  
  motorA.setMaxSpeed(3000);
  motorA.setSpeed(3000);

  // initialize the lcd 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  //lcd.print("");
  //lcd.clear();
}

void loop() {

  if (btSerial.available()) {
    Serial.write(btSerial.read());
  }

  while(Serial.available() > 0) {
    char readChar = Serial.read ();
    if(readChar == ';'){
      newInput = true; 
    } else {
      if(index < 80 ){
        inData[index++] = readChar;
        inData[index] = '\0';
      }
    }
 }
 if(newInput && strlen(inData)) {
  int tokenIndex = 0;
  tokens [tokenIndex] = strtok (inData, delimiters);
  while ((tokenIndex < 4 - 1) && tokens [tokenIndex])
  {
    tokenIndex++;
    tokens [tokenIndex] = strtok (NULL, delimiters);
  }

   if(strcmp(tokens[0],"relay") == 0){
      if(strcmp(tokens[1],"on")==0){
        digitalWrite(RELAY_PIN, HIGH);
      } else {
        digitalWrite(RELAY_PIN, LOW);
      }
   }



   if(strcmp(tokens[0],"m1") == 0){
     // motor A
      if(strcmp(tokens[1],"on")==0){ // enable motor 1
       motorA.enableOutputs();
       enabledA = true;
       lcd.clear();
       lcd.print("M1 on");
     } else if(strcmp(tokens[1],"stop")==0){ // disable motor 1
        motorA.stop(); // Stop as fast as possible: sets new target
        //motorA.runToPosition();   // Now stopped after quickstop
        lcd.clear();
        lcd.print("M1 off");
     } else if(strcmp(tokens[1],"off")==0){ // disable motor 1
         motorA.disableOutputs();
         enabledA = false;
        lcd.clear();
        lcd.print("M1 off");
     } else if(strcmp(tokens[1],"cp")==0){ // get current position
       lcd.clear();
       lcd.print("M1 pos: ");
       lcd.print(motorA.currentPosition());
     } else if(strcmp(tokens[1],"steps")==0){ // move in steps
       motorA.move(atoi(tokens[2]));
       lcd.clear();
       lcd.print("M1 steps: ");
       lcd.print(atoi(tokens[2]));
     } else if(strcmp(tokens[1],"speed")==0){ // set speed
       motorA.setSpeed(atoi(tokens[2]));
       lcd.clear();
       lcd.print("M1 speed: ");
       lcd.print(atoi(tokens[2]));
     } else if(strcmp(tokens[1],"mm")==0){ // set speed
       motorA.move((atoi(tokens[2])*STEPS_PER_MM)*steps_dir);
       lcd.clear();
       lcd.print("M1 mm: ");
       lcd.print(atoi(tokens[2]));
     } else if(strcmp(tokens[1],"dir")==0){ // set direction
       lcd.clear();
        if(strcmp(tokens[2],"back") ==0){
           steps_dir = 1;
           lcd.print("M1 direction: back");
        } else {
           steps_dir = -1;
           lcd.print("M1 direction: fw");
        }
     }




   }  
   
   newInput = false;
   index = 0;
   inData[0] = '\0';
 }

 if(enabledA){
  motorA.run();
 }


 
}


