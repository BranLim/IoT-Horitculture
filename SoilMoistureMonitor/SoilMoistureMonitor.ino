/*
   Soil moisture sensor and monitoring.
   Brandon Lim
   2021-03-06

   A sketch that monitors the soil moisture and sends out an alert if it gets too low
*/

#include "Grove_I2C_Motor_Driver.h"
#include "Arduino_SensorKit.h"

#define button 4
#define sensorVccPin 7
#define sensor2VccPin 8
#define alarmVccPin 5
#define MOTOR_DRIVER_ADDRESS 0x0F
#define OLED_ADDRESS 0x3C
#define MOIST_ENOUGH 800
#define NOT_MOIST_ENOUGH 600

bool moistureGood = false;
uint16_t moistureLevels[] = {0, 0};

//OLED Display config
unsigned long displayActivatedOn = 0;
bool activateDisplay = false;
unsigned long displayStayOn = 8000;

//Buzzer config
unsigned long buzzerActive = 0;
unsigned long buzzerDelay = 1500;
unsigned long nextBuzzerActive = 1800000;
bool buzzerGoodToGo = true;
unsigned long nextBuzzer = 0;

//Soil moisture config
uint8_t signalPin = A0;
uint8_t signal2Pin = A1;
unsigned long soilMoistureReadTime = 0;
unsigned long soilMoistureNextRead = 1000UL * 30UL; //30 seconds
bool firstReading = true;

//Motor driver
bool motorsRunning[] = {false, false};
unsigned long motorsRunTime[] = {0, 0};
unsigned long motor1ToRun = 34000; //34 seconds
unsigned long motor2ToRun = 30000; //30 seconds
unsigned long motorRunCheckMoistureDelay = 4000; //4 seconds

void setup()
{
  Serial.begin(9600); //Open serial over USB

  delay(2000);

  Serial.println("Initialising OLED");
  if (Oled.begin())
  {
    Oled.clear();
    Oled.setFlipMode(true);
    Oled.setFont(u8x8_font_chroma48medium8_r);
    Serial.println("OLED initialised");
  }
  else
  {
    Serial.println("Fail to initialise OLED");
  }

  Serial.println("Initialising sensors...");
  displayMessage("Init. Sensors...");
  pinMode(sensorVccPin, OUTPUT); //Enable D#7 with power
  digitalWrite(sensorVccPin, LOW); //Set D#7 pin to LOW to cut off power

  pinMode(sensor2VccPin, OUTPUT); //Enable D#8 with power
  digitalWrite(sensor2VccPin, LOW); //Set D#8 pin to LOW to cut off power

  pinMode(alarmVccPin, OUTPUT); //Enable D#5 with power
  noTone(alarmVccPin); //Set the buzzer voltage low and make no noise

  pinMode(button, INPUT); //Set D#4 to be button input

  delay(2000);
  Serial.println();

  Serial.println("Initialising motor driver...");
  displayMessage("Init M. Drv. ...");
  Motor.begin(MOTOR_DRIVER_ADDRESS);

  Serial.println("Testing motor driver...");
  displayMessage("Test m. drv...");
  motorInitialisationTest();
  Serial.println("Motor driver initialised.");
  displayMessage("M. Drv. ready.");

  delay(2000);

  displayMessage("Sys. Ready");

  delay(2000);
  Oled.clear();
  Oled.setPowerSave(1);
  delay(2000);
}

void loop()
{
  if (isButtonPressed() && !activateDisplay)
  {
    Serial.println("Activating display");
    activateDisplay = true;
    displayActivatedOn = millis();
    Oled.setPowerSave(0);

  }

  if (firstReading || millis() >= soilMoistureReadTime + soilMoistureNextRead)
  {
    if (firstReading)
    {
      firstReading = false;
    }
    soilMoistureReadTime = millis();
    readSoilMoistureLevel();
    Serial.print("Soil Moisture 1: ");
    Serial.println(moistureLevels[0]);

    Serial.print("Soil Moisture 2: ");
    Serial.println(moistureLevels[1]);

    Serial.println("Checking if need to water plant...");
    waterPlantIfNeeded();
  }

  if (activateDisplay)
  {
    displayMoistureLevel();
  }

  raiseAlarm();

  if (activateDisplay && millis() > displayActivatedOn + displayStayOn)
  {
    Serial.println("Deactivating display");
    Oled.setPowerSave(1);
    activateDisplay = false;
  }

  if (motorsRunning[0] || motorsRunning[1])
  {
    Serial.println("Checking if pumps need to stop...");
    Oled.setPowerSave(0);
    stopPumpingWaterWhenRequired();
    Oled.setPowerSave(1);
  }
}

void waterPlantIfNeeded()
{
  if (moistureLevels[0] >= MOIST_ENOUGH && moistureLevels[1] >= MOIST_ENOUGH)
  {
    Serial.println("Nothing need to water");
    return;
  }

  Oled.setPowerSave(0);
  if (moistureLevels[0] <= NOT_MOIST_ENOUGH)
  {
    Serial.println("Plant 1 needs water");
    displayMessage("Pl. 1 thirsty");
    startPump1();
  }
  if (moistureLevels[1] <= NOT_MOIST_ENOUGH)
  {
    Serial.println("Plant 2 needs water");
    displayMessage("Pl. 2 thirsty");
    startPump2();
  }
  Oled.setPowerSave(1);
}

