/*******************************************************************
 *  Thermostat32     un cronotermostato su Esp32                   *
 *  con lcd nextion advanced                                       *
 *  multitasking !!!!!                                             *
 *  written by Jpnos......jpnos at gmx.com                         *
 *  con l'aiuto di  Gianpi69                                       *
 *  implementato alexa da 1.1.0                                    *
 *******************************************************************/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "time_ntp.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <FS.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <ESPmDNS.h>
#include <time.h> 
#include <TimeLib.h>
#include <ArduinoOTA.h>
#include "esp_wifi.h"



#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1

///////////////////

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");
#include "webinit.h"

TaskHandle_t  Task1,Task2,Task3,Task4;

#define SEALEVELPRESSURE_HPA (1013.25)






Adafruit_BME280 bme; // I2C
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///Wifi Configuration ///////////////// Attenzione vengono riscritti da file rete.json nel data dello spiffs///
////// Va modificato il file altrimenti i dati usati saranno quelli del file //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

String wifissid = "wifi ssid";                       // ssid rete wifi
String wifipassword = "wifi pwd";             // pwd rete wifi
String wifiName = "Esp32 Crono";                // Nome se ap invece di connessione al wifi
String httpusername = "admin";                  // Username per accesso web
String httppassword = "administrator";          // Pwd per accesso web e accesso ap ricordare almeno 8 caratteri
unsigned long ulNextWifiCheck;                  // siamo in ap check wifi
int dhcp = 0;                                   ////0 dhcp disable, 1 dhcp enabled
char static_ip[16] = "192.168.1.160";
char static_gateway[16] = "192.168.1.1";
char static_netmask[16] = "255.255.255.0";
IPAddress ip;
IPAddress gateway;
IPAddress netmask;
IPAddress dns(8,8,8,8);
int check_wifi = 0;

String versione = "Thermo32_ds 1.1.2";
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int scheduleDay [8][3][10];                                   // Array dello Scheduling max setpoint scheduling = 10
bool debug = 1;                                               // Debug su Seriale
unsigned int cycle_down = 0;                                  // Variabile per spegnere lo schermo
unsigned int num_cycle_down = 50;                             // numero di cicli di next_down per lo spegnimento 0 non spegne mai 
byte checkEnable = 1;                                         // Controllo 0 = OFF NoIce 1 = Auto 2 = Manuale
byte checkEnablePre;
byte checkTemperature = 0;                                    // Switch per le Temperature
byte checkTemperaturePre;
float setPoint[4] = { 15.0,18.0, 20.0, 21.5};                 // Setpoint NoIce,Eco,Normal,Comfort,Comfort+
char* descPoint[4] = {"NoIce  ","Eco    ", "Normal ", "Comfort"};    // Nomi NoIce Setpoint Eco,Normal,Comfort,Comfort+
char* descProg[4] = {"NoIce  ","Auto   ","Manuale","Alexa    "};               // Nomi programmi
unsigned long ulSecs2000_timer;                               // orario in microsecondi
//unsigned long ulSecsRtc_timer;                                // orario rtc in microsecondi
unsigned long ulNextntp;                                      // next ntp check
unsigned long ulNtpDelta = 3600000;                           // prossimo check orario fatto ogni ora
unsigned long ulNextgraph;                                    // prossimo salvataggio grafico
unsigned long ulGraphDelta = 600000;                          // Delta salvataggio grafico
float Temperatura;                                            // read temperature
float Umidita;                                                // read umidity
float Pressione;                                              // read pressure
float setT = 15;                                              // Set Temperatura
int tempHist=20;                                              // temp histeresis 0.2 diventa 20 per aumentare la precisione
float tempCorrect=-0.3;                                       // Correzione della temperatura del bme280
String stato= "OFF";                                          // stato sistema
bool status;                                                  // stato 
const int relePin = 13;                                       // pin per rele
const int pirPin = 4;                                         // pin sensore PIR
bool releON = 0;                                              // 0 per scheda con optoisolatore , 1 per diretto
int iDay_prev;                                                // ultimo giorno registrato per cancellare il grafico
bool shouldReboot = false;                                    // reboot se update di nuovo firmware
int rtcSensor = 0;                                            // tipo di rtc utilizzato 0 = Nextion , 1 = DS3132
bool screen = true;                                           // utilizzo o meno dello schermo !!!! mettere false se non si ha schermino
bool settaTime = false;
bool alexaOn = true;                                          // True utilizzo Alexa  --- False non utilizzo alexa
File fsUploadFile;

