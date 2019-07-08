void wifiStart(){ 
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(false);
        WiFi.mode(WIFI_STA);
        Serial.print("Dhcp abilitato: ");Serial.println(dhcp);
        if (dhcp == 0){
            WiFi.config(ip,gateway,netmask,gateway);
            Serial.print("Letto  Ip Fisso: ");Serial.print(ip);Serial.print(" - ");Serial.print(gateway);Serial.print(" - ");Serial.println(netmask);
          }
        Serial.print("Dati Connessione Wifi : ");Serial.print( wifissid);Serial.print(" - ");Serial.println(wifipassword);
        WiFi.begin(wifissid.c_str(), wifipassword.c_str());
        WiFi.printDiag(Serial);
        for ( int count = 0; count < 20 ; count++ )  {
            Serial.print("test:");Serial.print( count); Serial.print(" - SSID:");Serial.print(WiFi.SSID());Serial.print( " - Status:" );
            Serial.print( WiFi.status() );Serial.print("-");Serial.print("RSSi: ");Serial.println(WiFi.RSSI());
            if (WiFi.status()  == WL_CONNECTED ) {
              break;
            }
            delay(500);
        }
        /*if (WiFi.waitForConnectResult()  == WL_CONNECTED) {
            Serial.println("");
            Serial.print("WiFi connessa --> ");Serial.println(WiFi.localIP());
            ulNextWifiCheck = millis()+500000;
            }*/
        if (WiFi.status()  == WL_CONNECTED ) {
              Serial.print("\n - WiFi Ip: ");Serial.print(WiFi.localIP());Serial.println(" - Connection ready.........");
              ulNextWifiCheck = millis()+300000;
            }
        else{
           Serial.printf("\r\nWiFi connect aborted !\r\n");
           delay(50);
           Serial.print("Dati AP :");Serial.print(wifiName);Serial.print(" - ");Serial.println(httppassword);
           WiFi.mode(WIFI_AP);
           WiFi.softAP(wifiName.c_str(), httppassword.c_str());
           delay(150);
           Serial.print("Access Point : ");Serial.print(wifiName);Serial.print(" - address ");Serial.println(WiFi.softAPIP());
           //check_wifi +=1;
           if (check_wifi >= 3){
                delay(100);
                Serial.println("Tentativi reconnect esuriti REBOOT");
                ESP.restart();
                }
           ulNextWifiCheck = millis()+60000;
        }
       
}

int printStations()
{
 
  wifi_sta_list_t stationList;
 
  esp_wifi_ap_get_sta_list(&stationList);  
 
  Serial.print("Number of connected stations: ");
  Serial.println(stationList.num);
 
  for(int i = 0; i < stationList.num; i++) {
 
    wifi_sta_info_t station = stationList.sta[i];
 
    for(int j = 0; j< 6; j++){
      char str[3];
 
      sprintf(str, "%02x", (int)station.mac[j]);
      Serial.print(str);
 
      if(j<5){
        Serial.print(":");
      }
    }
    Serial.println();
  }
  return stationList.num;
  Serial.println("-----------------");
}
