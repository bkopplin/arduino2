#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

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

// variables for LED to function
String behaviourLED;

ESP8266WebServer server(80);

// variables for RGB
#include <Adafruit_NeoPixel.h>
#define PIN 13
Adafruit_NeoPixel ring = Adafruit_NeoPixel(160, PIN, NEO_GRB + NEO_KHZ800); 


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

  doAPICall();

  /*
       set up web server
  */
  server.on("/", handleRoot);
  server.on("/weather", sendWeather);
  server.on("/change_behaviour", changeBehaviour);
  server.onNotFound(handleNotFound);
  server.on("/changelocation", handleLocation);
  server.begin();

  //setup for RGB
  ring.begin();
  ring.setBrightness(200);
  ring.show();
  setColour(ring.Color(255, 255, 0));

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
  determineBehaviour();
}

/*
    define functions for web server
*/
void handleRoot() {
  Serial.println("request for / recieved");
  String index = "<!DOCTYPE html><html><head> <meta charset='utf-8'/> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <title>Page Title</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'> <script src='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js' integrity='sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy' crossorigin='anonymous'></script> <style>#newLocation, #btnSubmit{/* display:none; */}/* the animation if modified from https://www.w3schools.com/howto/howto_css_loader.asp */ .loader{top: 50px; display:none; border: 10px solid #858585; border-radius: 50%; border-top: 10px solid #1b00b6; width: 120px; height: 120px; -webkit-animation: spin 1s linear infinite; /* Safari */ animation: spin 1s linear infinite; position: relative; margin: 0 auto;}#loader-container{background-color: #d9d9d9; display: none;}/* Safari */@-webkit-keyframes spin{0%{-webkit-transform: rotate(0deg);}100%{-webkit-transform: rotate(360deg);}}@keyframes spin{0%{transform: rotate(0deg);}100%{transform: rotate(360deg);}}.container{max-width: 670px;}#ptemperature{font-size: 40px;}#pdescription, #icon{display: inline-block;}#btnSubmit:hover{cursor: pointer;}#newLocation{text-align: center}</style></head><body> <div id='loader-container' style='position: fixed; width: 100%; height: 100%'> <div class='loader' id='loader' style='display:none'></div></div><nav class='navbar navbar-expand-lg navbar-light bg-light'> <div class='container'> <a class='navbar-brand' href='#'>myCactus</a> <div class='navbar' id='navbarNav'> <ul class='navbar-nav'> <li class='nav-item '> <a class='nav-link' href='#'>192.168.137.132</a> </li></ul> </div></div></nav> <div class='container' id='content'> <div class='row'> <div class='col-12'> <div class='custom-control custom-switch'> <input type='checkbox' class='custom-control-input' id='customSwitch1'> <label class='custom-control-label' for='customSwitch1'>off</label> </div><div class='input-group mb-3 mt-5'> <div class='input-group-prepend'> <span class='input-group-text' id='basic-addon1'>@</span> </div><input type='text' class='form-control form-control-lg' placeholder='type in your location' id='newLocation'> <div class='input-group-append' id='btnSubmit'> <span class='input-group-text'>change</span> </div></div><h1 id='plocation'></h1> <div><span id='ptemperature'>19</span><span> °C</span></div><img id='icon' src='http://openweathermap.org/img/w/10d.png' alt='image weather' style='display: inline-block'> <p id='pdescription'>light rain</p><p id='pwindspeed'></p><hr> <h3>change behaviour</h3> <div class='input-group'> <select class='custom-select' id='selectBehaviour'> <option value='clear sky'>clear sky</option> <option value='rain'>rain</option> <option value='heavy rain'>heavy rain</option> <option value='thunderstorm'>thunderstorm</option> <option value='cloudy'>cloudy</option> </select> <div class='input-group-append' id='btnChangeBehaviour'> <label class='input-group-text'>change</label> </div></div></div></div></div><script>var newLocation, btnSubmit, loader_container, loader, plocation, ptemperature, pdescription, icon, btnChangeBehaviour, selectBehaviour; var host='192.168.137.143'; loader_container=document.getElementById('loader-container'); loader=document.getElementById('loader'); plocation=document.getElementById('plocation'); icon=document.getElementById('icon'); ptemperature=document.getElementById('ptemperature'); pdescription=document.getElementById('pdescription'); pwindspeed=document.getElementById('pwindspeed'); btnChangeWeather=document.getElementById('btnChangeWeather'); newLocation=document.getElementById('newLocation'); btnSubmit=document.getElementById('btnSubmit'); btnChangeBehaviour=document.getElementById('btnChangeBehaviour'); selectBehaviour=document.getElementById('selectBehaviour'); btnSubmit.addEventListener('click', function (){setLocation(newLocation.value);}); btnChangeBehaviour.addEventListener('click', function (){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); var str=JSON.parse(xhttp.responseText); if (str['cod']==200){alert('behaviour successfully changed');}}}; xhttp.open('POST', '/change_behaviour'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('new_behaviour=' + selectBehaviour.value);}); function updateWeatherInformation(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); setWeatherInfo(xhttp.responseText); toggleAnimation();}}; toggleAnimation(); xhttp.open('GET', '/changelocation'); xhttp.send();}function setLocation(location){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); setWeatherInfo(xhttp.responseText); toggleAnimation();}}; toggleAnimation(); xhttp.open('POST', '/changelocation'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('location=' + location);}function setWeatherInfo(jsonWeatherdata){var weatherData=JSON.parse(jsonWeatherdata); if (weatherData['cod']=='404'){alert(weatherData['message']);}else{newLocation.value=weatherData['location']; ptemperature.innerHTML=weatherData['temperature']; pdescription.innerHTML=weatherData['weather']; icon.src='http://openweathermap.org/img/w/' + weatherData['icon'] + '.png'; pwindspeed.innerHTML=weatherData['windSpeed'] + ' km/h';}}function toggleAnimation(){var display=loader.style.display; if (display=='none'){loader_container.style.display='block'; loader.style.display='block'; document.getElementById('content').style.opacity=0.5;}else{loader_container.style.display='none'; loader.style.display='none'; document.getElementById('content').style.opacity=1;}}// setLocation('Dundee'); updateWeatherInformation(); </script></body></html>";
  server.send(200, "text/html", index);
}
void handleLocation() {
  Serial.println("request for /changelocation recieved");
  String new_location = server.arg("location");
  Serial.print("new location: ");
  Serial.println(new_location);
  location = new_location;
  doAPICall();
  // server.send(200, "text/html", "recieved");
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
    weatherjson += "\"windSpeed\":\"" + windSpeed + "\"";
  } else {
    weatherjson += "\"cod\":\"" + api_cod + "\",";
    weatherjson += "\"message\":\"" + api_message + "\"";
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

/*
   functins for LED
*/
String determineBehaviour() {
  Serial.print("LED behaviour: ");
  if (weatherDescription == "moderate rain" || weatherDescription == "light rain" || weatherDescription == "light intensity shower rain" || weatherDescription == "shower rain") {
    Serial.println("rain");
    behaviourLED = "rain";
  } else if (weatherMain == "Rain"  ) {
    Serial.println("heavy rain");
    behaviourLED = "heavy rain";
  } else if (weatherDescription == "clear sky" || weatherDescription == "few clouds") {
    Serial.println("clear sky");
    behaviourLED = "clear sky";
  } else if (weatherMain == "Clouds") {
    Serial.println("cloudy");
    behaviourLED = "cloudy";
  } else if (weatherMain == "Thunderstorm") {
    Serial.println("thunderstorm");
    behaviourLED = "thunderstorm";
  }
}

/*
   loop
*/
void loop() {
  server.handleClient();
  displayLED();
}

void displayLED() {

  if (behaviourLED == "light rain") {
    lightRain();
    }
  else if (behaviourLED == "heavy rain") {
    heavyRain();
    }
  else if (behaviourLED == "clear sky") {
    clearSky();
    }
  else if (behaviourLED == "cloudy") {
    cloudy();
    }
  else if (behaviourLED == "thunderstorm") {
    thunderstorm();
    }     
}
// behaviour for light rain
void lightRain() {
  // short short short
  setColour(ring.Color(0,0,255));
  onShort();
  onShort();
  onShort();
  
}

void clearSky() {
  // long short short;
  setColour(ring.Color(127,127,127));
  delay(50);
  setColour(ring.Color(255,178,102));
  delay(50);
}
void heavyRain() {
  setColour(ring.Color(0,255,0));
  //long long long
  onLong();
  onLong();
  onLong();
  
}
void thunderstorm() {
  setColour(ring.Color(127,127,127));
  // short long long
  onShort();
  onLong();
  onLong;
  
}

void cloudy() {
  
}

void onLong() {
  //setColour(ring.Color(red, green, blue)); //on long
  delay(500);
  Serial.println("long");
  setColour(ring.Color(0,0,0)); //off
  delay(50);
}

void onShort() {
  // setColour(ring.Color(red, green, blue)); //on short
  delay(200);
  Serial.println("short");
  setColour(ring.Color(0,0,0)); //off
  delay(50);
}

void setColour(uint32_t c) {
  for(uint16_t i=0; i<ring.numPixels(); i++) {
      ring.setPixelColor(i, c);
      ring.show();
  }
}
