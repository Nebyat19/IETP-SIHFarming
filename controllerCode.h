#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DHT11.h> 

#define pumpPin 4 //D2 Pin connected to the pump relay
#define fanPin 0 //D3 Pin connected to the fan relay
#define LED_PIN 2 //D4 Pin connected to the LED relay

#define  buzzerPin  14 //D5
#define  buttonPin  12 //D6
#define  waterLevelPin 15 //d8
 
 int waterLevelValue=100;

// Variables
bool buzzerActive = false;
unsigned long buzzerStartTime = 0;
const unsigned long buzzerDuration = 5000; 
bool isbuzzerOff=false;

// 10 seconds
DHT11 dht11(5); //
/** Humid And Temp* **/
int MAX_TEMP = 49;
int MAX_HUMID =70;
int MIN_TEMP = 10;
int MIN_HUMID =10;  
int TEMP_DELAY = 10;
int tempValue=30;
int humidValue=30;
bool  isHumidity=true;
unsigned long TempPreviousMillis =0;


/*** FAn ***/
bool fanState = false;
int fanStartDelay = 3000; 
int fanDuration = 2000;
int fanOffTime = 0;
unsigned long fanPreviousMillis = 0; 
unsigned long fanCurrentMillis = 0;

/*******led light***********/

bool ledState = false;
int ledStartDelay = 3000;
int ledDuration = 2000;
int ledOffTime = 0;

unsigned long ledPreviousMillis = 0;
unsigned long ledCurrentMillis = 0;

const char* ssid = "IETP-g03";     // Enter your WiFi name
const char* password = "12345678"; // Enter your WiFi password

ESP8266WebServer server(80);
WebSocketsServer webSocketServer(81);
/*****pump****/
bool pumpState = false;
int pumpStartDelay = 3000; 
int pumpDuration = 2000;
int pumpOfTime = 0;

unsigned long pumpPreviousMillis = 0; 
unsigned long pumpCurrentMillis = 0;

void headers() {
  // Add the necessary CORS headers
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
}
void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Pump Control</h1>";

  if (pumpState) {
    html += "<p>Pump is currently <strong>ON</strong></p>";
  } else {
    html += "<p>Pump is currently <strong>hh</strong></p>";
  }

  html += "<h2>pump Schedule</h2>";
  html += "<form action='/pump/setSchedule' method='POST'>";
  html += "Start delay (minutes): <input type='number' name='startDelay' min='1' value='" + String(pumpStartDelay) + "'><br>";
  html += "Duration (minutes): <input type='number' name='duration' min='1' value='" + String(pumpDuration) + "'><br>";
  html += "<input type='submit' value='Set Schedule'>";
  html += "</form>";
  html += "<h1>Fan Control</h1>";

  if (fanState) {
    html += "<p>Fan is currently <strong>ON</strong></p>";
  } else {
    html += "<p>Fan is currently <strong>OFF</strong></p>";
  }
  html += "<h2>Manual Control</h2>";
  html += "<a href='/pump/on'><button>Turn Pump ON</button></a>&nbsp;";
  html += "<a href='/pump/off'><button>Turn Pump OFF</button></a>";
  
  html += "<h2> fan Schedule</h2>";
  html += "<form action='/fan/setSchedule' method='POST'>";
  html += "Start delay (minutes): <input type='number' name='startDelay' min='1' value='" + String(fanStartDelay) + "'><br>";
  html += "Duration (minutes): <input type='number' name='duration' min='1' value='" + String(fanDuration) + "'><br>";
  html += "<input type='submit' value='Set Schedule'>";
  html += "</form>";

  html += "<h2>Manual Control</h2>";
  html += "<a href='/fan/on'><button>Turn Fan ON</button></a>&nbsp;";
  html += "<a href='/fan/off'><button>Turn Fan OFF</button></a>";

if (ledState) {
html += "<p>LED is currently <strong>ON</strong></p>";
} else {
html += "<p>LED is currently <strong>OFF</strong></p>";
}

html += "<h2>Schedule</h2>";
html += "<form action='/led/setSchedule' method='POST'>";
html += "Start delay (minutes): <input type='number' name='startDelay' min='1' value='" + String(ledStartDelay) + "'><br>";
html += "Duration (minutes): <input type='number' name='duration' min='1' value='" + String(ledDuration) + "'><br>";
html += "<input type='submit' value='Set Schedule'>";
html += "</form>";

