#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <DHT.h>

using namespace std;

const char *ssid = "Misery";
const char *password = "12345679";
const char *thingSpeakApiKey = "OAGCKALLSGF7ONOO";
const char *thingSpeakChannelID = "2395540";

const char *mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char *mqttClientId = "ESP32_Client";
const int dhtPin = 5;  // DHT signal pin

DHT dht(dhtPin, DHT11);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
  dht.begin();
  connectToWifi();
  connectToBroker();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void connectToBroker()
{
  client.setServer(mqttServer, mqttPort);
  while (!client.connected())
  {
    if (client.connect(mqttClientId))
    {
    }
    else
    {
      Serial.print("Failed to Connect to Broker, rc=");
      Serial.print(client.state());
    }
  }
}

void connectToWifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() 
{
   if (WiFi.status() != WL_CONNECTED)
  {
    connectToWifi();
  }
  if (!client.connected())
  {
    connectToBroker();
  }
  int temperature = dht.readTemperature();
  int humidity = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  char sentdata;

 if(temperature <=21)
 {
  Serial2.print('J');
  sentdata = 'J';
 }
 else
 {
  Serial2.print('K');
  sentdata = 'K';
 }
 Serial.print("Sentdata = ");
 Serial.println(sentdata);
 delay(100);
 if(humidity >=80)
 {
  Serial2.print('H');
  sentdata = 'H';
 }
 else
 {
  Serial2.print('I');
  sentdata = 'I';
 }
 Serial.print("Sentdata = ");
 Serial.println(sentdata);
 delay(100);
   if(Serial2.available()>0)
   {
    String data = Serial2.readStringUntil('\n');
    Serial.println(data);
    processUARTData(data);
   }
  updateThingSpeak(temperature, humidity);
  publishMQTT("temperature2022CS", String(temperature));
  publishMQTT("humidity2022CS", String(humidity));
  delay(100);
}
void processUARTData(String data) {
  int fan;
  int led;
   sscanf(data.c_str(), "%d-%d", &fan, &led);
  
    Serial.print("LED: ");
    Serial.print(led);
    Serial.println();
    Serial.print("Fan: ");
    Serial.print(fan);
    Serial.println();
    if (led == 1) {
      client.publish("led_state2022CS", "OFF");
    } else if (led == 0) {
      client.publish("led_state2022CS", "ON");
    }
    if (fan == 1) {
      client.publish("fan_state2022CS", "OFF");
    } else if (fan == 0) {
      client.publish("fan_state2022CS", "ON");
    }

}
void updateThingSpeak(float temperature, float humidity) {
  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + String(thingSpeakApiKey) +
               "&field1=" + String(temperature) +
               "&field2=" + String(humidity);

  http.begin(url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    
  } else {
    Serial.println("Thingspeak update failed");
  }
  http.end();
}

void publishMQTT(const char *topic, String payload) {
  if (!client.connected()) {
    connectToBroker();
  }
  client.publish(topic, payload.c_str());
}

void callback(char *topic, byte *payload, unsigned int length) {
   Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)payload[i];
  }
  Serial.print(messageTemp);
  Serial.println();
  if (messageTemp == "A" || messageTemp == "B" || messageTemp == "C" || messageTemp == "D")
  {
    // Send the entire payload through UART
    for (int i = 0; i < length; i++) {
      Serial2.write(payload[i]);
    }
  }
}
