#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "SevenSegmentTM1637.h"
 
const char *ssid = "Adam's Puzzlebox";
const char *password = "laplacier";

const byte PIN_CLK = 19;   // define CLK pin (any digital pin)
const byte PIN_DIO = 23;   // define DIO pin (any digital pin)
SevenSegmentTM1637    display(PIN_CLK, PIN_DIO);

// Set LED/Relay GPIO
const int ledPin = 22;
// Stores LED state
String ledState;

// Flags for checking states
bool open_flag = 0;
bool on_flag = 0;
bool off_flag = 0;
 
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "OFF";
    }
    else{
      ledState = "ON";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}

void setup(){
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  display.begin();            // initializes the display
  display.setBacklight(100);  // set the brightness to 100 %

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
 
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
 
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    open_flag = 1;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to open the puzzle box
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);
    display.print("OPEN");
    open_flag = 1;
    on_flag = 1;
    off_flag = 0;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to lock the puzzle box
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH); 
    display.print("LOCK");
    open_flag = 1;
    on_flag = 0;
    off_flag = 1;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
 
  server.begin();
}
 
void loop(){
  //Print IP until page request
  while(!open_flag){
    display.print("    192.168.4.1    ");
  }
  if(on_flag && !off_flag){
    display.print("OPEN");
    on_flag = 0;
  }
  if(!on_flag && off_flag){
    display.print("LOCK");
    off_flag = 0;
  }
}
