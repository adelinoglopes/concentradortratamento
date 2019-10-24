/*
//rotina que inicializar a leitura de tempo
void ntpinitserver(){
    timeClient.begin();
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
  
    // Extract date
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
  
    // Extract time
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

   //rotina ntpinternet ira pegar a hora da internet cada 5 min
     xTaskCreatePinnedToCore(
      ntpinternet, // Function to implement the task 
      "NTP", // Name of the task 
      20000,  // Stack size in words 
      NULL,  // Task input parameter 
      20,  // Priority of the task 
      &Task5,  // Task handle. 
      0); // Core where the task should run 

     Serial.println ("FIM SETUP NTP " + String(dayStamp) + String(timeStamp));
}
*/

/* tem que ver o que ocorreu pois apos atualizar o arduino a rotina parou. por hora nao esta sendo usada
//atualiza a hora pegando da internet e envia para os outros loras
void ntpinternet (void * parameter){
  for(;;){ //loop infinito da rotina
      while(!timeClient.update()) {
        timeClient.forceUpdate();
      }
      formattedDate = timeClient.getFormattedDate(); //peguei a data e a hora   no formato 2018-05-28T16:00:13Z
      
      // Extract date
      int splitT = formattedDate.indexOf("T");
      dayStamp = formattedDate.substring(0, splitT);
      
      // Extract time
      timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1); //ntp internet timeStamp: 06:26:46

      envio = String (VARIABLE_LABEL_HORA) + "," + String (formattedDate);
      for(int i=0;i<3;i++)        //envia 3 vezes
      {
          lorasend(String(envio), LABORATORIOLORA,BROADCAST);       
      } //envia 3 vezes

      vTaskDelay (300000); //5 minuto
  }  //loop
}
*/
//rotinina que inicializa o relogio interno
void relogioinit(){
  int identificador1, identificador2;
  String temporario;

  if (dayStamp == "") { //verifica se ja recebeu a hora do concentrdor
    dayStamp = "2019-01-01";
    timeStamp = "00:00:00";
  }

  //hora e data inicial que veio do ntp
  identificador1 = dayStamp.indexOf('-'); //achando ano
  temporario = dayStamp.substring (2,identificador1); //pega somente os dois ultimos dias do ano 2019 = 19
  tinfo.tm_year = temporario.toInt();
  identificador2 = dayStamp.indexOf('-', identificador1 + 1); //achando mes
  temporario = dayStamp.substring (identificador1 + 1,identificador2);
  tinfo.tm_mon = temporario.toInt();
  temporario = dayStamp.substring (identificador2 + 1,identificador2 + 3); 
  tinfo.tm_mday = temporario.toInt();
  identificador1 = timeStamp.indexOf(':'); //achando hora
  temporario = timeStamp.substring (0,identificador1);
  tinfo.tm_hour = temporario.toInt();
  identificador2 = timeStamp.indexOf(':', identificador1 + 1); //achando minuto
  temporario =  timeStamp.substring (identificador1+1,identificador2);
  tinfo.tm_min = temporario.toInt(); 
  temporario = timeStamp.substring (identificador2 + 1,identificador2 + 3);
  tinfo.tm_sec = temporario.toInt();
  //Serial.println("relogio tinfo.tm_sec: " + String (temporario.toInt()) + " " + String (tinfo.tm_sec));
  //Serial.println ( "tm_hour0: " + String(tinfo.tm_hour) + " tm_min: " +  String(tinfo.tm_min) + " tm_sec: " +  String(tinfo.tm_sec));
  //Serial.println ( "tm_tm_mday0: " + String(tinfo.tm_mday) + " tm_mon: " +  String(tinfo.tm_mon) + " tm_year " +  String(tinfo.tm_year));
  
  // Convert time structure to timestamp
  initialt = timelib_make(&tinfo);
  //Serial.println("relogio initialt: " + String (initialt));
  
  // Set system time counter
  timelib_set(initialt);
 
  // Configure the library to update / sync every day (for hardware RTC)
  timelib_set_provider(time_provider, TIMELIB_SECS_PER_DAY);
  
  //pega a hora do relogio interno
  now = timelib_get();
  
  // Convert to human readable format
  timelib_break(now, &tinfo);
  
  // Send to serial port
  horario = String(tinfo.tm_hour) + ":" + String(tinfo.tm_min) + ":" + String(tinfo.tm_sec);
  dia = String(tinfo.tm_mday) + "/" + String(tinfo.tm_mon) + "/" + String(tinfo.tm_year);


   //rotina que atualiza as variaveis do tempo interno
     xTaskCreatePinnedToCore(
      relogio, 
      "Relogio", 
      20000,  
      NULL,  
      20,  
      &Task6,  
      0); 

  if (NOME_DISPOSITIVO  != "Concentrador") {
   //rotina que atualiza as variaveis do tempo interno recebendo o tempo pela rede lora
     xTaskCreatePinnedToCore(
      relogiorecebelora, 
      "Relogio Recebe", 
      20000,  
      NULL,  
      20,  
      &Task10,  
      0); 
  } //so faco atualizaÃ§Ã£o do relogio se nao for no concetrador que tem wifi
        
    Serial.println ("FIM Relogio INIT hora: " + String(horario) + " dia: " + String(dia));
}


