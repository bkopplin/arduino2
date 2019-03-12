#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

// wifi setup
const char* ssid = "bjarne_pc";
const char* password = "passwort";
const char* host = "api.openweathermap.org";
const char* url =  "/data/2.5/weather?q=Dundee,uk&appid=79314a41d8fb700c1e1b6aecc40f0cb0";

// variables to store weather data
String weatherMain = "";
String weatherDescription = "";
double windSpeed = 0;
double temp;
double humidity;

// LED pins
int greenLED = 14;
int redLED = 12;
int blueLED = 13;

int greenVal = 0;
int redVal = 0;
int blueVal = 0;

String behaviourLED;

void setup() {
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);

  Serial.begin(115200);
  delay(100);

  // connecting to a WiFi network
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
  // get weather data from JSON object
  Serial.println(F("Response:"));

  weatherMain = root["weather"][0]["main"].as<char*>();
  weatherDescription = root["weather"][0]["description"].as<char*>();
  windSpeed = root["wind"]["speed"].as<double>();
  temp = root["main"]["temp"].as<double>();
  temp = temp - 273.15;
  humidity = root["main"]["humidity"].as<double>();;
  Serial.print("main: ");
  Serial.println(weatherMain);
  Serial.print("weather description: ");
  Serial.println(weatherDescription);
  Serial.print("wind speed: ");
  Serial.println(windSpeed);
  Serial.print("temp: ");
  Serial.println(temp);
  Serial.print("humidity: ");
  Serial.println(humidity);



  // Disconnect
  client.stop();

  // determine the behaviour of the rgb LED
  determineBehaviour();
  behaviourLED = "heavy_rain";
  setColour(redVal, greenVal, blueVal);

}

void loop() {
  displayWeather();
}

/**
   updates the state of the led depending on the weather
*/
void displayWeather() {
  if (behaviourLED == "light_rain") {
    /*
       light rain
    */
    for (int i = 10; i < 150; ++i) {
      changeBrightness(i);
      delay(20);
    }
    delay(700);
    for (int i = 150; i > 10; --i) {
      changeBrightness(i);
      delay(20);
    }
    delay(1000);
  } else if (behaviourLED == "heavy_rain") {
    Serial.println("yeah it works");
    /*
       heavy rain
    */
    for (int i = 1; i < 150; ++i) {
      changeBrightness(i);
      delay(10);
    }
    delay(500);
    for (int i = 150; i > 1; --i) {
      changeBrightness(i);
      delay(10);
    }
    delay(700);
  }
}

/**
   determines the behaviour of the LED depending on the weather description from the API
   reference: https://openweathermap.org/weather-conditions
*/
String determineBehaviour() {
  Serial.print("LED behaviour: ");
  if (weatherDescription == "moderate rain" || weatherDescription == "light rain" || weatherDescription == "light intensity shower rain" || weatherDescription == "shower rain") {
    Serial.println("light rain");
    behaviourLED = "light_rain";
    greenVal = 255;
    blueVal = 255;
  } else if (weatherMain == "Rain"  ) {
    Serial.println("heavy rain");
    behaviourLED = "heavy_rain";
  } else if (weatherDescription == "clear sky" || weatherDescription == "few clouds") {
    Serial.println("clear");
    behaviourLED = "clear";
  } else if (weatherMain == "Clouds") {
    Serial.println("clouds");
    behaviourLED = "clouds";
  } else if (weatherMain == "Thunderstorm") {
    Serial.println("thunderstorm");
    behaviourLED = "thunderstorm";
  }
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
  setColour(0, 255, 255);  // blue
  Serial.println("BLUE");
  delay(1000);
  setColour(0, 100, 100);  // blue
  Serial.println("BLUE");
  delay(1000);
  setColour(255, 0, 255);  // blue
  Serial.println("BLUE");
  delay(1000);
  setColour(255, 255, 0);  // blue
  Serial.println("BLUE");
  delay(1000);
}

//receive values for RGB from main loop function
//for each value received, output this to the appropriate pin
void setColour(int r, int g, int b)
{
  analogWrite(redLED, r);
  analogWrite(greenLED, g);
  analogWrite(blueLED, b);
}

void changeBrightness(int brightness) {
  brightness = 255 - brightness;
  setColour(redVal - brightness, greenVal - brightness, blueVal - brightness);
}
