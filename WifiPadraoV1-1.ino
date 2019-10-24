//Rotinas padrao para o Wifi Projetos Ober

void iniciawifi(){
  //Inicia WiFi com o SSID e a senha
  int counter = 1;
  char enderecodns [20];
  unsigned int lastStringLength = 0;
  //char sufixo [7] = ".local";
  int n;
  
  IPAddress ipinterno;
  Serial.println("Procurando Wifi.....");
  Serial.print("Rede: ");Serial.println(WIFISSID);
  n = WiFi.scanNetworks(); //procura as redes que esta escutando em volta
  if (n == 0) {
      Serial.println("no networks found");
  } else {
      Serial.print(n); // mostra na serial as redes que esta escutando
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          if (String (WiFi.SSID(i)) == String (WIFISSID)) //identifica se a rede que se quer conectar esta presente
            {
            Serial.println ("Rede Produdada Presente");
            Serial.print (WiFi.RSSI(i));
            }
          }
      }
  WiFi.disconnect(true);
  IPWifi = "";
  WiFi.setAutoConnect(true);
  if (IPfixo) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
  }
  WiFi.begin(WIFISSID, PASSWORD); //qual rede e qual senha quer se conectar. vem do arquivo parametro
  while (WiFi.status() != WL_CONNECTED)
  {   
    counter++;
	Serial.print("Conectando Wifi: ");
    Serial.println(counter);								  
    if (counter > 10) { //tenta 20 vezes se conectar
      break;
    }
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    ipinterno = WiFi.localIP();
    IPWifi = String(ipinterno[0]) + '.' + String(ipinterno[1]) + '.' + String(ipinterno[2]) + '.' + String(ipinterno[3]);
    lastStringLength = NOME_DISPOSITIVO.length(); 
    NOME_DISPOSITIVO.toCharArray(enderecodns, lastStringLength+1);
    MDNS.begin(enderecodns);
    if (MDNS.begin(enderecodns)) {
        MDNS.addService("_iot", "_tcp", SERVER_PORT);
    }
 }
}

//rotina que inicializa a rede wifi da placa
void wifiinit(){
  //Inicia WiFi com o SSID e a senha

  iniciawifi();
   //rotina que ira fazer o registro do nome de forma continua MDNS
  xTaskCreatePinnedToCore(
    MDNSCLIENT, // Function to implement the task 
    "MDNSCLIENT", // Name of the task 
    8192,  // Stack size in words 
    NULL,  // Task input parameter 
    10,  // Priority of the task 
    &Task11,  // Task handle. 
    0); // Core where the task should run no mesmo cor que roda o Lora
 delay (1000);
}

//caso tenha perdido o wifi tenta reconectar
//apesar do parametro autoreconect true
void reconnectwifi() {
  WiFi.disconnect(true);
  WiFi.setAutoConnect(true);
  if (IPfixo) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("STA Failed to configure");
    }
  }
  WiFi.begin(WIFISSID, PASSWORD); //qual rede e qual senha quer se conectar. vem do arquivo parametro
}

void wifiserverinit (){ //usado no contredor, sobe o servidor wifi para receber mensagem do loratorre no ip que esta o concentrador
  WiFiServer serverwifi(SERVER_PORT);
  WiFiClient serverClients[MAX_SRV_CLIENTS];
  serverwifi.begin();
}

void procuraconcentrador (){
  int counter = 0;
  IPAddress ipinterno;
  String IP;
  //unsigned int lastStringLength = 0;
  //char enderecodns [20];

  ConcentradorDNS.trim();
  IPAddress serverIp = MDNS.queryHost(ConcentradorDNS); //procurando o ip do concentrador
  while (serverIp.toString() == "0.0.0.0") {
    serverIp = MDNS.queryHost(ConcentradorDNS);
    IP = "0.0.0.0";
    IP.toCharArray(ConcentradorIP, 16);
    counter++;
    if (counter > 10) {//tenta se conectar 10 vezes
      IP = "192.168.0.50";
      IP.toCharArray(ConcentradorIP, 16);
      return;
    }
    delay(100);                     
  }
  IP = String(serverIp[0]) + '.' + String(serverIp[1]) + '.' + String(serverIp[2]) + '.' + String(serverIp[3]);
  IP.toCharArray(ConcentradorIP, 16);
}

void wificlientinit (){
  
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect(true);
      iniciawifi();
    }
    if (IPWifi = "0.0.0.0") {
      WiFi.disconnect(true);
      iniciawifi();
    } 
    if (WiFi.status() == WL_CONNECTED){
      procuraconcentrador ();
    }       
    
   //rotina que ira fazer a procura do ip do concentrador
   xTaskCreatePinnedToCore(
    CONCENTRADORST, // Function to implement the task 
    "CONCENTRADORST", // Name of the task 
    8192,  // Stack size in words 
    NULL,  // Task input parameter 
    10,  // Priority of the task 
    &Task12,  // Task handle. 
    0); // Core where the task should run no mesmo cor que roda o Lora 
  delay (1000);
}