timelib_t time_provider()
{
  // Prototype if the function providing time information
  return initialt;
}

//atualiza as variaveis de hora do relogio interno 
void relogio (void * parameter){
  for(;;){ //loop infinito da rotina
      now = timelib_get();
      
      // Convert to human readable format
      timelib_break(now, &tinfo);
      
      // Send to serial port
      horario = String(tinfo.tm_hour) + ":" + String(tinfo.tm_min) + ":" + String(tinfo.tm_sec);
      dia = String(tinfo.tm_mday) + "/" + String(tinfo.tm_mon) + "/" + String(tinfo.tm_year);
       
      //Serial.println ("Hora : " + horario);
      //Serial.println ("Data : " + dia);
      vTaskDelay (30000); //fica parado por 30 segundos
  }  //loop
}

//rotina que atualiza as variaveis do tempo interno recebendo o tempo pela rede 
void relogiorecebelora (void * parameter){
  String msgrecebida = "";
  String identificador = "";
  String valorstring = "";
  //char valorstringronda [15] = "";
  //float valor = 0;
  int splitT;
  int identificador1, identificador2;
  String temporario;
  
  for(;;){ //loop infinito da rotina
    // receber msg lora
    msgrecebida = lorareceive(BROADCAST);
    if (msgrecebida != "") {
      //Serial.print("Mensagem Lora Recebida: "); 
      //Serial.println(msgrecebida); 
      int virgula = msgrecebida.indexOf(','); //Lora send: hora,2019-05-24T11:16:31Z
      identificador =  msgrecebida.substring(0,virgula);
      valorstring = msgrecebida.substring(virgula + 1);
      if (identificador == VARIABLE_LABEL_HORA) {
        splitT = formattedDate.indexOf("T");
        dayStamp = formattedDate.substring(5, splitT);
        timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
        identificador1 = dayStamp.indexOf('-'); //achando ano
        temporario = dayStamp.substring (2,identificador1); //pega somente os dois ultimos dias do ano 2019 = 19
        tinfo.tm_year = temporario.toInt();
        identificador2 = dayStamp.indexOf('-', identificador1 + 1); //achando mes
        temporario = dayStamp.substring (identificador1 + 1,identificador2);
        tinfo.tm_mon = temporario.toInt();
        temporario = dayStamp.substring (identificador2 + 1,identificador2 + 3); 
        tinfo.tm_mday = temporario.toInt();
        identificador1 = timeStamp.indexOf(':'); //achando hora
        temporario = timeStamp.substring (0,identificador1);
        tinfo.tm_hour = temporario.toInt();
        identificador2 = timeStamp.indexOf(':', identificador1 + 1); //achando minuto
        temporario =  timeStamp.substring (identificador1+1,identificador2);
        tinfo.tm_min = temporario.toInt(); 
        temporario = timeStamp.substring (identificador2 + 1,identificador2 + 3);
        tinfo.tm_sec = temporario.toInt();
        Serial.println("relogio tinfo.tm_sec: " + String (temporario.toInt()) + " " + String (tinfo.tm_sec));
        Serial.println ( "tm_hour0: " + String(tinfo.tm_hour) + " tm_min: " +  String(tinfo.tm_min) + " tm_sec: " +  String(tinfo.tm_sec));
        Serial.println ( "tm_tm_mday0: " + String(tinfo.tm_mday) + " tm_mon: " +  String(tinfo.tm_mon) + " tm_year " +  String(tinfo.tm_year));
        // Convert time structure to timestamp
        initialt = timelib_make(&tinfo);
        //Serial.println("relogio initialt: " + String (initialt));
        
        // Set system time counter
        timelib_set(initialt);
       
        // Configure the library to update / sync every day (for hardware RTC)
        timelib_set_provider(time_provider, TIMELIB_SECS_PER_DAY);
        
        //pega a hora do relogio interno
        now = timelib_get();
        
        // Convert to human readable format
        timelib_break(now, &tinfo);
        
        // Send to serial port
        horario = String(tinfo.tm_hour) + ":" + String(tinfo.tm_min) + ":" + String(tinfo.tm_sec);
        dia = String(tinfo.tm_mday) + "/" + String(tinfo.tm_mon) + "/" + String(tinfo.tm_year);
        Serial.println ("Hora : " + horario);
        Serial.println ("Data : " + dia);
        
      } // if (identificador
    }   //if (msgrecebida != "")
    vTaskDelay (5000); //verifica se tem atualizacao de horario a cada 5 s
  } //loop
}