void startPump1()
{
  if (!motorsRunning[0])
  {
    Serial.println("Running pump 1");
    displayMessage("Run Pump 1");
    motorsRunTime[0] = millis();
    Motor.speed(MOTOR1, 100);
    motorsRunning[0] = true;
  }
}

void startPump2()
{
  if (!motorsRunning[1])
  {
    Serial.println("Running pump 2");
    displayMessage("Run Pump 2");
    motorsRunTime[1] = millis();
    Motor.speed(MOTOR2, 100);
    motorsRunning[1] = true;
  }
}

void stopPumpingWaterWhenRequired()
{
  if (motorsRunning[0] && ( moistureLevels[0] >= MOIST_ENOUGH || millis() > motorsRunTime[0] + motor1ToRun ))
  {
    Serial.println("Stop water pump 1");
    displayMessage("Stop Pump 1");
    Motor.stop(MOTOR1);
    motorsRunning[0] = false;
  }

  if (motorsRunning[1] && ( moistureLevels[1] >= MOIST_ENOUGH || millis() > motorsRunTime[1] + motor2ToRun ))
  {
    Serial.println("Stop water pump 2");
    displayMessage("Stop Pump 2");
    Motor.stop(MOTOR2);
    motorsRunning[1] = false;
  }

  if ((motorsRunning[0] || motorsRunning[1]) && millis() >= soilMoistureReadTime + motorRunCheckMoistureDelay)
  {
    Serial.println("Checking soil moisture level...");
    soilMoistureReadTime = millis();
    readSoilMoistureLevel();
    Serial.println("Soil moisture level checked.");

  }
}

bool isButtonPressed()
{
  int buttonState = digitalRead(button);
  if (buttonState == HIGH)
  {
    return true;
  }
  return false;
}

void displayMessage(String message)
{
  Oled.clear();
  Oled.setCursor(0, 0);
  Oled.print(message);
  Oled.refreshDisplay();
}

void displayMoistureLevel()
{
  Oled.setCursor(0, 0);
  Oled.print("Soil Moist. 1: ");
  Oled.setCursor(0, 18);
  Oled.print(moistureLevels[0]);

  if (moistureLevels[0] < 100)
  {
    Oled.print("   ");
  }

  Oled.setCursor(0, 28);
  Oled.print("Soil Moist. 2: ");
  Oled.setCursor(0, 38);
  Oled.print(moistureLevels[1]);
  if (moistureLevels[1] < 100)
  {
    Oled.print("   ");
  }

  Oled.refreshDisplay();
}


void readSoilMoistureLevel()
{
  readSoilMoistureSensor1();
  readSoilMoistureSensor2();
}

void readSoilMoistureSensor1()
{
  digitalWrite(sensorVccPin, HIGH); //Turn D#7 on with power
  delay(20); //wait 20 milliseconds
  moistureLevels[0] = analogRead(signalPin); //Read the SIG value from the sensor
  digitalWrite(sensorVccPin, LOW); //Turn D#7 off.
}

void readSoilMoistureSensor2()
{
  digitalWrite(sensor2VccPin, HIGH); //Turn D#8 on with power
  delay(20); //wait 20 milliseconds
  moistureLevels[1] = analogRead(signal2Pin); //Read the SIG value from the sensor
  digitalWrite(sensor2VccPin, LOW); //Turn D#8 off.
}

void raiseAlarm()
{
  if (moistureLevels[0] < 50 && moistureLevels[1] < 50)
  {
    return;
  }
  if (!buzzerGoodToGo && millis() >= buzzerActive + nextBuzzerActive)
  {
    Serial.println("Buzzer good to go again!");
    buzzerGoodToGo = true;
  }
  if (moistureLevels[0] <= NOT_MOIST_ENOUGH || moistureLevels[1] <= NOT_MOIST_ENOUGH)
  {
    if (moistureGood)
    {
      moistureGood = false;
    }
    if (buzzerGoodToGo)
    {
      Serial.println("Buzzer good to go!");
      buzzerActive = millis();

      Serial.print("Buzz ");
      buzzUrgent();

      Serial.println("Buzzer stop!");
      buzzerGoodToGo = false;
    }
    return;
  }
  if (moistureLevels[0] > 900 && moistureLevels[1] > 900 && !moistureGood)
  {
    moistureGood = true;
    if (buzzerGoodToGo)
    {
      buzzerActive = millis();
      buzzGood();
    }
  }
}

void buzzUrgent()
{
  tone(alarmVccPin, 85);
  delay(250);
  noTone(alarmVccPin);
  delay(250);
  tone(alarmVccPin, 85);
  delay(250);
  noTone(alarmVccPin);
  delay(250);
}

void buzzGood()
{
  tone(alarmVccPin, 85);
  delay(500);
  noTone(alarmVccPin);
}

void motorInitialisationTest()
{
  Motor.speed(MOTOR1, 100);
  delay(1500);
  Motor.stop(MOTOR1);

  Motor.speed(MOTOR2, 100);
  delay(1500);
  Motor.stop(MOTOR2);
}
