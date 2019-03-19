#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#define PIN 15


const char* ssid = "bjarne_pc";
const char* password = "passwort";
const char* host = "api.openweathermap.org";
String location = "Dundee";
String url =  "/data/2.5/weather?q=" + location + "&appid=79314a41d8fb700c1e1b6aecc40f0cb0";
//http://api.openweathermap.org/data/2.5/weather?q=dundee&appid=79314a41d8fb700c1e1b6aecc40f0cb0
// variables to store weather data
String weatherMain = "";
String weatherDescription = "";
String icon = "";
String windSpeed = "";
double temp;
String tempStr = "";
double humidity;
String api_cod;
String api_message = "";
bool power_on = true;

// variables for LED to function
String behaviourLED;

ESP8266WebServer server(80);

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:

    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    int loopA = 0;
    int loopB = 0;
    int loopC = 0;
    int loopD = 0;
    //int pattern = 3;

    //void (*OnComplete)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
      : Adafruit_NeoPixel(pixels, pin, type)
    {
      // OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {
      if ((millis() - lastUpdate) > Interval) // time to update
      {
        lastUpdate = millis();
        switch (ActivePattern)
        {
          case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
          case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
          case COLOR_WIPE:
            ColorWipeUpdate();
            break;
          case SCANNER:
            ScannerUpdate();
            break;
          case FADE:
            FadeUpdate();
            break;
          default:
            break;
        }
      }
    }

    // Increment the Index and reset at the end
    void Increment()
    {
      if (Direction == FORWARD)
      {
        Index++;
        if (Index >= TotalSteps)
        {
          Index = 0;
          //if (OnComplete != NULL)
          //{
          //OnComplete(); // call the comlpetion callback
          //}
        }
      }
      else // Direction == REVERSE
      {
        --Index;
        if (Index <= 0)
        {
          Index = TotalSteps - 1;
          //if (OnComplete != NULL)
          //{
          //OnComplete(); // call the comlpetion callback
          //}
        }
      }
    }

    // Reverse pattern direction
    void Reverse()
    {
      if (Direction == FORWARD)
      {
        Direction = REVERSE;
        Index = TotalSteps - 1;
      }
      else
      {
        Direction = FORWARD;
        Index = 0;
      }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = RAINBOW_CYCLE;
      Interval = interval;
      TotalSteps = 255;
      Index = 0;
      Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
      }
      show();
      Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = THEATER_CHASE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color1;
      Color2 = color2;
      Direction = dir;
      Index = 0;
    }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if ((i + Index) % 3 == 0)
        {
          setPixelColor(i, Color1);
        }
        else
        {
          setPixelColor(i, Color2);
        }
      }
      show();
      Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
      ActivePattern = COLOR_WIPE;
      Interval = interval;
      TotalSteps = numPixels();
      Color1 = color;
      Index = 0;
      Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
      setPixelColor(Index, Color1);
      show();
      Increment();
    }

    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
      ActivePattern = SCANNER;
      Interval = interval;
      TotalSteps = (numPixels() - 1) * 2;
      Color1 = color1;
      Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    {
      for (int i = 0; i < numPixels(); i++)
      {
        if (i == Index)  // Scan Pixel to the right
        {
          setPixelColor(i, Color1);
        }
        else if (i == TotalSteps - Index) // Scan Pixel to the left
        {
          setPixelColor(i, Color1);
        }
        else // Fading tail
        {
          setPixelColor(i, DimColor(getPixelColor(i)));
        }
      }
      show();
      Increment();
    }

    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {

      ActivePattern = FADE;
      Interval = interval;
      TotalSteps = steps;
      Color1 = color1;
      Color2 = color2;
      Index = 0;
      Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
      // Calculate linear interpolation between Color1 and Color2
      // Optimise order of operations to minimize truncation error
      uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
      uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
      uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

      ColorSet(Color(red, green, blue));
      show();
      Increment();
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
      // Shift R, G and B components one bit to the right
      uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
      return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
      for (int i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, color);
      }
      show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
      return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
      return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
      return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
      WheelPos = 255 - WheelPos;
      if (WheelPos < 85)
      {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
      }
      else if (WheelPos < 170)
      {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
      }
      else
      {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
      }
    }
};

void Ring1Complete();

NeoPatterns Ring1(24, PIN , NEO_GRB + NEO_KHZ800, &Ring1Complete);

