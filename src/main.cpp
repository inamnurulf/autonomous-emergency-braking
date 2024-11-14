#include <Adafruit_VL53L1X.h>
#include <ArduinoJson.h>

#include "websocket.h"
#include "motor.h"
#include "fuzzyBrakeController.h"

const char *ssid = "aegis";
const char *password = "okemantapbos";

/* websocket */
const char *ip = "10.42.0.1";
const uint16_t port = 3000;
WebSocket webSocket(ssid, password, ip, port);

/* lidar */
Adafruit_VL53L1X vl53;
unsigned long lastTime = 0;
int16_t lastDistance = 0;
float relativeSpeed = 0.0;
int16_t currentDistance = 0;

const int brakeDistance = 400;
const int emergencyDistance = 100;

float speed = 0;
float distance = 0;
int initialSpeed = 0;

motor myMotor(27, 26, 33, 32);

const int trigPin = 16;
const int echoPin = 17;

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

FuzzyBrakeController brakeController;
int brakingCategory;
String outputName;

void setup()
{
  Serial.begin(9600);

  Wire.begin(22, 21);
  if (!vl53.begin(0x29, &Wire))
  {
    Serial.println("Error initializing VL53L1X sensor");
    while (1)
      delay(10);
  }

  vl53.startRanging();
  vl53.setTimingBudget(15);

  lastTime = millis();
  lastDistance = vl53.distance();

  webSocket.begin();
}

void loop()
{
  webSocket.loop();

  /* ultrasonik */
  // digitalWrite(trigPin, LOW);
  // delayMicroseconds(2);
  // digitalWrite(trigPin, HIGH);
  // delayMicroseconds(10);
  // digitalWrite(trigPin, LOW);

  // duration = pulseIn(echoPin, HIGH);
  // distanceCm = duration * SOUND_SPEED / 2;
  /* */

  // brakingCategory = brakeController.getBrakeLevel(distanceCm, myMotor._dutyCycle);
  // outputName = brakeController.getBrakeLevelName(brakingCategory);

  if (vl53.dataReady())
  {
    currentDistance = vl53.distance();
    unsigned long currentTime = millis();

    if (currentDistance != -1)
    {
      float deltaTime = (currentTime - lastTime) / 1000.0;
      relativeSpeed = (currentDistance - lastDistance) / deltaTime;

      // brakingCategory = brakeController.getBrakeLevel(currentDistance, myMotor._dutyCycle);
      // outputName = brakeController.getBrakeLevelName(brakingCategory);

      if (currentDistance < emergencyDistance)
      {
        myMotor.rapidBraking();
      }
      else if (currentDistance < brakeDistance)
      {
          myMotor.apply3StageBrake(relativeSpeed);

        if (relativeSpeed > -30 && currentDistance >= emergencyDistance)
        {
          myMotor.accelerateMotor();
        }

        lastDistance = currentDistance;
        lastTime = currentTime;
      }

      Serial.print("Distance: ");
      Serial.print(currentDistance);
      Serial.print(" mm, Relative Speed: ");
      Serial.print(relativeSpeed);
      Serial.print(" mm/s, Current PWM Duty Cycle: ");
      Serial.print(myMotor._dutyCycle);
      Serial.println(" (0-255)");

      myMotor.accelerateMotor();
    }
  }

  // Create a JSON object to send current data
  DynamicJsonDocument jsonDoc(200);
  jsonDoc["speed"] = myMotor._dutyCycle; /* dikirim */
  jsonDoc["distanceFront"] = currentDistance;
  jsonDoc["distanceBack"] = 0;
  // Serialize JSON to a string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Send JSON string over WebSocket
  webSocket.send(jsonString);

  delay(200);
}
