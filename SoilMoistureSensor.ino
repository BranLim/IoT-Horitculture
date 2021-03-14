/*
   Soil moisture sensor and monitoring.
   Brandon Lim
   2021-03-06

   A sketch that monitors the soil moisture and sends out an alert if it gets too low
*/

#include "Arduino_SensorKit.h"

#define button 4
#define sensorVccPin 7
#define alarmVccPin 5

bool moistureGood = false;
int moistureLevel = 0;

//OLED Display config
unsigned long displayActivatedOn = 0;
bool activateDisplay = false;
unsigned long displayStayOn = 5000;

//Buzzer config
unsigned long buzzerActive = 0;
unsigned long buzzerDelay = 1500;
unsigned long nextBuzzerActive = 1800000;
bool buzzerGoodToGo = true;
unsigned long nextBuzzer = 0;

//Soil moisture config
int signalPin = A0;
unsigned long soilMoistureReadTime = 0;
unsigned long soilMoistureNextRead = 1000UL * 30UL;// * 5UL; //30 seconds
bool firstReading = true;

void setup()
{
  Serial.begin(115200); //Open serial over USB

  pinMode(sensorVccPin, OUTPUT); //Enable D#7 with power
  digitalWrite(sensorVccPin, LOW); //Set D#7 pin to LOW to cut off power

  pinMode(alarmVccPin, OUTPUT); //Enable D#8 with power
  noTone(alarmVccPin); //Set the buzzer voltage low and make no noise

  pinMode(button, INPUT);

  if (Oled.begin())
  {
    Oled.setFlipMode(true);
    Oled.setFont(u8x8_font_chroma48medium8_r);
  }
  else
  {
    Serial.print("Fail to initialise OLED");
  }
}

void loop()
{
  if (isButtonPressed())
  {
    activateDisplay = true;
    displayActivatedOn = millis();
    Oled.setPowerSave(0);
    Serial.println("Activating display");   
  }
  
  if (firstReading || millis() >= soilMoistureReadTime + soilMoistureNextRead)
  {
    if (firstReading)
    {
      firstReading = false;
    }
    soilMoistureReadTime = millis();
    readSoilMoistureLevel();
    Serial.print("Soil Moisture = ");
    Serial.println(moistureLevel);
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

void displayMoistureLevel()
{
  Oled.setCursor(0, 48);
  Oled.print("Soil Moisture = ");
  Oled.setCursor(0, 28);
  Oled.print(moistureLevel);
  if (moistureLevel < 100)
  {
    Oled.print("   ");
  }
  Oled.refreshDisplay();
}

void readSoilMoistureLevel()
{  
  digitalWrite(sensorVccPin, HIGH); //Turn D#7 on with power
  delay(20); //wait 20 milliseconds
  moistureLevel = analogRead(signalPin); //Read the SIG value from the sensor
  digitalWrite(sensorVccPin, LOW); //Turn D#7 off.
}

void raiseAlarm()
{
  if (moistureLevel < 50)
  {
    return;
  }
  if (!buzzerGoodToGo && millis() >= buzzerActive + nextBuzzerActive)
  {
    Serial.println("Buzzer good to go again!");
    buzzerGoodToGo = true;
  }
  if (moistureLevel <= 600)
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
  if (moistureLevel > 900 && !moistureGood)
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
