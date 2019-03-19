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
bool power_on = true;

// variables for LED to function
String behaviourLED;

ESP8266WebServer server(80);

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
  String index = "<!DOCTYPE html><html><head> <meta charset='utf-8'/> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <title>Page Title</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'> <script src='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js' integrity='sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy' crossorigin='anonymous'></script> <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'> <style>:root{--main-color: #52928b; /* --secondary-color: #004d47; */ --secondary-color: #b9c4c7;}/* the animation if modified from https://www.w3schools.com/howto/howto_css_loader.asp */ .loader{top: 50px; display:none; border: 10px solid var(--main-color); border-radius: 50%; border-top: 10px solid var(--secondary-color); width: 120px; height: 120px; -webkit-animation: spin 1s linear infinite; /* Safari */ animation: spin 1s linear infinite; position: relative; margin: 0 auto;}@keyframes spin{0%{transform: rotate(0deg);}100%{transform: rotate(360deg);}}#loader-container{background-color: #b9b9b9; opacity: 0.9; display: none; z-index: 200;}.secondary-bg-color{background-color: var(--secondary-color);}.container{max-width: 670px;}#ptemperature{font-size: 40px;}#pdescription, #icon{display: inline-block;}#btnSubmit:hover, #btnChangeBehaviour:hover, #powerToggle:hover{cursor: pointer; opacity: 0.8; transition: opacity 1s;}#btnSubmit, #btnChangeBehaviour, #powerToggle{transition: opacity 1s;}#btnChangeBehaviour .input-group-text:hover{cursor: pointer;}#newLocation{text-align: left;}.input-group-append .input-group-text{background-color: var(--main-color); color: white;}.input-group-prepend .input-group-text{background-color: var(--secondary-color); color: white;}.navbar{background-color: var(--main-color);}.navbar a{color: white;}.power-on{color: #ffffff;}.power-off{color: black;}</style></head><body> <div id='loader-container' style='position: fixed; width: 100%; height: 100%'> <div class='loader' id='loader' style='display:none'></div></div><nav class='navbar navbar-expand-lg '> <div class='container'> <a class='navbar-brand' href='#'>myCactus</a> <div class='navbar' id='navbarNav'> <ul class='navbar-nav'> <li class='nav-item '> <span class='nav-link power-on' id='powerToggle'><i class='fa fa-power-off '></i></span> </li></ul> </div></div></nav> <div class='container' id='content'> <div class='row'> <div class='col-12'> <div id='alertBox' class='alert alert-success mt-4' role='alert' style='visibility: visible'> </div><h4>change location</h4> <div class='input-group mb-3 mt-2 input-group-lg'> <div class='input-group-prepend'> <span class='input-group-text' id='basic-addon1'><i class='fa fa-map-marker-alt '></i></span> </div><input type='text' class='form-control' placeholder='type in your location' id='newLocation'> <div class='input-group-append' id='btnSubmit'> <span class='input-group-text'><i class='fa fa-search '></i></span> </div></div><h1 id='plocation'></h1> <div><span id='ptemperature'></span><span> °C</span></div><img id='icon' src='http://openweathermap.org/img/w/10d.png' alt='image weather' style='display: inline-block'> <p id='pdescription'></p><hr> <h4>change behaviour</h4> <div class='input-group input-group-lg mb-3'> <div class='input-group-prepend'> <span class='input-group-text' id='basic-addon1'><i class='fa fa-lightbulb '></i></span> </div><select class='custom-select form-control' id='selectBehaviour'> <option value='clear sky'>clear sky</option> <option value='rain'>rain</option> <option value='thunderstorm'>thunderstorm</option> <option value='cloudy'>cloudy</option> </select> <div class='input-group-append' id='btnChangeBehaviour'> <label class='input-group-text'><i class='fa fa-arrow-right '></i></label> </div></div></div></div></div><script>var newLocation, btnSubmit, loader_container, loader, plocation, ptemperature, pdescription, icon, btnChangeBehaviour, selectBehaviour, alertBox, powerToggle; alertBox=document.getElementById('alertBox'); powerToggle=document.getElementById('powerToggle'); loader_container=document.getElementById('loader-container'); loader=document.getElementById('loader'); plocation=document.getElementById('plocation'); icon=document.getElementById('icon'); ptemperature=document.getElementById('ptemperature'); pdescription=document.getElementById('pdescription'); btnChangeWeather=document.getElementById('btnChangeWeather'); newLocation=document.getElementById('newLocation'); btnSubmit=document.getElementById('btnSubmit'); btnChangeBehaviour=document.getElementById('btnChangeBehaviour'); selectBehaviour=document.getElementById('selectBehaviour'); btnSubmit.addEventListener('click', function (){setLocation(newLocation.value);}); newLocation.addEventListener('keypress', function (e){if (e.keyCode==13){setLocation(newLocation.value);}}); powerToggle.addEventListener('click', function(){togglePowerState();}); btnChangeBehaviour.addEventListener('click', function (){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); var str=JSON.parse(xhttp.responseText); if (str['cod']==200){toggleAnimation(); displayAlert('behaviour successfully changed', 'success');}}}; toggleAnimation(); xhttp.open('POST', '/change_behaviour'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('new_behaviour=' + selectBehaviour.value);}); function updateWeatherInformation(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); setWeatherInfo(xhttp.responseText); toggleAnimation();}}; toggleAnimation(); xhttp.open('GET', '/weather'); xhttp.send();}function setLocation(location){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); setWeatherInfo(xhttp.responseText); toggleAnimation();}}; toggleAnimation(); xhttp.open('POST', '/changelocation'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('location=' + location);}function setWeatherInfo(jsonWeatherdata){var weatherData=JSON.parse(jsonWeatherdata); if (weatherData['cod']=='404'){displayAlert(weatherData['message'], 'danger');}else{setPowerButton(weatherData['state']); newLocation.value=weatherData['location']; ptemperature.innerHTML=weatherData['temperature']; pdescription.innerHTML=weatherData['weather']; icon.src='http://openweathermap.org/img/w/' + weatherData['icon'] + '.png'; displayAlert('weather updated', 'success');}}function toggleAnimation(){var display=loader.style.display; if (display=='none'){loader_container.style.display='block'; loader.style.display='block';}else{loader_container.style.display='none'; loader.style.display='none';}}function displayAlert(message, alertType){alertBox.innerHTML=message; alertBox.style.visibillity='visible'; if (alertType=='success'){alertBox.classList.add('alert-success'); alertBox.classList.remove('alert-danger');}else if (alertType=='danger'){alertBox.classList.remove('alert-success'); alertBox.classList.add('alert-danger');}else{alertBox.classList.remove('alert-danger'); alertBox.classList.remove('alert-success'); alertBox.classList.add('alert-primary');}}function togglePowerState(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function (){if (xhttp.readyState==4 && xhttp.status==200){console.log(xhttp.responseText); var response=JSON.parse(xhttp.responseText); setPowerButton(response['state']); toggleAnimation();}}; toggleAnimation(); xhttp.open('POST', '/togglepower'); xhttp.send();}function setPowerButton(state){if (state=='on'){powerToggle.classList.add('power-on'); powerToggle.classList.remove('power-off'); displayAlert('power turned on');}else{powerToggle.classList.remove('power-on'); powerToggle.classList.add('power-off'); displayAlert('power turned off');}} updateWeatherInformation(); </script></body></html>";
  server.send(200, "text/html", index);
}
void handleLocation() {
  Serial.println("request for /changelocation recieved");
  String new_location = server.arg("location");
  Serial.print("new location: ");
  Serial.println(new_location);
  location = new_location;
  doAPICall();
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
  if(power_on) {
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
  if(power_on) {
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
    Serial.println("clear");
    behaviourLED = "clear";
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