html += "<h2>Manual Control</h2>";
html += "<a href='/led/on'><button>Turn LED ON</button></a> ";
html += "<a href='/led/off'><button>Turn LED OFF</button></a>";


  html += "</body></html>";

  server.send(200, "text/html", html);
}

/********************led************/

void handleSetLedSchedule() {
  headers();
if (server.args() == 2) {
ledStartDelay = server.arg("startDelay").toInt();

ledDuration = server.arg("duration").toInt();

ledPreviousMillis = millis(); // Reset the previousMillis when the schedule is set
server.send(200, "text/plain", "Schedule set successfully");
} else {
server.send(400, "text/plain", "Invalid request");
}
}

void ledOn() {
  ledCurrentMillis = millis();
ledOffTime = ledCurrentMillis + ledDuration * 1000;
String statusMessage = "The LED has been turned on";
webSocketServer.broadcastTXT(statusMessage);
ledState = true;
digitalWrite  (LED_PIN, HIGH);
}

void ledOff() {
String statusMessage = "The LED has been turned off";
webSocketServer.broadcastTXT(statusMessage);
ledState = false;
digitalWrite(LED_PIN, LOW);
}


void handleLedOn() {
  headers();
ledOn();
server.sendHeader("Location", "/");
server.send(302, "text/plain", "LED turned ON");
}

void handleLedOff() {
  headers();
ledOff();
server.sendHeader("Location", "/");
server.send(302, "text/plain", "LED turned OFF");
}

void handleGetLedStatus() {
  headers();
String status = (ledState) ? "ON" : "OFF";
server.send(200, "text/plain",   status);
}

void handleGetLedSchedule() {
  headers();
String schedule =  String(ledStartDelay) + " /";
schedule += String(ledDuration);
server.send(200, "text/plain", schedule);
}




/***********fan**************/

void handleSetFanSchedule() {
  headers();
  if (server.args() == 2) {
    fanStartDelay = server.arg("startDelay").toInt();
    
    fanDuration = server.arg("duration").toInt();
   
    fanPreviousMillis = millis(); // Reset the previousMillis when the schedule is set
    server.send(200, "text/plain", "Schedule set successfully");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}

void fanOn() {
   fanCurrentMillis = millis();
   fanOffTime = fanCurrentMillis + fanDuration * 1000;
  String statusMessage = "The fan has been turned on";
  webSocketServer.broadcastTXT(statusMessage);
  fanState = true;
  digitalWrite(fanPin, HIGH);
}

void fanOff() {
  String statusMessage = "The fan has been turned off";
  webSocketServer.broadcastTXT(statusMessage);
  fanState = false;
  digitalWrite(fanPin, LOW);
}

void handleFanOn() {
  headers();
  fanOn();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "ON");
}

void handleFanOff() {
  headers();
  fanOff();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "OFF");
}

void handleGetFanStatus() {
   headers();
  String status = (fanState) ? "ON" : "OFF";
  server.send(200, "text/plain", "Fan is " + status);
}

void handleGetFanSchedule() {
  String schedule = String(fanStartDelay) + " /";
  schedule += String(fanDuration);
  server.send(200, "text/plain", schedule);
}
//END FAN

//fan
void handleSetPumpSchedule() {
  headers();
  if (server.args() == 2) {
    pumpStartDelay = server.arg("startDelay").toInt();
   
    pumpDuration = server.arg("duration").toInt();
   
    pumpPreviousMillis = millis(); // Reset the previousMillis when the schedule is set
    server.send(200, "text/plain", "Schedule set successfully");
  } else {
    server.send(400, "text/plain", "Invalid request");
  }
}
void pumpOn(){
    pumpCurrentMillis = millis();
   pumpOfTime=pumpCurrentMillis + pumpDuration *1000;

   String statusMessage="The pump has been turned on";
   webSocketServer.broadcastTXT(statusMessage);
   pumpState = true;
  digitalWrite(pumpPin, HIGH);
}
void pumpOff(){
   String statusMessage="The pump has been turned off";
   webSocketServer.broadcastTXT(statusMessage);
   pumpState = false;
  digitalWrite(pumpPin, LOW);
  }
void handlePumpOn() {
  headers();
   pumpCurrentMillis = millis();
   pumpOfTime=pumpCurrentMillis + pumpDuration *1000;
   
  pumpState = true;  
  digitalWrite(pumpPin, HIGH);
  server.sendHeader("Location", "/");
  server.send(200, "text/plain", "Pump turned ON");
}