/*
   setup
*/
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(600);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Ring1.begin();
  Ring1.setBrightness(200);
  doAPICall();

  /*
       set up web server
  */
  server.on("/", handleRoot);
  server.on("/weather", sendWeather);
  server.on("/change_behaviour", changeBehaviour);
  server.onNotFound(handleNotFound);
  server.on("/changelocation", handleLocation);
  server.on("/togglepower", togglePower);
  server.begin();


}

void doAPICall() {
  /*
      get weather information from API
  */
  // connect to server
  Serial.print("connection to ");
  Serial.println(host);
  WiFiClient client;
  if (!client.connect(host, 80)) {
    Serial.println("connection failed");
    return;
  }

  // send HTTP request
  client.print(String("GET ") + "/data/2.5/weather?q=" + location + "&appid=79314a41d8fb700c1e1b6aecc40f0cb0" + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }
  client.stop();
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
  api_cod = root["cod"].as<char*>();
  if (api_cod == "200") {

    weatherMain = root["weather"][0]["main"].as<char*>();
    weatherDescription = root["weather"][0]["description"].as<char*>();
    icon = root["weather"][0]["icon"].as<char*>();
    windSpeed = root["wind"]["speed"].as<char*>();
    temp = root["main"]["temp"].as<double>();
    temp = temp - 273.15;
    humidity = root["main"]["humidity"].as<double>();
    Serial.print("location: ");
    Serial.println(location);
    Serial.print("weather description: ");
    Serial.println(weatherDescription);
    Serial.print("icon: ");
    Serial.print(icon);
    Serial.print("wind speed: ");
    Serial.println(windSpeed);
    Serial.print("temp: ");
    Serial.println(temp);
    Serial.println();
  } else if (api_cod == "404") {
    api_message = root["message"].as<char*>();
  }
  tempStr = String(temp, 1);
}