void wifisend(String envio){ 
  int contador = 0;
  char mensagem [300] = "";
  IPAddress ipinterno;
  //unsigned int lastStringLength = 0;
  //char enderecodns [20];

  whatdogtimerreset();
  WiFiClient clientewifi;
  
  //Se nÃ£o conseguiu se conectar entÃ£o retornamos
  while (!clientewifi.connect(ConcentradorIP, SERVER_PORT)){
      contador++;   
      if ( (contador == 5) || (contador == 10) || (contador == 15) ) {
        if (WiFi.status() != WL_CONNECTED) {
          WiFi.disconnect(true);
          //Serial.print("WL_CONNECTED Wifi: "); 
          iniciawifi();
        }
        if (IPWifi = "0.0.0.0") {
          WiFi.disconnect(true);
          //Serial.print("0000 Wifi: "); 
          iniciawifi();
        } 
        if (WiFi.status() == WL_CONNECTED){
          procuraconcentrador ();
        }       
      }
      if (contador > 5) {//tenta se conectar 10 vezes
        return;
      }
      delay(100);
  }
  if ((clientewifi.connect(ConcentradorIP, SERVER_PORT)) && (WiFi.status() == WL_CONNECTED)){
    envio.toCharArray(mensagem,300); //envia para a mensagem para o concentrador
    clientewifi.write(mensagem);
    clientewifi.flush();
    clientewifi.stop();
    Serial.print("Mensagem Wifi: "); 
    Serial.println(mensagem); 
  }else {
    Serial.println("Nao se conectou ao concentrador ");
  }
}

String wifirecive() 
{         
   String mensagem = "";
   char caracter;

  whatdogtimerreset();
  serverwifi.begin();
  //Verifica se tem algum cliente se conectando
  WiFiClient clientrecive = serverwifi.available();
  if (clientrecive) 
  {    
      //Se o cliente tem dados que deseja nos enviar
      while (clientrecive.available())
      {
          caracter = clientrecive.read();
          mensagem = mensagem + caracter;
      }
      //Fecha a conexÃ£o com o cliente
      clientrecive.stop();
  }
  return mensagem; 
}


//rotina que fica verificando wifi e registrando o nome na rede
void MDNSCLIENT(void * parameter){
  //unsigned int lastStringLength = 0;
  //char enderecodns [20];
  //int counter = 1;
  IPAddress ipinterno;
  char msglog [100] = "";
  char ROTINA_EXECUTADA [15] = "MDNSCLIENT";
    
  for(;;){ //loop infinito da rotina
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    if (WiFi.status() != WL_CONNECTED){
      iniciawifi();
    }      
    if (IPWifi == "0.0.0.0"){
      iniciawifi();
    } 
    if (IPWifi == ""){
      iniciawifi();
    }                
    envio = String (NOME_DISPOSITIVO) + "," + String (VERSAO) + "," + String ("MDNS Client :") + IPWifi + String (" Heap: ") + String (uxHighWaterMark);
    lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA); //fica a cada minuto avisando que esta vivo
    if ((NOME_DISPOSITIVO == "Torre1") || (NOME_DISPOSITIVO == "Torre2"))  {
      envio.toCharArray(msglog,100);
      if (WiFi.status() == WL_CONNECTED){
        wifisend(envio);
      }
    } 
    if (NOME_DISPOSITIVO == "Concentrador") {
      envio.toCharArray(msglog,100);
      sendValuesmqttshttp (contador, msglog, VARIABLE_LABEL_LOGCON);
    }                      
    vTaskDelay (120000);   //a cada 2 min  
    }   
}

//rotina que fica procurando o concentrador
void CONCENTRADORST(void * parameter){
  //unsigned int lastStringLength = 0;
  //char enderecodns [20];
  //int counter = 1;
  IPAddress ipinterno;
  char msglog [100] = "";
  char ROTINA_EXECUTADA [15] = "CONCENTRADORST";
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
  for(;;){ //loop infinito da rotina
    
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    if (WiFi.status() != WL_CONNECTED){
      iniciawifi();
    }      
    if (IPWifi == "0.0.0.0"){
      iniciawifi();
    } 
    if (IPWifi == ""){
      iniciawifi();
    } 
    if (WiFi.status() == WL_CONNECTED){
      procuraconcentrador ();
    }
    envio = String (NOME_DISPOSITIVO) + "," + String (VERSAO) + "," + String ("Client :") + IPWifi + String (" Concentrador :") + ConcentradorIP + String (" Heap: ") + String (uxHighWaterMark);
    lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA); 
    if ((NOME_DISPOSITIVO == "Torre1") || (NOME_DISPOSITIVO == "Torre2"))  {
      envio.toCharArray(msglog,100);
      if (WiFi.status() == WL_CONNECTED){
        wifisend(envio);
      }
    } 
    if (NOME_DISPOSITIVO == "Concentrador") {
      envio.toCharArray(msglog,100);
      sendValuesmqttshttp (contador, msglog, VARIABLE_LABEL_LOGCON);
    };
    vTaskDelay (240000);   //a cada 2 min 
  }
}