void handlePumpOff() {
  headers();
  pumpState = false;
  digitalWrite(pumpPin, LOW);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Pump turned OFF");
}

void handleGetPumpStatus() {
  headers();
  String status = (pumpState) ? "ON" : "OFF";
  server.send(200, "text/plain", "Pump is " + status);
}

void handleGetPumpSchedule() {
  headers();
  String schedule = String(pumpStartDelay) + "/";
  schedule += String(pumpDuration);
  server.send(200, "text/plain", schedule);
}

void setup() {
 
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
   pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
   pinMode(waterLevelPin, INPUT); 
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
//ROUTE FOR PUMP
  server.on("/", handleRoot);
  server.on("/pump/setSchedule", handleSetPumpSchedule);
  server.on("/pump/on", handlePumpOn);
  server.on("/pump/off", handlePumpOff);
  server.on("/pump/getStatus", handleGetPumpStatus);
  server.on("/pump/getSchedule", handleGetPumpSchedule);
//ROUTE FOR tempHumid
 server.on("/tempHumid", handleTempAndHumid);
//ROUTE FOR FAN
  server.on("/fan/setSchedule", handleSetFanSchedule);
  server.on("/fan/on", handleFanOn);
  server.on("/fan/off", handleFanOff);
  server.on("/fan/getStatus", handleGetFanStatus);
  server.on("/fan/getSchedule", handleGetFanSchedule);

//ROUTE FOR LED

server.on("/led/setSchedule", handleSetLedSchedule);
server.on("/led/on", handleLedOn);
server.on("/led/off", handleLedOff);
server.on("/led/getStatus", handleGetLedStatus);
server.on("/led/getSchedule", handleGetLedSchedule);

//stopBuzzer
server.on("/buzzer/off", handleStopBuzzer); //stopBuzzer
server.on("/buzzer/on", handleStartBuzzer); //startBuzzer

//TURN ALL OFF
server.on("/turnAll/on", turnAllOn); //turnAllOn
server.on("/turnAll/off", turnAllOff); //turnAllOff
server.on("/Handleisbuzzer/on", HandleisbuzzerOff); // HandleisbuzzerOff
server.on("/Handleisbuzzer/off", HandleisbuzzerOff);

//Water Levek
server.on("/waterLevel", handlewaterLevel); //waterLevel

  /*
   * handdle buzzer
   */
  server.begin();
  webSocketServer.begin();
  
  Serial.println("HTTP server started");
}

void PumpController(){
    pumpCurrentMillis = millis();

  // Check if the delay period has passed and it's time to activate the pump
  if (pumpCurrentMillis - pumpPreviousMillis >= pumpStartDelay * 1000) {
    if (!pumpState) {
     
      pumpState = true;
      pumpPreviousMillis = pumpCurrentMillis;
      pumpOfTime=pumpCurrentMillis + pumpDuration *1000;
      digitalWrite(pumpPin, HIGH);
    } 
  }
  if(pumpCurrentMillis >= pumpOfTime && pumpOfTime>0){
     pumpState = false;
     digitalWrite(pumpPin, LOW);
  }
}
void ledController() {
ledCurrentMillis = millis();

// Check if the delay period has passed and it's time to activate the LED
if (ledCurrentMillis - ledPreviousMillis >= ledStartDelay * 1000) {
if (!ledState) {
ledState = true;
ledPreviousMillis = ledCurrentMillis;
ledOffTime = ledCurrentMillis + ledDuration * 1000;
digitalWrite(LED_PIN, HIGH);
}
}

if (ledCurrentMillis >= ledOffTime) {
ledState = false;
digitalWrite(LED_PIN, LOW);
}
}
void fanController() {
  fanCurrentMillis = millis();

  // Check if the delay period has passed and it's time to activate the fan
  if (fanCurrentMillis - fanPreviousMillis >= fanStartDelay * 1000) {
    if (!fanState) {
      fanState = true;
      fanPreviousMillis = fanCurrentMillis;
fanOffTime = fanCurrentMillis + fanDuration * 1000;
      digitalWrite(fanPin, HIGH);
    } 
  }
  
  if (fanCurrentMillis >= fanOffTime && fanOffTime>0) {
     fanState = false;
     digitalWrite(fanPin, LOW);
  }
}

/***** temprature and humidity ****/

