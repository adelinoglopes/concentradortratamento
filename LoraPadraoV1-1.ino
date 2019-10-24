//Rotinas padrao para o Lora Projetos Ober

//rotina que inicializar o controlador lora
void lorainit(){
  int counter = 1;
  pinMode(25,OUTPUT); //placas estao no pino 25 padrao Lora
  while (!Serial);
  SPI.begin(SCK,MISO,MOSI,SS); //biblioteca Lora
  LoRa.setPins(SS,RSTL,DI0);
  Serial.println("LoRa ativando");
// BAND Frequencia do radio - podemos utilizar ainda : 433E6, 868E6, 915E6

  while (!LoRa.begin(BAND,PABOOST)) // trabalha em 4 bandas, estamo usando a 433Mhz PABOOST potencia no maximo
  {  
    if (counter > 30) //verifica 30 vezes de Lora ativou
      break;
    counter++;
    delay (1000);
    }
  if (!LoRa.begin(BAND,PABOOST)) {
    Serial.println("Starting LoRa failed!");
  }
  else 
  {
      LoRa.setSyncWord(SYNCWORD);           // ranges from 0-0xFF, default 0x34, see API docs
      LoRa.setSpreadingFactor(SPREADINGFACTOR);  // ranges from 6-12,default 7 see API docs
      //LoRa.setTxPower(13);
      //LoRa.enableCrc(); // habilita o check de validade do pacote 
      //LoRa.receive(); 
      Serial.println("Starting LoRa OK!");   
  }
  envio = String (NOME_DISPOSITIVO) + "," + String (VERSAO) + "," +  String ("Starting LoRa OK! ") + "," + IPWifi;
  for(int i=0;i<3;i++)        //envia 3 vezes
    {
      lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA); //ao iniciar manda mensagem avisando que ativou
    }

    
 /*   
   //rotina loraping. NAO ATIVAR NAO DESCOBRI POR QUE TRAVA
     if (NOME_DISPOSITIVO != "Concentrador")  {
      Serial.println("loraping criando...");
       xTaskCreatePinnedToCore(
        LoraPing, // Function to implement the task 
        "LoaraPing", // Name of the task 
        8192,  // Stack size in words 
        NULL,  // Task input parameter 
        10,  // Priority of the task 
        &Task3,  // Task handle. 
        0); // Core where the task should run no mesmo cor que roda o Lora
    } 
    */
    delay (1000);
      
}

//rotina que transmite o pacote lora, dado a ser tranmito, placa de origem e placa destino
// cada placa lora foi definido um codigo hexa para identificar ela
//normalmente o destino e o concentrador
void lorasend(String outgoing, byte origem, byte destino)
{
  
  whatdogtimerreset();
  while (recebendolora) //somente transmite se nao estiver na rotina de recepcao do lora
  {
      Serial.println("Esta recebendo, aguardar para poder transmitir"); 
      whatdogtimerreset();
      delay (1000);
    }
      portENTER_CRITICAL (&myMutex); //nao deixa fazer mudanÃ§a de processamento
      transmitindolora = true;
      outgoing = outgoing + "," + String(contador);
      LoRa.beginPacket();                   // start packet
      LoRa.write(destino);              // add destination address
      LoRa.write(origem);             // add sender address
      LoRa.write(msgCount);                 // add message ID
      LoRa.write(outgoing.length());        // add payload length
      LoRa.print(outgoing);                 // add payload
      LoRa.endPacket();                     // finish packet and send it
      //LoRa.receive();
      Serial.print("Lora send: "); 
      Serial.println(outgoing); 
      msgCount++;                           // increment message ID
      transmitindolora = false;
      whatdogtimerreset();
      portEXIT_CRITICAL (&myMutex); //terminou a trava de mudanÃ§a de processamento
}

//rotina para receber pacotes se for destinado ao respectivo lora
//normalmente quem recebe e o concentrador

String lorareceive( byte localAddress)
{
  
  //long freqErr = 0;
  //LoRa.receive();
  while (transmitindolora) //somente ira receber quando parar de transmitir
  {
    Serial.println("Esta trasmitindo, aguardar para poder receber"); 
    delay (1000);
  }
 portENTER_CRITICAL (&myMutex);
  recebendolora = true;
  String incoming = "";
  //Serial.println("Lorareceive");
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    //Serial.println("Pacote > 0"); 
    // read packet header bytes:
    int recipient = LoRa.read();          // recipient address
    byte sender = LoRa.read();            // sender address
    byte incomingMsgId = LoRa.read();     // incoming msg ID
    byte incomingLength = LoRa.read();    // incoming msg length
    //Serial.print("Pacote...");
    //Serial.println (packetSize);
    //Serial.print("recipient...");
    //Serial.println (String(recipient)); 
    //Serial.print("Sender...");
    //Serial.println (String(sender));
    //Serial.print("incomingMsgId...");
    //Serial.println (String(incomingMsgId));
    //Serial.print("incomingLength...");
    //Serial.println (String(incomingLength)); 
    //delay (5000);
    //LoRa.receive(); 
    while (LoRa.available()) //ler os caracteres da interface lora
    {
      incoming += (char)LoRa.read();
      //Serial.println("recebendo...");
    }
    
    if (incomingLength != incoming.length()) //verifica se o tamanho esta correto
    {   // check length for error
      //Serial.println("error: message length does not match length");
      recebendolora = false;
      incoming = "";
      whatdogtimerreset();
      portEXIT_CRITICAL (&myMutex);
      return incoming;                             // skip rest of function
    }
    
    // if the recipient isn't this device or broadcast,
    //0xFF Ã© brodcast enviado para todos os Lora
    //somente processa o pacote se o destino foi o codigo Lora e este
    if (recipient != localAddress && recipient != 0xFF) {
      //Serial.println("This message is not for me.");
      recebendolora = false;
      incoming = "";
      whatdogtimerreset();
      portEXIT_CRITICAL (&myMutex);
      return incoming;                            // skip rest of function
    }
    
    // if message is for this device, or broadcast, print details:
    //Serial.println("Received from: 0x" + String(sender, HEX));
    //Serial.println("Sent to: 0x" + String(recipient, HEX));
    //Serial.println("Message ID: " + String(incomingMsgId));
    //Serial.println("Message length: " + String(incomingLength));
    //Serial.println("Message: " + incoming);
    //Serial.println("RSSI: " + String(LoRa.packetRssi()));
    //Serial.println("Snr: " + String(LoRa.packetSnr()));
    //Serial.println();
    recebendolora = false;
    //Serial.println("LoraRecive: " + String (incoming));
    whatdogtimerreset();
    portEXIT_CRITICAL (&myMutex);
    return incoming;
  }

  recebendolora = false;
  incoming = "";
  portEXIT_CRITICAL (&myMutex);
  return incoming;
}

//rotina que fica enviando uma mensagem a cada 1 min avisando que estou vivo
void LoraPing(void * parameter){
  char ROTINA_EXECUTADA [15] = "LoraPing";
    
  //Serial.println("Entrei LoraPing");
  //uint8_t templora = 0;
  for(;;){ //loop infinito da rotina
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    
    envio = String (NOME_DISPOSITIVO) + "," + String (VERSAO) + "," + String (" Heap: ")  + String (uxHighWaterMark) + "," + String ("Estou Ativo! ") + "," + IPWifi;
    lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA); //fica a cada minuto avisando que esta viv
    //Serial.println("Ping Serial: " + envio + "Contador: " + String (contador));
    vTaskDelay (300000);   //a cada 5 min   
  }  
}
