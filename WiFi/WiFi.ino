#include "WiFiEsp.h"

const byte rxPin = 2;
const byte txPin = 3;

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial wifiSerial(rxPin, txPin); //PIN 2 to receive data from WiFi, PIN 3 to transmit data to WiFi module
#endif

const char* wifiName = "";
const char* wifiPassword = "";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);

  wifiSerial.begin(9600);
  WiFi.init(&wifiSerial);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(wifiName, wifiPassword);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("*");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address is: ");
  Serial.println(WiFi.localIP());

}

void loop() {

}
