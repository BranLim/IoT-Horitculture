#include "WiFiEsp.h"

const byte rxPin = 2;
const byte txPin = 3;

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial wifiSerial(rxPin, txPin); //PIN 2 to receive data from WiFi, PIN 3 to transmit data to WiFi module
#endif



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  wifiSerial.begin(9600);
  WiFi.init(&wifiSerial);

}

void loop() {
  Serial.println();
  Serial.println("Scanning available networks ... ");

  printNetworkScanResult();
  delay(10000);
}

void printNetworkScanResult()
{
  int numSsid = WiFi.scanNetworks();
  while (numSsid == -1)
  {
    Serial.println("Couldn't get a WiFi connection");
    delay(3000);
    numSsid = WiFi.scanNetworks();
  }


  Serial.print("Number of available networks: ");
  Serial.println(numSsid);

  for (int i = 0; i < numSsid; i++)
  {
    Serial.print(i + 1);
    Serial.print(")");
    Serial.print(WiFi.SSID(i));
    Serial.println();
  }
}
