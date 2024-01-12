#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <DHT11.h>

#define pumpPin 4 // Pin connected to the pump relay
#define fanPin 0 // Pin connected to the fan relay

DHT11 dht11(4); //
/** Humid And Temp* **/
int MAX_TEMP = 49;
int MAX_HUMID =60;
int MIN_TEMP = 10;
int MIN_HUMID =10;
int TEMP_DELAY = 5;
int tempValue=0;
int humidValue=0;
bool  isHumidity=true;
unsigned long TempPreviousMillis =0;


/*** FAn ***/
bool fanState = false;
int fanStartDelay = 30; 
int fanDuration = 2;
int fanOffTime = 0;
unsigned long fanPreviousMillis = 0; 
unsigned long fanCurrentMillis = 0;


const char* ssid = "my empire";     // Enter your WiFi name
const char* password = "code5515"; // Enter your WiFi password

ESP8266WebServer server(80);
WebSocketsServer webSocketServer(81);
/*****pump****/
bool pumpState = false;
int pumpStartDelay = 30; 
int pumpDuration = 2;
int pumpOfTime = 0;

unsigned long pumpPreviousMillis = 0; 
unsigned long pumpCurrentMillis = 0;


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


  html += "</body></html>";

  server.send(200, "text/html", html);
}



/***********fan**************/

void handleSetFanSchedule() {
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
   fanOffTime = fanCurrentMillis + fanDuration * 60000;
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
  fanOn();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Fan turned ON");
}

void handleFanOff() {
  fanOff();
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Fan turned OFF");
}

void handleGetFanStatus() {
  String status = (fanState) ? "ON" : "OFF";
  server.send(200, "text/plain", "Fan is " + status);
}

void handleGetFanSchedule() {
  String schedule = "Start delay: " + String(fanStartDelay) + " minutes\n";
  schedule += "Duration: " + String(fanDuration) + " minutes";
  server.send(200, "text/plain", schedule);
}
//END FAN

//fan
void handleSetPumpSchedule() {
  
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
   pumpOfTime=pumpCurrentMillis + pumpDuration *60000;

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
   pumpCurrentMillis = millis();
   pumpOfTime=pumpCurrentMillis + pumpDuration *60000;
   
  pumpState = true;  
  digitalWrite(pumpPin, HIGH);
  server.sendHeader("Location", "/");
  server.send(200, "text/plain", "Pump turned ON");
}

void handlePumpOff() {
  pumpState = false;
  digitalWrite(pumpPin, LOW);
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Pump turned OFF");
}

void handleGetPumpStatus() {
  String status = (pumpState) ? "ON" : "OFF";
  server.send(200, "text/plain", "Pump is " + status);
}

void handleGetPumpSchedule() {
  String schedule = "Start delay: " + String(pumpStartDelay) + " minutes\n";
  schedule += "Duration: " + String(pumpDuration) + " minutes";
  server.send(200, "text/plain", schedule);
}

void setup() {
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
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

 /**
  * handle fan
  */
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
  if (pumpCurrentMillis - pumpPreviousMillis >= pumpStartDelay * 60000) {
    if (!pumpState) {
     
      pumpState = true;
      pumpPreviousMillis = pumpCurrentMillis;
      pumpOfTime=pumpCurrentMillis + pumpDuration *60000;
      digitalWrite(pumpPin, HIGH);
    } 
  }
  if(pumpCurrentMillis >= pumpOfTime && pumpOfTime>0){
     pumpState = false;
     digitalWrite(pumpPin, LOW);
  }
}

void fanController() {
  fanCurrentMillis = millis();

  // Check if the delay period has passed and it's time to activate the fan
  if (fanCurrentMillis - fanPreviousMillis >= fanStartDelay * 60000) {
    if (!fanState) {
      fanState = true;
      fanPreviousMillis = fanCurrentMillis;
fanOffTime = fanCurrentMillis + fanDuration * 60000;
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
       *  send notifcation and turn of fan
       */
    }else if (tempValue <= MIN_TEMP){
       /**
        * send notificaion turn of fan
        */
    }else{
      /*
       * turn off fan
       */
    }
      if(humidValue >= MAX_HUMID){
      
    }else if (humidValue <= MIN_HUMID){
       
    }
  }
  }

void loop() {
  server.handleClient();
  webSocketServer.loop();
 
  //ReadTmpAndHumidity();
   PumpController();
   fanController();
}
