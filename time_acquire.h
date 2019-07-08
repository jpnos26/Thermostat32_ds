#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>


unsigned long timezoneRead()
 {
      Serial.print("Ora Corrente da server Internet: " );
      String keyDB = "Key db timezonedb";        ///chiave database internet
      HTTPClient http;
      // We now create a URI for the request
      String url = "http://api.timezonedb.com/v2/get-time-zone?key="+keyDB+"&format=json&by=zone&zone=Europe/Rome";
      Serial.print("Requesting URL: ");Serial.println(url);
      http.begin(url);
      http.addHeader("Content-Type", "application/json");
      String json;
      int httpCode = http.GET();
      if (httpCode) {
        if (httpCode == 200) {
          json = http.getString();
          Serial.println(json);
        }
      }
      http.end();
      const size_t bufferSize = JSON_OBJECT_SIZE(13) + 250;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      
      Serial.print("Got data:");Serial.println(json);
      JsonObject& root = jsonBuffer.parseObject(json);
      if (!root.success()){
        Serial.println("parseObject() failed");
        return (0);
        }
      String data = root["status"];
      int data1= root["gmtOffset"]; 
      unsigned long data2 = root["timestamp"]; 
      if (data =="OK"){
          data2 -= 946684800UL;
          return(data2);
      }
      else {
        return (0);
      }
    
 }
void ntpacquire() 
  {
  ///////////////////////////////
  // connect to NTP and get time
  ///////////////////////////////
  //Leggo Time da NTP
  //unsigned long timeNtp=getNTPTimestamp();
  //if (!timeNtp == 0){
  //  ulSecs2000_timer=timeNtp;
  //  Serial.print("Ora Corrente da server NTP : " );
  //  Serial.println(epoch_to_string(ulSecs2000_timer).c_str());
  //  ulSecs2000_timer -= millis()/1000;  // keep distance to millis() counter
  //  ulNextntp=millis()+3600000;
  //}
  //delay(100);
  //Leggo Time da TimezoneDb
  if (WiFi.status() == WL_CONNECTED){
    unsigned long timeInternet=timezoneRead();
    if (!timeInternet == 0)
      {
        ulSecs2000_timer=timeInternet;
        Serial.print("\nOra Corrente da server Internet: " );
        printDateTime(ulSecs2000_timer);
           
        //ulSecs2000_timer -= millis()/1000;  // keep distance to millis() counter
        setTime( timeInternet);//+946684800UL);
        delay(100);
        rtc_check(true);
        /*time_t timeNow = now();
        
        Serial.print("Ora Set :"+ String(hour(timeNow))+":"+String(minute(timeNow))+":"+String(second(timeNow))+"-"+String(day(timeNow))+"/"+String(month(timeNow))+"/"+String(year(timeNow))+"\n");
      */}
      else {
        rtc_check(false); 
      }
  }
    else {
      rtc_check(false);
    }
  ulNextntp=millis()+ulNtpDelta;
 }