/*
    define functions for web server
*/
void handleRoot() {
  Serial.println("request for / recieved");
  String index = "<!DOCTYPE html><html><head> <meta charset='utf-8'/> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <title>Page Title</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'> <script src='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js' integrity='sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy' crossorigin='anonymous'></script> <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'> <style>:root{--main-color: #52928b; /* --secondary-color: #004d47; */ --secondary-color: #b9c4c7;}/* the animation if modified from https://www.w3schools.com/howto/howto_css_loader.asp */ .loader{top: 50px; display:none; border: 10px solid var(--main-color); border-radius: 50%; border-top: 10px solid var(--secondary-color); width: 120px; height: 120px; -webkit-animation: spin 1s linear infinite; /* Safari */ animation: spin 1s linear infinite; position: relative; margin: 0 auto;}@keyframes spin{0%{transform: rotate(0deg);}100%{transform: rotate(360deg);}}#loader-container{background-color: #b9b9b9; opacity: 0.9; display: none; z-index: 200;}.secondary-bg-color{background-color: var(--secondary-color);}.container{max-width: 670px;}#ptemperature{font-size: 40px;}#pdescription, #icon{display: inline-block;}#btnSubmit:hover, #btnChangeBehaviour:hover, #powerToggle:hover{cursor: pointer; opacity: 0.8; transition: opacity 1s;}#btnSubmit, #btnChangeBehaviour, #powerToggle{transition: opacity 1s;}#btnChangeBehaviour .input-group-text:hover{cursor: pointer;}#newLocation{text-align: left;}.input-group-append .input-group-text{background-color: var(--main-color); color: white;}.input-group-prepend .input-group-text{background-color: var(--secondary-color); color: white;}.navbar{background-color: var(--main-color);}.navbar a{color: white;}.power-on{color: #ffffff;}.power-off{color: black;}</style></head><body> <div id='loader-container' style='position: fixed; width: 100%; height: 100%'> <div class='loader' id='loader' style='display:none'></div></div><nav class='navbar navbar-expand-lg '> <div class='container'> <a class='navbar-brand' href='#'>myCactus</a> <div class='navbar' id='navbarNav'> <ul class='navbar-nav'> <li class='nav-item '> <span class='nav-link power-on' id='powerToggle'><i class='fa fa-power-off '></i></span> </li></ul> </div></div></nav> <div class='container' id='content'> <div class='row'> <div class='col-12'> <div id='alertBox' class='alert alert-success mt-4' role='alert' style='visibility: visible'> </div><h4>change location</h4> <div class='input-group mb-3 mt-2 input-group-lg'> <div class='input-group-prepend'> <span class='input-group-text' id='basic-addon1'><i class='fa fa-map-marker-alt '></i></span> </div><input type='text' class='form-control' placeholder='type in your location' id='newLocation'> <div class='input-group-append' id='btnSubmit'> <span class='input-group-text'><i class='fa fa-search '></i></span> </div></div><h1 id='plocation'></h1> <div><span id='ptemperature'></span><span> °C</span></div><img id='icon' src='http://openweathermap.org/img/w/10d.png' alt='image weather' style='display: inline-block'> <p id='pdescription'></p><hr> <h4>change behaviour</h4> <div class='input-group input-group-lg mb-3'> <div class='input-group-prepend'> <span class='input-group-text' id='basic-addon1'><i class='fa fa-lightbulb '></i></span> </div><select class='custom-select form-control' id='selectBehaviour'> <option value='clear sky'>clear sky</option> <option value='rain'>rain</option> <option value='thunderstorm'>thunderstorm</option> <option value='cloudy'>cloudy</option> </select> <div class='input-group-append' id='btnChangeBehaviour'> <label class='input-group-text'><i class='fa fa-arrow-right '></i></label> </div></div></div></div></div><script>var newLocation, btnSubmit, loader_container, loader, plocation, ptemperature, pdescription, icon, btnChangeBehaviour, selectBehaviour, alertBox, powerToggle; alertBox=document.getElementById('alertBox'); powerToggle=document.getElementById('powerToggle'); loader_container=document.getElementById('loader-container'); loader=document.getElementById('loader'); plocation=document.getElementById('plocation'); icon=document.getElementById('icon'); ptemperature=document.getElementById('ptemperature'); pdescription=document.getElementById('pdescription'); btnChangeWeather=document.getElementById('btnChangeWeather'); newLocation=document.getElementById('newLocation'); btnSubmit=document.getElementById('btnSubmit'); btnChangeBehaviour=document.getElementById('btnChangeBehaviour'); selectBehaviour=document.getElementById('selectBehaviour'); btnSubmit.addEventListener('click', function (){setLocation(newLocation.value);}); newLocation.addEventListener('keypress', function (e){if (e.keyCode==13){setLocation(newLocation.value);}}); powerToggle.addEventListener('click', function(){togglePowerState();}); btnChangeBehaviour.addEventListener('click', function (){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); var str=JSON.parse(xhttp.responseText); if (str['cod']==200){toggleAnimation(); displayAlert('behaviour successfully changed', 'success');}}}; toggleAnimation(); xhttp.open('POST', '/change_behaviour'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('new_behaviour=' + selectBehaviour.value);}); function updateWeatherInformation(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); setWeatherInfo(xhttp.responseText); toggleAnimation();}}; toggleAnimation(); xhttp.open('GET', '/weather'); xhttp.send();}function setLocation(location){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); setWeatherInfo(xhttp.responseText); toggleAnimation();}}; toggleAnimation(); xhttp.open('POST', '/changelocation'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('location=' + location);}function setWeatherInfo(jsonWeatherdata){var weatherData=JSON.parse(jsonWeatherdata); if (weatherData['cod']=='404'){displayAlert(weatherData['message'], 'danger');}else{setPowerButton(weatherData['state']); newLocation.value=weatherData['location']; ptemperature.innerHTML=weatherData['temperature']; pdescription.innerHTML=weatherData['weather']; icon.src='http://openweathermap.org/img/w/' + weatherData['icon'] + '.png'; displayAlert('weather updated', 'success');}}function toggleAnimation(){var display=loader.style.display; if (display=='none'){loader_container.style.display='block'; loader.style.display='block';}else{loader_container.style.display='none'; loader.style.display='none';}}function displayAlert(message, alertType){alertBox.innerHTML=message; alertBox.style.visibillity='visible'; if (alertType=='success'){alertBox.classList.add('alert-success'); alertBox.classList.remove('alert-danger');}else if (alertType=='danger'){alertBox.classList.remove('alert-success'); alertBox.classList.add('alert-danger');}else{alertBox.classList.remove('alert-danger'); alertBox.classList.remove('alert-success'); alertBox.classList.add('alert-primary');}}function togglePowerState(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); var response=JSON.parse(xhttp.responseText); setPowerButton(response['state']); toggleAnimation();}}; toggleAnimation(); xhttp.open('POST', '/togglepower'); xhttp.send();}function setPowerButton(state){if (state=='on'){powerToggle.classList.add('power-on'); powerToggle.classList.remove('power-off'); displayAlert('power turned on', 'success');}else{powerToggle.classList.remove('power-on'); powerToggle.classList.add('power-off'); displayAlert('power turned off', 'success');}}updateWeatherInformation(); </script></body></html>";
  server.send(200, "text/html", index);
}
void handleLocation() {
  Serial.println("request for /changelocation recieved");
  String new_location = server.arg("location");
  Serial.print("new location: ");
  Serial.println(new_location);
  location = new_location;
  doAPICall();
  determineBehaviour();
  sendWeather();
}
void handleNotFound() {
  server.send(200, "text/html", "not found");
}
void sendWeather() {
  String weatherjson = "{";
  if (api_cod == "200") {
    weatherjson += "\"location\":\"" + location + "\",";
    weatherjson += "\"weather\":\"" + weatherDescription + "\",";
    weatherjson += "\"icon\":\"" + icon + "\",";
    weatherjson += "\"temperature\":\"" + tempStr + "\",";
    weatherjson += "\"windSpeed\":\"" + windSpeed + "\",";
  } else {
    weatherjson += "\"cod\":\"" + api_cod + "\",";
    weatherjson += "\"message\":\"" + api_message + "\",";
  }
  if (power_on) {
    weatherjson += "\"state\":\"on\"";
  } else {
    weatherjson += "\"state\":\"off\"";
  }

  weatherjson +=  "}";
  server.send(200, "text/html", weatherjson);
}

