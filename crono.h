#include <Arduino.h>

void SaveStato () {
  String salva = "{\"tempSet\":";
  salva += String(setT);
  salva += ", \"setTemp\":";
  salva += String(checkTemperature);
  salva +=", \"Programma\":" +String(checkEnable);
  salva +=" , \"temp\": [";
  for(int i=0;i<4;i++){
    salva  += String(setPoint[i]);
    if(i < 3){
      salva +=",";
    }
  }
  salva +="]}";
  //Serial.print("Saving in SPIFFS : "); Serial.println(salva);
  
  // open file for writing
  File spiffs = SPIFFS.open("/stato.json", "w");
  if (!spiffs) {
    Serial.println("Failed to open crono.json");
    return;
  }
  //qui salvo il buffer su file
  spiffs.println(salva);
  Serial.println("Salvo in SPIFFS il buffer con i settings :"); Serial.println(salva);
  delay(1);
  //chiudo il file
  spiffs.close();
}

void loadStato(){
  File  carica = SPIFFS.open("/stato.json", "r");
    if (!carica) {
      Serial.println(" ");
      Serial.println("Non riesco a leggere stato.json! ...");
      }
    else{
        String risultato = carica.readStringUntil('\n');
        const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + 7*JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 350;
        char json[bufferSize];
        risultato.toCharArray(json, bufferSize);
        Serial.print("Array json convertito: ");Serial.println(json);Serial.print("Num char: ");Serial.println(bufferSize);
        DynamicJsonBuffer jsonBuffer(bufferSize);
        //StaticJsonBuffer<650> jsonBuffer_inlettura;
        JsonObject& root = jsonBuffer.parseObject(json);
        if (!root.success()) {
          Serial.println("parseObject() failed");
            }
        else{
          setT = root["tempSet"].as<float>();
          checkTemperature = root["setTemp"].as<int>();
          checkEnable = root["Programma"].as<int>();
          for ( int c = 0; c < 4 ;c++){
            setPoint[c] = root["temp"][c];
          }
          Serial.print(checkTemperature);Serial.println(checkEnable);
        }
    }
}

void loadCrono(){
  Serial.println("Read Scheduling Settings from SPIFFS...");
  File  schedule_load = SPIFFS.open("/schedule.json", "r");
  if (!schedule_load) {
    Serial.println("Failed to open schedule.json");
    return;
    }
  String risultato;
  while (schedule_load.available()){
    char inchar = (char)schedule_load.read(); 
    if ((' ' < inchar) && (inchar <= '~'))
      risultato += inchar;
  }
  schedule_load.close();
  //Serial.print("Ho letto dal file : ");Serial.println(risultato);
  DynamicJsonBuffer jsonBuffer_schedule;

  JsonObject& rootschedule = jsonBuffer_schedule.parseObject(risultato);
  if (!rootschedule.success()) {
    Serial.println("parseObject() failed");
  }
  JsonObject& heat1 = rootschedule["heat"];
  for (JsonObject::iterator it=heat1.begin(); it!=heat1.end(); ++it){
        String Oggi1 = it->key;
        int Oggi = Oggi1.toInt();
        //Serial.print("heat1 ");Serial.print(it->key);Serial.print(" - ");Serial.println(heat1[it->key].as<String>());
        //JsonArray& heat = heat1[it->key];
        int arraySize =  heat1[it->key].size();
          for (int i = 0; i< arraySize; i++){
            int array2 = heat1[it->key][i].size();
              String orario=heat1[it->key][i][0];
              //Serial.println(orario);
              int calore = heat1[it->key][i][1];
              scheduleDay[Oggi][0][i] = orario.substring(0,2).toInt();
              scheduleDay[Oggi][1][i] = orario.substring(3,5).toInt();
              scheduleDay[Oggi][2][i] = calore;
              //Serial.print(scheduleDay[Oggi][0][i]);Serial.print(scheduleDay[Oggi][1][i]);Serial.println(scheduleDay[Oggi][2][i]);
            }
        }
        
          //Serial.print("stage 2 ");Serial.print(heat.value.as<int>());Serial.println(heat.key);
        //for (JsonObject::iterator it1=heat.begin(); it1!=heat.end(); ++it1){  
        //  Serial.println(heat1[it->key][it1->key].as<);
        //}
          
        
    
}