/////////////////// Initialize Telegram BOT

#define BOTtoken "Telegram Token"  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


String telegram_Password = "/pwd";
String telegram_id;
String chat_id;
unsigned long telegram_timer; 
byte telegramEnabled;


  
///// qui selezionare quale tipo di rtc si ha 
#include "rtc3231.h"                      /// togliere "//" all'inizio se si vuole usare questo , Selezionare un solo rtc
//#include "rtc1307.h"                    /// togliere "//" all'inizio se si vuole usare questo 
#include "ssd1306.h"
#include "crono.h"
#include "telegram.h"
#include "wifi.h"
#include "Task.h"
#include "time_acquire.h"
#include "web.h"
#include "alexa.h"

void setup() {
  
  /// Initialize Serial
  Serial.begin(115200);
  Serial.println("\n********************************");
  Serial.println("********Thermostat32_ds*********");
  Serial.println("********************************");
  
  rtc_setup();
  delay(500);
  
  if(screen) init_display();
  
  //SPIFFs Initialize
  SPIFFS.begin();
  loadStato();
  
  carica_setting();
  delay(500);
 
  // Attempt to connect to Wifi network:
  wifiStart();

  delay(500);
  pinMode(relePin, OUTPUT); // Inizializzo pin
  digitalWrite(relePin, !releON); // initialize pin as off
  delay(100);
  
  //initialize ntp
  ntpacquire();
  
  loadCrono();
  
  status = bme.begin(0x77,&Wire);  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        
    }
  if(screen) clear_display();
  xTaskCreatePinnedToCore(
    codeForTask1,
    "telegram",
    6000,
    NULL,
    1,
    &Task1,
    0);
  delay(500);
  xTaskCreatePinnedToCore(
    codeForTask3,
    "bme280",
    3000,
    NULL,
    3,
    &Task3,
    0);
  delay(500);
  xTaskCreatePinnedToCore(
    codeForTask4,
    "Caldaia",
    2000,
    NULL,
    4,
    &Task4,
    1); 
  delay(500);
  Serial.println("Starting server web.....");
  server_web();
  server.begin();
  setupAlexa();
  Serial.print("Setup finalizzato Free heap: ") ;Serial.println(ESP.getFreeHeap());
  Serial.println("************************************");
}

void loop() { 
  if (alexaOn){
    webSocket.loop();
    if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H"); 
          Serial.println ("Alexa send H");         
      }

      // Send the tempature settings to server
      if((now - tempratureUpdateTimestamp) > TEMPRATURE_INTERVAL) {
          tempratureUpdateTimestamp = now;
          setSetTemperatureSettingOnServer(); 
          Serial.println("Alexa send temperatura ");
      }
    } 
  }
  ////salvo grafico
  if (millis()>= ulNextgraph)
      {     
       bool rele = digitalRead(relePin);
       byte relesend = 0;
       if (releON == 0){
        rele = !rele;
       }
       if (rele){
        byte relesend = 5;
       }
       save_datalogger(setT,Temperatura,Umidita,Pressione,relesend);
       delay(10);
       ulNextgraph = millis()+ulGraphDelta;
      } 
  /// controllo se devo fare il reboot
  if ((shouldReboot == true) || (check_wifi == 1)){
     ESP.restart();
  }
  /// controllo se salvare lo stato del sistema
  if ((checkEnable != checkEnablePre) || (checkTemperature != checkTemperaturePre)){
    SaveStato();
    checkEnablePre = checkEnable;
    checkTemperaturePre = checkTemperature; 
    if (alexaOn){
      setSetTemperatureSettingOnServer();  
    }
  }
  
  ///controllo se aggiornare l'orario
  if ( millis() >= ulNextntp){
    ntpacquire();
    
  }
  ///controllo se wifi e connesso
  if (millis() >= ulNextWifiCheck){
    //Serial.print("Stazioni connesse: ");Serial.println (printStations());
    if ((WiFi.status() != WL_CONNECTED) && (printStations() == 0)){
      wifiStart();
    }
  }
  
  delay(500);
}
