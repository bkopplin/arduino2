#include <ESP8266WiFi.h>
#include <WiFiClient.h>

char* ssid = "bjarne_pc";
char* pass = "passwort";
String website = "<!DOCTYPE html><html><head> <meta charset='utf-8'/> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <title>Page Title</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'><script src='https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js' integrity='sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy' crossorigin='anonymous'></script><style>#newLocation, #btnCancel, #btnSubmit{/* display:none; */}</style></head><body> <div class='container'> <div class='row'> <div class='jumbotron col-12'> <h1 id='location'>Dundee</h1> <img id='icon' src='http://openweathermap.org/img/w/10d.png' alt='image weather'> <p id='temperature'>19C</p><p id='description'>light rain</p><button id='btnChange'>change location</button> <input type='text' name='newLocation' id='newLocation'> <button id='btnCancel'>cancel</button> <button id='btnSubmit'>submit change</button> </div></div></div><script>var btnChange, newLocation, btnCancel, btnSubmit; var host='192.168.137.143'; btnChange=document.getElementById('btnChange'); newLocation=document.getElementById('newLocation'); btnCancel=document.getElementById('btnCancel'); btnSubmit=document.getElementById('btnSubmit'); btnChange.addEventListener('click', function(){btnChange.style.display='none'; newLocation.style.display='inline-block'; btnCancel.style.display='inline-block'; btnSubmit.style.display='inline-block';}); btnCancel.addEventListener('click', function(){newLocation.value=''; btnChange.style.display='inline-block'; newLocation.style.display='none'; btnCancel.style.display='none'; btnSubmit.style.display='none';}); btnSubmit.addEventListener('click', function(){setLocation();}); function setLocation(){var xhttp=new XMLHttpRequest(); xhttp.onreadystatechange=function(){if(xhttp.readyState==4 && xhttp.status==200){alert('response');}}; xhttp.open('POST', '/changeLocation'); xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded'); xhttp.send('location=london'); alert('h');}</script></body></html>";
WiFiServer server = 80;

void setup() {
  Serial.begin(115200);
  Serial.println("trying to connect to Wifi");

  WiFi.begin(ssid, pass);
  delay(100);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  server.begin();
  Serial.println(WiFi.localIP());

}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    boolean onFirstLine = true;
    String currentLine;
    char c;
    // read a character
    while (client.connected()) {
    

      if (client.available()) {
        c = client.read();
      }
      currentLine += c;
      // check if line ends
      if ( c == '\n') {
        // get information from first line
        if (onFirstLine) {
          onFirstLine = false;
          char* firstLine = strtok(currentLine, " ");
          Serial.println(firstLine[0]);
        }
        Serial.println(currentLine);
        currentLine = "";
      } else {
        
      }
      
      if (onFirstLine && c == '\n')
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println(website);
          break;
        }
      if (c == '\n') {
        // you're starting a new line
        currentLineIsBlank = true;
      } else if (c != '\r') {
        // you've gotten a character on the current line
        currentLineIsBlank = false;
      }

    }
   
    
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  //  WiFiClient client = server.available();
  //  if (client) {
  //    boolean currentLineBlank = true;
  //    Serial.println("new client");
  //    while (client.connected()) {
  //      if (client.available()) {
  //        char c = client.read();
  //        Serial.write(c);
  //
  //        // check if at end of request. Send the reply
  //        if (c == '\n' && currentLineBlank) {
  //          client.printf("HTTP/1.1 200 OK\r\n\
  //        Content-Type: text/html\r\n\
  //        Connection: close\r\n\
  //        \r\n\
  //        <!DOCTYPE HTML>\
  //        <html>\
  //        <p>you are online</p>\
  //        </html>\
  //        \r\n\
  //        \r\n\
  //        ");
  //        } // end if
  //        if (c == '\n') {
  //          currentLineBlank = true;
  //        } else if (c != '\r') {
  //          currentLineBlank = false;
  //        }
  //
  //      }
  //    }
  //
  //    delay(1);
  //    client.stop();
  //  }
}
