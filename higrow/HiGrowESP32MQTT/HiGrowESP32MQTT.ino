/*
  HiGrowESP32MQTT
  (c) Claus Kuehnel 2018-03-18 info@ckuehnel.ch

  The ESP32 reads data from HiGrow device and publishes these via MQTT

  based on https://github.com/LilyGO/higrowopen/tree/master/HiGrowEsp32
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "credentials.h"

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define uS_TO_S_FACTOR 1000000

//int DEEPSLEEP_SECONDS = 1800; // 30 min
int DEEPSLEEP_SECONDS = 180; // 3 min

/* create an instance of PubSubClient client */
WiFiClient espClient;
PubSubClient client(espClient);

uint64_t chipid;

const int dhtpin = 22;
const int soilpin = 32;
const int POWER_PIN = 34;
const int LIGHT_PIN = 33;

// Initialize DHT sensor.
DHT dht(dhtpin, DHTTYPE);

// Temporary variables
static char celsiusTemp[7];
static char humidityTemp[7];
char msg[20];

/* topics */
#define TEMP_TOPIC    "temperature"
#define HUMI_TOPIC    "humidity"
#define SOIL_TOPIC    "soil"
#define LIGHT_TOPIC   "light"
#define GPS_TOPIC     "gps"

#define ShallISleep  false

// Client variables 
char linebuf[80];
int charcount=0;

char deviceid[21];

void setup() 
{
  dht.begin();
  
  Serial.begin(115200);
  delay(2000); // wait for monitor

  esp_sleep_enable_timer_wakeup(DEEPSLEEP_SECONDS * uS_TO_S_FACTOR);

  pinMode(16, OUTPUT); // blue LED
  pinMode(POWER_PIN, INPUT);
  digitalWrite(16, LOW);  

  chipid = ESP.getEfuseMac();
  sprintf(deviceid, "%" PRIu64, chipid);
  Serial.print("DeviceId: ");
  Serial.println(deviceid);

  connectWiFi();
  configureMQTT();
}

void loop() 
{
  char body[1024];
  digitalWrite(16, LOW); //switched on

  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) {
    mqttconnect();
  }

  sensorsData(body);
  delay(500);
  if (ShallISleep==true)
  {
      WiFi.disconnect(true);
      Serial.println("Going to Deep Sleep...");
      esp_deep_sleep_start();
  } else
  {
    Serial.println("not going to Deep Sleep, but wait 10 seconds...");
    delay(10000);               // used for test    
  }

}

void sensorsData(char* body)
{
  //This section reads all sensors
  
  int waterlevel = analogRead(soilpin);
  int lightlevel = analogRead(LIGHT_PIN);
  
  waterlevel = map(waterlevel, 0, 4095, 0, 1023);
  waterlevel = constrain(waterlevel, 0, 1023);
  if (!isnan(waterlevel)) 
  {
    snprintf (msg, 20, "%d", waterlevel);
    /* publish the message */
    //client.publish(SOIL_TOPIC, msg);
    char s[100];
    sprintf(s,"higrow/%s/%s",deviceid,SOIL_TOPIC);
    client.publish(s, msg);
  }
  
  lightlevel = map(lightlevel, 0, 4095, 0, 1023);
  lightlevel = constrain(lightlevel, 0, 1023);
  if (!isnan(lightlevel)) 
  {
    snprintf (msg, 20, "%d", lightlevel);
    /* publish the message */
    char s[100];
    sprintf(s,"higrow/%s/%s",deviceid,LIGHT_TOPIC);
    client.publish(s, msg);
  }
  
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  if (!isnan(humidity)) 
  {
    snprintf (msg, 20, "%5.1f", humidity);
    char s[100];
    sprintf(s,"higrow/%s/%s",deviceid,HUMI_TOPIC);
    /* publish the message */
    client.publish(s, msg);
  }
  
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  if (!isnan(temperature)) 
  {
    snprintf (msg, 20, "%5.1f", temperature);
    /* publish the message */
    char s[100];
    sprintf(s,"higrow/%s/%s",deviceid,TEMP_TOPIC);
    client.publish(s, msg);
  }

  char s[100];
  sprintf(s,"higrow/%s/%s",deviceid,GPS_TOPIC);
  // Pat
  // client.publish(s, "48.161852, 11.413413");
  // Winand
  client.publish(s, "48.171890, 11.639667");
  sprintf(s,"higrow/%s/%s",deviceid,"name");
  client.publish(s, "Winand");
 
  Serial.print("DevideId: "); Serial.println(deviceid);
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" *C");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %rF");
  Serial.print("Soil: "); Serial.println(waterlevel);
  Serial.print("Light: "); Serial.println(lightlevel);
}


