#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

//#include <credentials.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


WiFiClient client;
const int httpPort = 80;

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define IO_USERNAME  "USERNAME"
#define IO_KEY       "AIO_KEY"
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, IO_USERNAME, IO_KEY);
Adafruit_MQTT_Subscribe ForceUpdate = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/FEED1");
Adafruit_MQTT_Subscribe AutoUpdate = Adafruit_MQTT_Subscribe(&mqtt, IO_USERNAME "/feeds/FEED2");


String host = "gfx.globalapi.net";
String url = "/api/v1/addresses.json";
bool isAutoUpdate = true;

unsigned long lastUpdateTime;
int updateInterval = 10 * 1000*60; // 10 minutes

void executeGetFlixUpdater()
{
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
  }
  client.print(String("PUT ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n" +
               "Accept: */*\r\n" +
               "User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n" +
               "Authorization:AUTHKEY\r\n" +
               "\r\n");

  Serial.println("request sent");
  client.stop();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  delay(10);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("GETFLIX_Updater");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  mqtt.subscribe(&ForceUpdate);
  mqtt.subscribe(&AutoUpdate);
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  lastUpdateTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  if (isAutoUpdate)
  {
    if ((millis() - lastUpdateTime) >= updateInterval)
    {
      executeGetFlixUpdater();
      lastUpdateTime = millis();
    }
  }
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &ForceUpdate) {
      String val = (char *)ForceUpdate.lastread;
      if (val == "1") {
        executeGetFlixUpdater();
        Serial.print("Force Update Val:");
        Serial.println(val);
        delay(100);
      }
    }
    else if (subscription == &AutoUpdate) {
      String val = (char *)AutoUpdate.lastread;
      Serial.print("Auto Update Val:");
      Serial.println(val);
      if (val == "ON") {
        isAutoUpdate = true;
      }
      else if (val == "OFF") {
        isAutoUpdate = false;
      }
    }
    yield();
  }
}
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
