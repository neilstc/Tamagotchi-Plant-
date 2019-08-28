#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "DHT.h"
#include <stdio.h>
#include <stdlib.h>

#define DHTTYPE DHT11 // DHT 11// Uncomment whatever type you're using!
//#define DHTTYPE DHT22
#define InPin12 12 // air temp and humidity
#define InPin39 39 // light intensity
#define InPin36 36 // soil humidity
/// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, 5,4);
// Initialize DHT sensor.
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(InPin12, DHTTYPE);

//wifi credentials
const char* ssid = "neil"; //wifi network name
const char* password = "123123123"; // wifi network password
WiFiServer server(80);

//mqtt credentials
const char* mqttServer = "postman.cloudmqtt.com";
const int mqttPort = 12113;
const char* mqttUser = "ywmcfstp";
const char* mqttPassword = "1ozS2uUbzLQd";

//clients
WiFiClient espClient;
PubSubClient client(espClient);

// Globals
int val = 0; //value for storing moisture value 
int soilPin = A0;//Declare a variable for the soil moisture sensor 
int soilPower = 7;//Variable for Soil moisture Power
char snum[10];

void getSoilMoistureSensor() {
  Serial.print("Soil Moisture = ");    
  //get soil moisture value from the function below and print it
  int val = readSoil();
  Serial.println(val);
  client.publish("soil", itoa(val, snum, 10));
  
  //This 1 second timefrme is used so you can test the sensor and see it change in real-time.
  //For in-plant applications, you will want to take readings much less frequently.
  delay(1000);//take a reading every second
}

void soilMoistureSensorSetup() {
  pinMode(soilPower, OUTPUT);//Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW);//Set to LOW so no power is flowing through the sensor
}

int readSoil() {
  digitalWrite(soilPower, HIGH);//turn D7 "On"
  delay(10);//wait 10 milliseconds 
  val = analogRead(InPin36);//Read the SIG value form sensor 
  digitalWrite(soilPower, LOW);//turn D7 "Off"
  return val;//send current moisture value
}

void lightSensorSetup() {
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void getLightSensor() {
  int value = analogRead( InPin39 );
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 128, String(value) );
  Serial.println("New light value: " + String(value));
  display.display();
  client.publish("light", itoa(value, snum, 10));
  delay(100);
}

void airTempHumiditySensorSetup() {
  //Serial.println("DHTxx test!");
  dht.begin();
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void getAirTempHumiditySensor() {
  // Wait between measurements.
  delay(1000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default) and as Fahrenheit (isFahrenheit = true)
  float temp = dht.readTemperature();
  float ftemp = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temp) || isnan(ftemp)) {
    //Serial.println("Failed to read from DHT sensor!"); return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temp, humidity, false);
  Serial.print("Humidity: "); Serial.print(humidity); Serial.print(" %\t"); Serial.print("Temperature: ");
  Serial.print(temp); Serial.print(" *C \t"); Serial.print("Heat index: "); Serial.print(hic); Serial.println(" *C ");
  // clear the display
  display.clear();
  //display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawStringMaxWidth(0, 0, 128, "Humidity:"+String(humidity));
  display.drawStringMaxWidth(0, 30, 128, "Temperature:"+String(temp));
  // write the buffer to the display
  display.display();
  client.publish("temperature", itoa(temp, snum, 10));
  //Serial.println("debug: " + (int)temp);
  client.publish("humidity", itoa(humidity, snum, 10));
  delay(1000);
}

void wifiSetup() {
  //connecting to wifi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected");
}

void mqttSetup() {
  client.setServer(mqttServer, mqttPort);
  while(!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if(client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Connected");
    }
    else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);   // open serial over USB
  wifiSetup();
  mqttSetup();
  lightSensorSetup();
  airTempHumiditySensorSetup();
  soilMoistureSensorSetup();
}

void loop() {
  getAirTempHumiditySensor();
  getLightSensor();
  getSoilMoistureSensor();
}