void handleTempAndHumid(){
headers();
   String tempAndHumid =  String(tempValue) + "/"+String(humidValue);

  server.send(200, "text/plain", tempAndHumid);
}
void ReadTmpAndHumidity(){

  if ( millis() - TempPreviousMillis >= TEMP_DELAY * 1000) {
    TempPreviousMillis=millis();
  if(isHumidity){
    isHumidity=false;
     int humidity = dht11.readHumidity();
      if (humidity != DHT11::ERROR_CHECKSUM && humidity != DHT11::ERROR_TIMEOUT)
    {
      humidValue=humidity;
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    }
    }else{
      isHumidity=true;
       int temperature = dht11.readTemperature();
         if (temperature != DHT11::ERROR_CHECKSUM && temperature != DHT11::ERROR_TIMEOUT)
    {
      tempValue=temperature;
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C");
    }
    }
   
    
   
    if(tempValue >= MAX_TEMP){
      /*
       *  send notifcation and turn on fan
       */
   String   temperature_status_message = "Temperature has exceeded the maximum threshold.";
   webSocketServer.broadcastTXT(temperature_status_message);
  fanOn();
  startBuzzer();

    }else if (tempValue <= MIN_TEMP){
String       temperature_status_message  = "Temperature has fallen below the minimum threshold.";
   webSocketServer.broadcastTXT(temperature_status_message);
   startBuzzer();
       fanOff();
    }
      if(humidValue >= MAX_HUMID){
      String  humidity_status_message = "Humidity has exceeded the maximum threshold.";
          webSocketServer.broadcastTXT(humidity_status_message);
          startBuzzer();
      
    }else if (humidValue <= MIN_HUMID){
        String humidity_status_message = "Humidity has fallen below the minimum threshold.";
          webSocketServer.broadcastTXT(humidity_status_message);
          startBuzzer();
    }
  }
  }

void buzzerController() {
  // Check if the buzzer should stop
  if (buzzerActive && (millis() - buzzerStartTime >= buzzerDuration)) {
    stopBuzzer();
  }

  // Check if the button is pressed
  if (digitalRead(buttonPin) == HIGH) {
 
  stopBuzzer();
     
  //   delay(1000);
  }

  // Buzzer control logic
  if (buzzerActive&&!isbuzzerOff) {
    digitalWrite(buzzerPin, HIGH);  // Turn on the buzzer
  } else {
    digitalWrite(buzzerPin, LOW);   // Turn off the buzzer
  }
}

void HandleisbuzzerOff(){
  isbuzzerOff=false;
   server.send(302, "text/plain", "isbuzzerOff false");
  }
// Function to stop the buzzer
void handleStopBuzzer(){
  stopBuzzer();
  server.send(302, "text/plain", "Buzzer OFF");
}
void handleStartBuzzer(){
  startBuzzer();
  server.send(302, "text/plain", "Buzzer ON");
}
void stopBuzzer() {
  isbuzzerOff=true;
  buzzerActive = false;
 //buzzerStartTime = millis() + 6000;
}

// Function to start the buzzer
void startBuzzer() {
 buzzerStartTime= millis();
  buzzerActive = true;
  
}
//TURN ALL ON

void turnAllOn(){
  ledOn();
  fanOn();
  pumpOn();
   server.send(302, "text/plain", "TEST ON");
}
void turnAllOff(){
  
  fanStartDelay=3000;
  fanDuration=2000;
  ledStartDelay=3000;
  ledDuration=2000;
  pumpStartDelay=3000;
  pumpDuration=2000;
   webSocketServer.broadcastTXT("schedule has been reset!");
  ledOff();
  fanOff();
  pumpOff();
   server.send(302, "text/plain", "TEST OFF");
  }

  void handlewaterLevel(){
    headers();
   server.send(200, "text/plain", String(waterLevelValue));
  }
  void waterLevelController() {
  int waterLevel = digitalRead(waterLevelPin);  // Read the water level pin

  if (waterLevel == LOW) {
  
    waterLevelValue=0;
   
     webSocketServer.broadcastTXT("Water level is low!");
        startBuzzer();
    
  }else{
      waterLevelValue=100;
      
  }

delay(50);
}
  
void loop() {
   ReadTmpAndHumidity();
  server.handleClient();
  webSocketServer.loop();
 
  
   PumpController();
   fanController();
   ledController();
   buzzerController();
  waterLevelController();
   
}
