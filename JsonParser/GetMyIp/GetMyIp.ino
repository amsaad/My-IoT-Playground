#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "StringSplitter.h"
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

WiFiClient client;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect();
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(1000);
  //    Serial.println("Connecting...");
  //  }
}

void loop() {
  // put your main code here, to run repeatedly:
  GetMyIP();
  delay(10000);
}
const char* host = "api.ipify.org";
const int httpPort = 80;

void GetMyIP()
{
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failled");
  }
  String Request = "GET /?format=json HTTP/1.1\r\n";
  Request += "Host: " + String(host) + "\r\n";
  Request += "\r\n";

  client.print(Request);
  String line = client.readString();
  Serial.println("request sent");
  String payload = "{";
  StringSplitter *splitter = new StringSplitter(line, '{', 2);
  payload += splitter->getItemAtIndex(1);
  char json[payload.length() + 1];
  payload.toCharArray(json, payload.length());;
  StaticJsonDocument<256> doc;
  deserializeJson(doc, json);
  String ip = doc["ip"];
  Serial.print("My IP: \t");
  Serial.println(ip);
}
