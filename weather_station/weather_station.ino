#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

const char* ssid = "bjarne_pc";
const char* password = "passwort";
//const char* host = "www.openweathermap.org/";
//const char* url = "/api/location/18637/";
const char* host = "api.openweathermap.org";
const char* url =  "/data/2.5/weather?q=Dundee,uk&appid=79314a41d8fb700c1e1b6aecc40f0cb0";

// LED stuff
int greenLED = 13;
int redLED = 12;
int blueLED = 14;

// sensor
Adafruit_AM2320 am2320 = Adafruit_AM2320();
void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  Serial.begin(115200);
  delay(100);
  am2320.begin();

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(5000);

  ///CAN BE LOOPED

  Serial.print("connecting to ");
  Serial.println(host);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  // Send HTTP request
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(F("Parsing failed!"));
    return;
  }

  //  JsonObject& weather = root["consolidated_weather"];
  //  if (!weather.success()) {
  //    Serial.println(F("Parsing failed!"));
  //    return;
  //  }
  // Extract values
  Serial.println(F("Response:"));
  Serial.print("main: ");
  Serial.println(root["weather"][0]["main"].as<char*>());



  // Disconnect
  client.stop();

}

void loop() {
 testLED();
// Serial.print("Temp: "); Serial.println(am2320.readTemperature());
//  Serial.print("Hum: "); Serial.println(am2320.readHumidity());
//  Serial.println("");
}

void testLED() {
   setColour(255, 0, 0);  // red
  Serial.println("RED");
  delay(1000);
  setColour(0, 255, 0);  // green
  Serial.println("GREEN");
  delay(1000);
  setColour(0, 0, 255);  // blue
  Serial.println("BLUE");
  delay(1000);
  setColour(255, 255, 0);  // yellow
  Serial.println("YELLOW");
  delay(1000);  
  setColour(80, 0, 80);  // purple
  Serial.println("PURPLE");
  delay(1000);
  setColour(0, 255, 255);  // aqua
  Serial.println("AQUA");
  delay(1000);
}
//
//// receive values for RGB from main loop function
//// for each value received, output this to the appropriate pin
void setColour(int r, int g, int b)
{
  analogWrite(redLED, r);
  analogWrite(greenLED, g);
  analogWrite(blueLED, b);  
}