void save_datalogger(float setpoint,float temperature,float humidity,float pressure,byte relaystatus) {
  time_t t = now()+946684800UL;
  String S_TimeDate = String(year(t))+"/"+String(month(t))+"/"+String(day(t))+" " +String(hour(t))+":"+String(minute(t)) + ":00";
  int iDay = weekday(t);
  String S_filename = "/datalog/datalogger" + String(iDay) + ".csv";
  

//  Serial.print("iDay. ");Serial.println(iDay);
//  Serial.print("iDay_prev. ");Serial.println(iDay_prev);


  if(iDay_prev > 0 && (iDay != iDay_prev)) {
    if(SPIFFS.exists(S_filename)==1)  {
      Serial.print("    Delete older file: ");Serial.println(S_filename);
      SPIFFS.remove(S_filename);
   }
  }
  


  File datalogger = SPIFFS.open(S_filename, "a");
  if (!datalogger) {
    Serial.print(S_filename);Serial.println(" open failed");
  }
    datalogger.print(S_TimeDate);datalogger.print(",");datalogger.print(setpoint,1);datalogger.print(",");datalogger.print(temperature,1);datalogger.print(",");datalogger.print(humidity,1);datalogger.print(",");datalogger.print(pressure,0);datalogger.print(",");datalogger.println(relaystatus);
    Serial.print("Salvo linea datalogger -> ");Serial.print(S_TimeDate);Serial.print(",");Serial.print(setpoint,1);Serial.print(",");Serial.print(temperature,1);Serial.print(",");Serial.print(humidity,1);Serial.print(",");Serial.println(relaystatus);
  Serial.print(S_filename);Serial.print(" size: ");Serial.println(datalogger.size());
  datalogger.close();
  iDay_prev = weekday(t);
  ////////////////////////
 
} 

void carica_setting(){
 /// carichiamo se esiste il file di configurazione
    File  carica_setting = SPIFFS.open("/rete.json", "r");
    if (!carica_setting) {
      Serial.println(" ");
      Serial.println("Non riesco a leggere rete. json! ...");
      }
    else{
        String risultato = carica_setting.readStringUntil('\n');
        const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + 7*JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(3) + 2*JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 350;
        char json[bufferSize];
        risultato.toCharArray(json, bufferSize);
        Serial.print("Array json convertito: ");Serial.println(json);Serial.print("Num char: ");Serial.println(bufferSize);
        DynamicJsonBuffer jsonBuffer(bufferSize);
        //StaticJsonBuffer<650> jsonBuffer_inlettura;
        JsonObject& root = jsonBuffer.parseObject(json);
        if (!root.success()) {
            Serial.println("parseObject() failed");
            }
        else{
            JsonObject& rete0 = root["rete"][0];
            wifissid = rete0["ssid"].as<String>();
            wifipassword = rete0["pwd"].as<String>();
            dhcp = rete0["dhcp"].as<int>();
            JsonArray& rete0_ip = rete0["ip"];
            String test= rete0_ip[0].as<String>()+"."+rete0_ip[1].as<String>()+"."+rete0_ip[2].as<String>()+"."+rete0_ip[3].as<String>();
            //Serial.print(test);Serial.print(" - ");Serial.println(static_ip);
            test.toCharArray(static_ip, 16);
            JsonArray& rete0_gateway = rete0["gateway"];
            test= rete0_gateway[0].as<String>()+"."+rete0_gateway[1].as<String>()+"."+rete0_gateway[2].as<String>()+"."+rete0_gateway[3].as<String>();
            //Serial.print(test);Serial.print(" - ");Serial.println(static_gateway);
            test.toCharArray(static_gateway, 16);
            JsonArray& rete0_netmask = rete0["netmask"];
            test= rete0_netmask[0].as<String>()+"."+rete0_netmask[1].as<String>()+"."+rete0_netmask[2].as<String>()+"."+rete0_netmask[3].as<String>();
            //Serial.print(test);Serial.print(" - ");Serial.println(static_netmask);
            test.toCharArray(static_netmask, 16);
            //IPAddress netmask(rete0_netmask[0],rete0_netmask[1],rete0_netmask[2],rete0_netmask[3]);
            JsonObject& ap0 = root["ap"][0];
            wifiName = ap0["hostname"].as<String>();
            httpusername = ap0["http_username"].as<String>();
            httppassword = ap0["http_password"].as<String>();
            }
        carica_setting.close();
        //set static ip
        ip.fromString(static_ip);
        gateway.fromString(static_gateway);
        netmask.fromString(static_netmask); 
        
        
        
        delay(100);
        }
}
