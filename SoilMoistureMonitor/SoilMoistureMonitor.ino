/*
   Soil moisture sensor and monitoring.
   Brandon Lim
   2021-03-06

   A sketch that monitors the soil moisture and sends out an alert if it gets too low
*/

#include "Arduino_SensorKit.h"

#define button 4
#define sensorVccPin 7
#define sensor2VccPin 8
#define alarmVccPin 5

bool moistureGood = false;

int moistureLevels[] = {0, 0};

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
int signal2Pin = A1;
unsigned long soilMoistureReadTime = 0;
unsigned long soilMoistureNextRead = 1000UL * 30UL;// * 5UL; //30 seconds
bool firstReading = true;

void setup()
{
  Serial.begin(115200); //Open serial over USB

  pinMode(sensorVccPin, OUTPUT); //Enable D#7 with power
  digitalWrite(sensorVccPin, LOW); //Set D#7 pin to LOW to cut off power

  pinMode(sensor2VccPin, OUTPUT); //Enable D#8 with power
  digitalWrite(sensor2VccPin, LOW); //Set D#8 pin to LOW to cut off power

  pinMode(alarmVccPin, OUTPUT); //Enable D#5 with power
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
    Serial.print("Soil Moisture 1: ");
    Serial.println(moistureLevels[0]);

    Serial.print("Soil Moisture 2: ");
    Serial.println(moistureLevels[1]);
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
  Oled.setCursor(0, 0);
  Oled.print("Soil Moist. 1: ");
  Oled.setCursor(0, 18);
  Oled.print(moistureLevels[0]);
  
  if (moistureLevel < 100)
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
  digitalWrite(sensorVccPin, HIGH); //Turn D#7 on with power
  delay(20); //wait 20 milliseconds
  moistureLevels[0] = analogRead(signalPin); //Read the SIG value from the sensor
  digitalWrite(sensorVccPin, LOW); //Turn D#7 off.

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
  if (moistureLevels[0] <= 600 || moistureLevels[1] <= 600)
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
