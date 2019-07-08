void message(String chatId){
  String state = "<b>-----Thermostat32-----</b>\n";
    state +="Temperatura in: " ;
    state += "<b>";
    state += Temperatura;
    state += "</b>";
    state += "\nProgramma : <b>" ;
    state += descProg[checkEnable];
    state +="</b>";
    if (checkEnable != 0){ 
      state += "\nSet Temp : <b>" ;
      state += descPoint[checkTemperature];
      state += " - ";
      state += setT;
      state += "</b>";
      }
    state += "\nCaldaia : <b>";
    state += stato;
    state += "</b>\n";
    state += versione;
   Serial.println(chatId +" - "+state);
  bot.sendMessage(chatId, state, "HTML");          
}

void getCommands(String chatId){
  String keyboardJson = "[[{ \"text\" : \"Auto\", \"callback_data\" : \"auto\" },{ \"text\" : \"Manuale\", \"callback_data\" : \"manuale\" },{ \"text\" : \"No Ice\", \"callback_data\" : \"noice\" }]";
      if (checkEnable != 0){
        keyboardJson += ",[{ \"text\" : \"Eco\", \"callback_data\" : \"eco\" },{ \"text\" : \"Normal\", \"callback_data\" : \"normal\" },{ \"text\" : \"Comfort\", \"callback_data\" : \"comfort\" }],[{ \"text\" : \"Reboot\", \"callback_data\" : \"reboot\" },{ \"text\" : \"Close\", \"callback_data\" : \"close\" }]]";
      }else{
        keyboardJson += ",[{ \"text\" : \"Reboot\", \"callback_data\" : \"reboot\" },{ \"text\" : \"Close\", \"callback_data\" : \"close\" }]]";
      }
      //Serial.println(keyboardJson);
   bot.sendMessageWithInlineKeyboard(chatId, "Thermostat32 Select", "", keyboardJson);
}

void handleNewMessages(int numNewMessages) {
  Serial.print("New Telegram Message :");Serial.print("handleNewMessages");Serial.print(" - ");Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    Serial.println(bot.messages[i].text);
    if (bot.messages[i].text == telegram_Password){
      telegram_id = bot.messages[i].from_id;
      chat_id = String(bot.messages[i].chat_id);
      message(chat_id);
      getCommands(chat_id);
      telegram_timer = millis() + 30000;
      telegramEnabled = 1;
    }
    else{
      if(bot.messages[i].from_id == telegram_id){
        if (telegramEnabled == 1){
          if (bot.messages[i].type == "callback_query") {
            Serial.print("Call back button pressed by: ");
            Serial.println(bot.messages[i].from_id);
            Serial.print("Data on the button: ");
            Serial.println(bot.messages[i].text);
            if (bot.messages[i].text == "auto"){
              checkEnable = 1;
            }
            else if (bot.messages[i].text == "noice"){
              checkEnable = 0;
            }
            else if (bot.messages[i].text == "manuale"){
              checkEnable = 2;
            }
            else if (bot.messages[i].text == "eco"){
                checkTemperature = 1;
            }
            else if (bot.messages[i].text == "normal"){
              checkTemperature = 2;
            }
            else if (bot.messages[i].text == "comfort"){
              checkTemperature = 3;
            }
            else if (bot.messages[i].text == "reboot"){
              check_wifi = 1;
            }
            else if (bot.messages[i].text == "close"){
              bot.sendMessage(chat_id, "Bot Disabled .....", "HTML");
              telegram_id = "";
              chat_id = "";
              telegramEnabled = 0;
              return;
            }
            message(chat_id);
            delay(100);
            getCommands(chat_id);
            telegram_timer = millis() + 30000;
            }
        }
       }
    }
  }
}
