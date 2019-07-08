#include <WebSocketsClient.h> 
#include <StreamString.h>
#define DEBUG_WEBSOCKETS true

WebSocketsClient webSocket;

const char* MyApiKey = "Sinric api key";

const char* thermoId = "device ID";

#define HEARTBEAT_INTERVAL  500000 // 5 Minutes 
#define TEMPRATURE_INTERVAL 300000 // 3 Minutes 

#define SERVER_URL "iot.sinric.com" //"iot.sinric.com"
#define SERVER_PORT 80 // 80

uint64_t heartbeatTimestamp = 0;
uint64_t tempratureUpdateTimestamp = 0;
bool isConnected = false;

void setPowerStateOnServer(String deviceId, String value);
void setSetTemperatureSettingOnServer();
void readTempature();

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Thermostat
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-thermostatcontroller.html
        // {"deviceId": xxxx, "action": "SetTargetTemperature", value: "targetSetpoint": { "value": 20.0, "scale": "CELSIUS"}} // https://developer.amazon.com/docs/device-apis/alexa-thermostatcontroller.html#settargettemperature
        // {"deviceId": xxxx, "action": "AdjustTargetTemperature", value: "targetSetpointDelta": { "value": 2.0, "scale": "FAHRENHEIT" }} // https://developer.amazon.com/docs/device-apis/alexa-thermostatcontroller.html#adjusttargettemperature
        // {"deviceId": xxxx, "action": "SetThermostatMode", value: "thermostatMode" : { "value": "COOL" }} // https://developer.amazon.com/docs/device-apis/alexa-thermostatcontroller.html#setthermostatmode
            
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String action = json ["action"];

        if (deviceId == thermoId){ // Device ID of first device
        
          if(action == "setPowerState") { // On or Off
            
            String value = json ["value"];
            Serial.println("[WSc] setPowerState" + value);
            if (value == "OFF"){
              checkEnable = 0;
            }
            else if(value == "ON"){
              checkEnable = 1;
            }
            setSetTemperatureSettingOnServer();
          }
          else if(action == "SetTargetTemperature") { 
            const char* error = json["value"]["targetSetpoint"]["value"];
            float value;
            String scale;
            if(error){
              value = json["value"]["targetSetpoint"]["value"];
              scale = json["value"]["targetSetpoint"]["scale"].as<String>();
            }else{
              float value1 = json["value"]["lowerSetpoint"]["value"];
              float value2 = json["value"]["upperSetpoint"]["value"];
              scale = json["value"]["upperSetpoint"]["scale"].as<String>();
              if ( setT ==  value1){
                value = value2;
              }else value = value1;
            }
            int checkValue = value;
            if ((checkValue <= 30) && (checkValue >= 15)){
              checkEnable = 3;
              setT = value;
              Serial.println("Settata Temperatura.....");
            }else {
              Serial.println("Temperatura Fuori range....");
            }
            setSetTemperatureSettingOnServer();
            Serial.print("[WSc] SetTargetTemperature value: ");Serial.println(value);
            Serial.println("[WSc] SetTargetTemperature scale: " + scale);
        }
          else if(action == "AdjustTargetTemperature") {
            //Alexa, make it warmer in here
            //Alexa, make it cooler in here
            String value = json["value"]["targetSetpointDelta"]["value"];
            String scale = json["value"]["targetSetpointDelta"]["scale"];  
            checkEnable = 3;
            setT = setT + value.toFloat();
            setSetTemperatureSettingOnServer();
            Serial.println("[WSc] AdjustTargetTemperature value: " + value);
            Serial.println("[WSc] AdjustTargetTemperature scale: " + scale);          
        }
        else if(action == "SetThermostatMode") { 
          //Alexa, set thermostat name to mode
          //Alexa, set thermostat to automatic
          //Alexa, set kitchen to off
          String value = json["value"]["thermostatMode"]["value"];
          if (value == "AUTO"){
            checkEnable = 1; 
          }
          Serial.println("[WSc] SetThermostatMode value: " + value);
        }
        else if (action == "test") {
                Serial.println("[WSc] received test command from sinric.com");
            }
        }
    }
    break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void setPowerStateOnServer(String deviceId, String value) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
}

// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 

//eg: setSetTemperatureSettingOnServer("deviceid", 25.0, "CELSIUS" or "FAHRENHEIT", 23.0, 45.3)
// setPoint: Indicates the target temperature to set on the termostat.
void setSetTemperatureSettingOnServer() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["action"] = "SetTemperatureSetting";
  root["deviceId"] = thermoId;
  
  JsonObject& valueObj = root.createNestedObject("value");
  JsonObject& temperatureSetting = valueObj.createNestedObject("temperatureSetting");
  temperatureSetting["setPoint"] = setT;
  temperatureSetting["scale"] = "CELSIUS";
  temperatureSetting["ambientTemperature"] = Temperatura;
  temperatureSetting["ambientHumidity"] = Umidita;
   
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
  Serial.print("Invio ad alexa---> ");Serial.println(databuf);
}
// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 

void setThermostatModeOnServer(String deviceId, String thermostatMode) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceId;
  root["action"] = "SetThermostatMode";
  root["value"] = thermostatMode;
  StreamString databuf;
  root.printTo(databuf);
  
  webSocket.sendTXT(databuf);
}

void setupAlexa(){

  Serial.print("Connecting Alexa.... ");
  
  // server address, port and URL
  webSocket.begin(SERVER_URL, SERVER_PORT, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
  /*while (!isConnected) {
    Serial.print(".");
    delay(500);
  }*/
  Serial.println("connected!");
  
}
