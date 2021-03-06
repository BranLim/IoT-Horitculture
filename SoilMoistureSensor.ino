/*
   Soil moisture sensor and monitoring.
   Brandon Lim
   2021-03-06

   A sketch that monitors the soil moisture and sends out an alert if it gets too low
*/

int signalPin = A0;
int sensorVccPin = 7;
int alarmVccPin = 5;
int buzzReminder = 3;
bool moistureGood = false;

void setup()
{
  Serial.begin(9600); //Open serial over USB

  pinMode(sensorVccPin, OUTPUT); //Enable D#7 with power
  pinMode(alarmVccPin, OUTPUT); //Enable D#8 with power

  digitalWrite(sensorVccPin, LOW); //Set D#7 pin to LOW to cut off power
  noTone(alarmVccPin); //Set the buzzer voltage low and make no noise
}

void loop()
{
  int moistureLevel = getSoilMoistureLevel();
  raiseAlarm(moistureLevel);
  //Serial.print("Soil Moisture = ");
  //Serial.println(moistureLevel);
  delay(1000UL * 60UL * 30UL); //Delay for 30 minutes before doing another check.
}

int getSoilMoistureLevel()
{
  int moistureLevel = 0;
  digitalWrite(sensorVccPin, HIGH); //Turn D#7 on with power
  delay(20); //wait 20 milliseconds
  moistureLevel = analogRead(signalPin); //Read the SIG value from the sensor
  digitalWrite(sensorVccPin, LOW); //Turn D#7 off.
  return moistureLevel;
}

void raiseAlarm(int moistureLevel)
{
  int buzzCount = 0;

  if (moistureLevel <= 600)
  {
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
  if (moistureLevel >= 1000 && !moistureGood)
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
  tone(alarmVccPin, 75);
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
