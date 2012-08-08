#include <Servo.h>

//FijiBot
//Light Seeking Robot made out of a 1.5 liter Fiji Water Bottle
// by Mike Soniat (msoniat@gmail.com)
// 8/7/2012 Posted to github

//debug
bool showReadings = true;
bool showDirections = true;
bool seekLight = true;
bool avoidStuff = true;

//assign servos
Servo rightServo;  //Digital Pin 7
Servo leftServo;   //Digital Pin 12
Servo pingServo;  //Digital Pin 11
Servo turnServo; //Digital Pin 10

//Ping
const int pingPin = 9;
const long minClearanceInches = 36; 
const long minTurnClearance = 12;
bool clearPath = true;
bool rightClear = true;
bool leftClear = true;
bool tooCloseToTurn = false;
int backUpCount = 0;

//assign analog pins to photoresistors
const int frontSensorPin = 2;
const int rightSensorPin = 3;
const int leftSensorPin = 4;
const int topSensorPin = 5;

//assign digital pins
const int greenLED = 5;
const int redLED = 6;

//rear wheels
const int rotateLeft = 150;
const int rotateRight = 0;
const int fullStop = 90;

//light readings
int readings[4] = {0,0,0,0};
int highestReading = 0;

//looking
const int headRight = 0;
const int headLeft = 160;
const int headForward = 80;

//turning
const int turnRight = 75;
const int turnLeft = 102;
const int goStraight = 90;

void setup(void){
  Serial.begin(9600);
  
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  rightServo.attach(7);  
  rightServo.write(fullStop); // initialize right servo 
  leftServo.attach(12);
  leftServo.write(fullStop); //initialize left servo
  pingServo.attach(11);
  pingServo.write(80); // initialize Ping servo
  turnServo.attach(10);
  turnServo.write(90); // initialize turn servo
  
  //while(1) {}

  //delay on start; blink leds five times
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(greenLED, HIGH); 
    digitalWrite(redLED, HIGH);     
    delay(500); 
    digitalWrite(greenLED, LOW); 
    digitalWrite(redLED, LOW);  
    delay(500);  
  }
 
}

void loop(void){
  Serial.println("Top of Loop");
  
    if (seekLight == true)
    {
        //read photoresistors
        readings[0] = analogRead(frontSensorPin);
        readings[1] = analogRead(rightSensorPin);
        readings[2] = analogRead(leftSensorPin);
        readings[3] = analogRead(topSensorPin);
        highestReading = 0;
        //display readings front, right, left, top and get highest
        for (int i = 0; i < 4; i++)
        {
          if (showReadings == true) {Serial.print(readings[i]);}
          if (readings[i] > readings[highestReading])
          {
            highestReading = i; 
          }
          if (i < 3)
          {
            if (showReadings == true) {Serial.print(", ");}
          }
          else
          {
            if (showReadings == true) {Serial.println("");}
          }
        }
    }
    else
    {
      highestReading = 0; 
    }
  
    if (highestReading != 3)
    {
       //check forward path
      lookForward();
      clearPath = pingBlocked() > minClearanceInches;
      
      if (tooCloseToTurn)
      {
        highestReading = 4; //backup
      }
      else if (!clearPath)
      {
        if (readings[1] > readings[2] && lookRight() > minTurnClearance)
        {
            highestReading = 1; //turn right
        }
        else if (lookLeft() > minTurnClearance)
          {
             highestReading = 2; //turn left
          } 
        else
        {
          highestReading = 4; //backup
        }
        lookForward();
      }
    }
  //move toward light or unobstructed path
  switch ( highestReading ) 
  {

  case 0 : //front
    if (showDirections == true) {Serial.println("Go Forward");}
    digitalWrite(greenLED, HIGH); 
    digitalWrite(redLED, HIGH);  
    setMotors(rotateLeft, rotateRight, goStraight);
    break;

  case 1 : //right
    if (lookRight() > minTurnClearance)
    {
      if (showDirections == true) {Serial.println("Turn Right");}
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, LOW);
      backUpCount = 0;
      lookForward();
      setMotors(rotateLeft, rotateRight, turnRight);
      delay(1000);
    }
    lookForward();
    break;

  case 2 : //left
    if (lookLeft() > minTurnClearance)
    {
      if (showDirections == true) {Serial.println("Turn Left");}
      digitalWrite(greenLED, LOW); 
      digitalWrite(redLED, HIGH);
      backUpCount = 0;
      lookForward();
      setMotors(rotateLeft, rotateRight, turnLeft);  
      delay(1000);
    }
    lookForward();
    break;
  
  case 3 : //top
    if (showDirections == true) {Serial.println("Stop");} //highest light level above; stop and bask
    digitalWrite(greenLED, HIGH); 
    digitalWrite(redLED, HIGH);  
    setMotors(fullStop, fullStop, goStraight);  
    break;
    
 case 4 : //backup
    backUpCount++;
    if (backUpCount < 2)
    {
      digitalWrite(greenLED, LOW); 
      digitalWrite(redLED, LOW);  
      if (showDirections == true) {Serial.println("Backup");}
      setMotors(rotateRight, rotateLeft, goStraight);
      delay(1000);  
    }
    else
    {
      //if already backed up twice, try short right turn
      backUpCount = 0;
      setMotors(rotateLeft, rotateRight, turnRight);
      delay(500);      
    }
    break;
  }
}
    
void setMotors(int left, int right, int turnPos)
{
    if (showDirections == true) {Serial.println("Clear Path");}
    turnServo.write(turnPos);
    leftServo.write(left);
    rightServo.write(right);  
    delay(20);
}

long pingBlocked()
{
  long duration, distance;
  //initialize Ping with LOW then HIGH
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  //read Ping
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  //convert microseconds to distance in inches
  distance = microsecondsToInches(duration);
  
  if (showReadings == true) {
    Serial.print(duration);
    Serial.print(", ");
    Serial.println(distance);
  }
  //fix for no reading (distance = 0)
  if (distance == 0)
  {
     distance = minClearanceInches; 
  }
  
  //check for turn clearance
  tooCloseToTurn = distance <  minTurnClearance;
  //compare to minClearanceInches
  return distance;
}

long microsecondsToInches(long microseconds)
{
  // The speed of sound is 13512 inches per second, which is 74 microseconds/in
  // Sound travels out and back to the Ping, so find half the distance recorded
  return microseconds / 74 / 2;
  
}

long lookRight()
{
  //turn Ping to right
  if (showDirections == true) {Serial.println("Look Right");}
  pingServo.write(headRight);
  delay(500);
  return pingBlocked();
}

long lookLeft()
{
  //turn Ping to left
  if (showDirections == true) {Serial.println("Look Left");}
  pingServo.write(headLeft);
  delay(500);
  return pingBlocked();
}

void lookForward()
{
  //turn Ping to front
  if (showDirections == true) {Serial.println("Look Forward");}
  pingServo.write(headForward);
}
