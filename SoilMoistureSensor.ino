/*
   Soil moisture sensor and monitoring.
   Brandon Lim
   2021-03-06

   A sketch that monitors the soil moisture and sends out an alert if it gets too low
*/

#include "Arduino_SensorKit.h"

int signalPin = A0;
int sensorVccPin = 7;
int alarmVccPin = 5;
int buzzReminder = 3;
bool moistureGood = false;
int moistureLevel = 0;

void setup()
{
  Serial.begin(115200); //Open serial over USB

  pinMode(sensorVccPin, OUTPUT); //Enable D#7 with power
  digitalWrite(sensorVccPin, LOW); //Set D#7 pin to LOW to cut off power

  pinMode(alarmVccPin, OUTPUT); //Enable D#8 with power
  noTone(alarmVccPin); //Set the buzzer voltage low and make no noise

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
  getSoilMoistureLevel();
  displayMoistureLevel();
  raiseAlarm();
  Serial.print("Soil Moisture = ");
  Serial.println(moistureLevel);
  delay(1000UL); //Delay for 30 minutes before doing another check.
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
  //delay(1000);

}

void getSoilMoistureLevel()
{
  digitalWrite(sensorVccPin, HIGH); //Turn D#7 on with power
  delay(20); //wait 20 milliseconds
  moistureLevel = analogRead(signalPin); //Read the SIG value from the sensor
  digitalWrite(sensorVccPin, LOW); //Turn D#7 off.
  return moistureLevel;
}

void raiseAlarm()
{
  int buzzCount = 0;
  if (moistureLevel < 50)
  {
    return;
  }

  if (moistureLevel <= 600)
  {
    if (moistureGood)
    {
      moistureGood = false;
    }

    while (buzzCount < buzzReminder)
    {

      if (moistureLevel < 400)
      {
        buzzUrgent();
      }
      else
      {
        buzzGentle();
      }

      buzzCount++;
    }
    return;
  }
  if (moistureLevel >= 850 && !moistureGood)
  {
    moistureGood = true;
    buzzGood();
  }
}

void buzzGentle()
{
  tone(alarmVccPin, 60);
  delay(1500);
  noTone(alarmVccPin);
  delay(1500);
}

void buzzUrgent()
{
  tone(alarmVccPin, 85);
  delay(1000);
  noTone(alarmVccPin);
  delay(1000);
}

void buzzGood()
{
  tone(alarmVccPin, 85);
  delay(1000);
  noTone(alarmVccPin);
  delay(500);
}
