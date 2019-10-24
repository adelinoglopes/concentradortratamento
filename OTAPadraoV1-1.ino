//Rotinas padrao para o OTA Projetos Ober

//OTA e o processo de atualizar por uma pagina web o programa que esta no Lora
//Esta rotina configura o servidor, indicado pagina principal e a de carga, que sao variaveis

void setupOTAWEB(){
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    reinicializaesp = false; //desliga o whatdog
    //Serial.println("OTA ServerINDEX");
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    reinicializaesp = false; //desliga o whatdog
    //Serial.println("OTA UPDATE");
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    reinicia();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      reinicializaesp = false; //desliga o whatdog
      Serial.println("OTA %");
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin(); //inicializa o server
  //Serial.println("OTA .....Iniciando Server");
}

//rotina que fica verificando se o usuario fez algo na pagina
//roda no nucleo 0 do processador
void OTAWEBServer(void * parameter){
  
  for(;;){ //loop infinito da rotina
          //Se o esp foi desconectado do wifi tenta reconectar

          server.handleClient();
          //Serial.println("OTA .....Server2");
          //uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
          
          vTaskDelay (2000); //2 segundos faz uma coleta         
  }  
}
//rotina que configura todo o ambiente ota
//configura o servidor
//coloca no nucleo 0 do processador a rotina que verifica se o usuario fez algo na  pagina
void SetubWEBServer(){
  setupOTAWEB(); //inicia o servidor web e parametriza
  xTaskCreatePinnedToCore(
      OTAWEBServer, // Function to implement the task 
      "ServidorOta", // Name of the task 
      8192,  // Stack size in words 
      NULL,  // Task input parameter 
      0,  // Priority of the task 
      &Task1,  // Task handle. 
      0); // Core where the task should run 
   delay (1000);
      
}