void changeBehaviour() {
  String message;
  behaviourLED = server.arg("new_behaviour");
  Serial.print("change behaviour to: ");
  Serial.println(behaviourLED);
  Serial.println();
  message = "{\"cod\":\"200\"}";
  server.send(200, "text/html", message);
}

void togglePower() {
  String message;
  String state;
  if (power_on) {
    power_on = false;
    state = "off";
    Serial.println("power turned off");
  } else {
    power_on = true;
    state = "on";
    Serial.println("power turned on");
  }
  message = "{\"state\":\"" + state + "\"}";
  server.send(200, "text/html", message);
}

/*
   functins for LED
*/
String determineBehaviour() {
  Serial.print("LED behaviour: ");
  if (weatherMain == "Rain" || weatherMain == "Drizzle" || weatherMain == "Snow") {
    Serial.println("rain");
    behaviourLED = "rain";
  } else if (weatherDescription == "clear sky" || weatherDescription == "few clouds") {
    Serial.println("clear sky");
    behaviourLED = "clear sky";
  } else if (weatherMain == "Clouds") {
    Serial.println("clouds");
    behaviourLED = "clouds";
  } else if (weatherMain == "Thunderstorm") {
    Serial.println("thunderstorm");
    behaviourLED = "thunderstorm";
  } else {
    Serial.print("error");
    behaviourLED = "error";
  }
}

/*
   loop
*/
void loop() {
  server.handleClient();
  if (power_on) {
    displayLED();
  }

}

void displayLED() {
Ring1.Update();

  if (behaviourLED == "thunderstorm" && Ring1.loopA == 0) {
    Ring1.loopA++;
    Ring1.loopB = 0;
    Ring1.loopC = 0;
    Ring1.loopD = 0;
    //setup();
    Ring1.TheaterChase(Ring1.Color(255, 255, 0), Ring1.Color(0, 0, 50), 100, FORWARD);
  } else if (behaviourLED == "rain" && Ring1.loopB == 0) {
    Ring1.loopB++;
      Ring1.loopA =0;
      Ring1.loopC = 0;
      Ring1.loopD = 0;
      Ring1.Fade(Ring1.Color(0,255,244), Ring1.Color(0,0,255), 10, 100, FORWARD);
  } else if (behaviourLED == "clear sky" && Ring1.loopC == 0) {
    Ring1.loopC++;
      Ring1.loopA = 0;
      Ring1.loopB = 0;
      Ring1.loopD = 0;
      Ring1.Scanner(Ring1.Color(187, 243, 52), 100);
  } else if (behaviourLED == "clear sky" && Ring1.loopD == 0) {
    
  }
}
// behaviour for light rain
void doLightRain() {

}

void doClearSky() {

}
void doHeavyRain() {

}
void doThunderstorm() {

}
