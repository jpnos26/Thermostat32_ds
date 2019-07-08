void codeForTask1( void * parameter )
{
  for (;;) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    if (millis() >= telegram_timer ){
        telegramEnabled = 0;
        if (telegram_id != ""){
            bot.sendMessage(chat_id, "Bot Disabled .....", "HTML");
            telegram_id = "";
            chat_id = "";
            telegramEnabled = 0;
        }
      }
    while(numNewMessages) {
      //Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
       
    }
  delay(1000);  
  }
}


void codeForTask3( void * parameter )
{
  for (;;) {
    float readT = bme.readTemperature();
    if (readT<= 60){
      Temperatura = readT+tempCorrect;
    }
    Umidita = bme.readHumidity();
    Pressione = bme.readPressure() / 100.0F;
    //Serial.print("Temperatura: ");Serial.print(Temperatura);Serial.print(" Umidita: ");Serial.print(Umidita);Serial.print(" Pressione: ");Serial.println(Pressione);
    if(screen) write_display();
    time_t t = now()+946684800UL;
    int ora = hour(t);
    int minuti = minute(t);
    int oggi = weekday(t)-1;
    //Serial.print("data riferimento: ");Serial.print(ora);Serial.print(":");Serial.print(minuti);Serial.print(" Giorno: ");Serial.println(oggi);
    for (int test=0; test<=9; test++){
        //Serial.print("Controllo Schedule: ");Serial.print(oggi);Serial.print(" - ");Serial.println(scheduleDay[oggi][0][test]);
        if (scheduleDay[oggi][0][test]!=0){
          if (scheduleDay[oggi][0][test] == ora && scheduleDay[oggi][1][test] == minuti){
            if (checkTemperature != scheduleDay[oggi][2][test]){
              checkTemperature = scheduleDay[oggi][2][test];
              if (checkEnable == 3) checkEnable = 1;
              Serial.println("Cambiata Temperatura: "+String(ora)+":"+String(minuti)+" --->"+String(setPoint[checkTemperature]));
            }  
          }
        }
      }
    delay(5000);  
  }
}
void codeForTask4( void * parameter )
{
  for (;;) {
    int setTempe =0;
    if (checkEnable == 1) {
      setTempe=setPoint[checkTemperature]*100;
    }
    else if(checkEnable == 2){
      setTempe=setPoint[checkTemperature]*100;
    }
    else if (checkEnable == 3){
      setTempe = setT*100;
    }
    else {
      setTempe = setPoint[0]*100;
    }
    int temp = Temperatura*100;
    int isteresi = setTempe-tempHist;
    setT=setTempe/100.0;
    //Serial.print("Temperatura: ");Serial.print(temp);Serial.print(" - Set: ");Serial.print(setTempe);Serial.print(" - isteresi: ");Serial.println(isteresi);
    if (temp >= setTempe){
      //Serial.println("Rele OFF");
       digitalWrite(relePin, !releON);
       stato = "OFF";
       
    }
    else if(temp <= isteresi){
      //Serial.println("Rele ON");
      digitalWrite(relePin, releON);
      stato = "ON";    
    }
    delay(2000);
  }
 }
