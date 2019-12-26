//whatdog init
void whatdoginit(){
    reinicializaesp = true;
    loopTime = millis();
    timeligou = millis();
       //rotina que veritifica se esta ativo e caso contrario whatdog
     xTaskCreatePinnedToCore(
      whatdog, // Function to implement the task 
      "Whatdog", // Name of the task 
      8192,  // Stack size in words 
      NULL,  // Task input parameter 
      10,  // Priority of the task 
      &Task7,  // Task handle. 
      0); // Core where the task should run no mesmo cor que roda o Lora
      Serial.println("Whatdog .....Iniciando Server");
      delay (1000);
  }

  //Verifica se estou travado whatdog
void whatdog(void * parameter){
  char msglog [50] = ""; 
  float tempocorrido; 
  char ROTINA_EXECUTADA [15] = "whatdog";
    
  for(;;){ //loop infinito da rotina
    //Serial.println("Whatdog .....Loop");
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    tempocorrido = millis() - loopTime;
    //Serial.println ("LoopTime: " + String(loopTime) + " Tempocorrido: " + String (tempocorrido) + " Timereset: " + String (timereset)); 
    if ((tempocorrido > timereset) && (reinicializaesp)) {
        //Serial.println("Reeset Whatdog");
        envio = String (VARIABLE_LABEL_WHATDOG) + "," + String (NOME_DISPOSITIVO) + "," + String (VERSAO) + "," + String (" Tempo Ligado:") + "," + String (tempocorrido); //tentar enviar que deu whatdog
        for(int i=0;i<3;i++)        //envia 3 vezes
        {
          lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
        }
        if ((NOME_DISPOSITIVO == "Torre1") || (NOME_DISPOSITIVO == "Torre2"))  {
          envio.toCharArray(msglog,50);
          if (WiFi.status() == WL_CONNECTED){
            wifisend(envio);
          }
        } 
        if (NOME_DISPOSITIVO == "Concentrador") {
          envio.toCharArray(msglog,50);
          sendValuesmqttshttp (contador, msglog, VARIABLE_LABEL_LOGCON);
        }
        reiniciarele (); 
        reinicia();
    } 
    
    tempocorrido = millis() - timeligou; //a ideia e reinicar a cada 24:00
    //Serial.println ("Timeligado: " + String(timeligado) + " Tempocorrido: " + String (tempocorrido) + " Timereset: " + String (timereset)); 
    if ((tempocorrido > timeligado) && (reinicializaesp)) {
        //Serial.println("Reeset Whatdog Diario");
        envio = String (VARIABLE_LABEL_WHATDOG) + "," + String (NOME_DISPOSITIVO) + "," + String (VERSAO) + String (" Tempo Ligado:") + "," + String (tempocorrido); //tentar enviar que deu whatdog
        for(int i=0;i<3;i++)        //envia 3 vezes
        {
          lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
        }
        if ((NOME_DISPOSITIVO == "Torre1") || (NOME_DISPOSITIVO == "Torre2"))  {
          envio.toCharArray(msglog,50);
          if (WiFi.status() == WL_CONNECTED){
            wifisend(envio);
          }
        } 
        if (NOME_DISPOSITIVO == "Concentrador") {
          envio.toCharArray(msglog,50);
          sendValuesmqttshttp (contador, msglog, VARIABLE_LABEL_LOGCON);
        }
        reiniciarele (); // no 24h ira fazer primeiro o rele, se nao tiver o rele vai para o reinicia
        reinicia();
      }
    vTaskDelay (300000); //verifica a cada 5 minuto          
  }  
}

void whatdogtimerreset(){
    loopTime = millis();
    //Serial.println ("WhatDog Reset TimeReset: " + String (loopTime)); 
}

void reinicia (){
        Serial.println ("WhatDog Reset ");
        //portENTER_CRITICAL (&myMutex);
        if( Task1 != NULL ) //ota
        {
            //vTaskDelete(Task1); //se resetar o ota ele para o processo
            Serial.println ("Reset Task1 OTA:");
        }
        if( Task3 != NULL ) //loraping
        {
            vTaskDelete(Task3);
            Serial.println ("Reset Task3 LoraPing:");
        }
        if( Task4 != NULL ) //GPS Ping
        {
            vTaskDelete(Task4);
            Serial.println ("Reset Task4 GPSPing:");
        }
        if( Task5 != NULL ) //loop NTP
        {
            vTaskDelete(Task5);
            Serial.println ("Reset Task5 NTP pega hora:");
        } 
        if( Task6 != NULL ) //loop relogio
        {
            vTaskDelete(Task6);
            Serial.println ("Reset Task6 NTP Atualiza:");
        }
        if( Task8 != NULL ) //GPSWhatdog
        {
            vTaskDelete(Task8);
            Serial.println ("Reset Task8 GPSWhatdog");
        }
        if( Task9 != NULL ) //Vazao
        {
            vTaskDelete(Task9);
            Serial.println ("Reset Task9 Leitura Vazao");
        }  
        if( Task10 != NULL ) //Vazao
        {
            vTaskDelete(Task9);
            Serial.println ("Reset Task10 NTP Relocio Receb");
        }              
        if( Task11 != NULL ) //MDNSCLIENT 
        {
            vTaskDelete(Task11);
            Serial.println ("Reset Task11 MDNSCLIENT ");
        }                
        if( Task12 != NULL ) //CONCENTRADORIP
        {
            vTaskDelete(Task12);
            Serial.println ("Reset Task12 CONCENTRADORIP ");
        }   
        if( Task13 != NULL ) //torreenvio
        {
            vTaskDelete(Task12);
            Serial.println ("Reset Task13 torreenvio ");
        }  
        if( Task14 != NULL ) //concentrador envio
        {
            vTaskDelete(Task12);
            Serial.println ("Reset Task14 concentrador envio ");
        }                                          
        //WiFi.setAutoConnect(false);
        //WiFi.disconnect(true);
        Serial.println ("WhatDog restart ");
        reiniciarele ();
        ESP.restart();  //tells the SDK to reboot, so its a more clean reboot ESP.restart(). ESP.reset() is a h
        //portEXIT_CRITICAL (&myMutex);
}

void reiniciarele (){
  digitalWrite(33, HIGH); //liga o rele ai vai reiniciar
  }

void sendHeap( char objeto [20], UBaseType_t valor) {            
    
    if (valor < 200) {
      envio = String (VARIABLE_LABEL_HEAP) + "," + String (NOME_DISPOSITIVO) + "," + String(objeto) + "," + String (valor);
      for(int i=0;i<3;i++)        //envia 3 vezes
        {
          lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
        }
    }
}
