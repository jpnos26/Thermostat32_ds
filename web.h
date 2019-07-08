

 void server_web(){
 

  MDNS.addService("http","tcp",80);
  
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client){
    client->send("hello!",NULL,millis(),1000);
  });
  server.addHandler(&events);

  server.addHandler(new SPIFFSEditor(SPIFFS, httpusername,httppassword));

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
   server.on("/all", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.printf("\nGET /all");
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ", \"temperature\":"+String(Temperatura);
    json += ", \"setpoint\":"+String(setT);
    json += ", \"umidita\":"+String(Umidita);
    json += ", \"pressione\":"+String(Pressione);
    json += ", \"noice\":"+String(setPoint[0]);
    json += ", \"relestatus\":\"" +stato+"\"";
    json += ", \"checkTemperatura\":" +String(checkTemperature);
    json += ", \"checkEnable\":"+String(checkEnable);
    json += ", \"sistema\": \""+versione+"\"";
    json += "}";
    Serial.printf("Json: \n");
    request->send(200, "application/json", json);
  });

  server.on("/send", HTTP_GET, [](AsyncWebServerRequest *request){
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      Serial.print("Param name: ");
      Serial.println(p->name());
      Serial.print("Param value: ");
      Serial.println(p->value());
      Serial.println("------");
      if (p->name() == "programma"){
        if (p->value().toInt() < 4){
          checkEnable = p->value().toInt();
        }
        else if(p->value().toInt() == 5){
          request->redirect("/index.html");
          shouldReboot = true;
        }
      }
      else if(p->name() == "temp"){
        if (checkEnable >0 && checkEnable <4){
          checkTemperature = p->value().toInt();
        }
      }
    }
  });

  server.on("/setTemp", HTTP_GET, [](AsyncWebServerRequest *request){
    int params = request->params();
    for(int i=0;i<4;i++){
      AsyncWebParameter* p = request->getParam(i);
      Serial.print("Param name: ");
      Serial.println(p->name());
      Serial.print("Param value: ");
      Serial.println(p->value());
      Serial.println("------");
      setPoint[i] = p->name().toFloat();
    }
    SaveStato();
    request->redirect("/ricarica");
  });
  
  server.on("/ricarica", HTTP_GET, [](AsyncWebServerRequest *request){
   request->redirect("/index.html");   
  });
  
  /// recive data for schedule
  server.on("/save",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      if(!index){
        String S_filena_WBS = "/schedule.json";
        fsUploadFile = SPIFFS.open(S_filena_WBS, "w");
        Serial.printf("BodyStart: %u B\n", total);
        if (!fsUploadFile){ 
          Serial.println("file open failed");
          return;
        }
      }
      for (size_t i = 0; i < len; i++) {
        Serial.write(data[i]);
        fsUploadFile.write(data[i]);
        }
      if(index + len == total){
        Serial.printf("BodyEnd: %u B\n", total);
        Serial.println ("Schedule Saved......");
        fsUploadFile.close();
        loadCrono();
      }
      request->send(200);
  });
  server.on("/setNetwork", HTTP_GET, [](AsyncWebServerRequest *request){
        int params = request->params();
        int i = 0;
        AsyncWebParameter* p = request->getParam(i);
        Serial.printf("HEADER[%s]: %s\n", p->name().c_str(), p->value().c_str());
        String salva_network = p->name().c_str();
        String S_filena_WBS = "/rete.json";
        request->redirect("/index.html");
        Serial.println("Data saved "+S_filena_WBS+" - "+salva_network);
        File fsUploadFile = SPIFFS.open(S_filena_WBS, "w");
        if (!fsUploadFile){ 
            Serial.println("file open failed");
            }
        else{
            fsUploadFile.println(salva_network); 
            Serial.printf("BodyEnd: %s\n",salva_network.c_str());
            }
        fsUploadFile.close();
        request->redirect("/index.html");
        check_wifi = 1;
        });
  // Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      //Update.runAsync(true);
      //if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
      //  Update.printError(Serial);
      //}
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
    
  });
  server
    .serveStatic("/", SPIFFS, "/")
    .setDefaultFile("index.html")
    .setCacheControl("max-age=300");

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index)
      Serial.printf("UploadStart: %s\n", filename.c_str());
    Serial.printf("%s", (const char*)data);
    if(final)
      Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index+len);
  });
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
});
// se update ok allora reboot
 if(shouldReboot){
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
 }
