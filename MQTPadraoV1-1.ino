//Rotinas padrao para o Mqt-Ubidots Projetos Ober

// incializa o protocolo MQTT, este e um protocolo Json para enviar para o UBIDOTS
void mqttinit()
{
  //Seta servidor com o broker e a porta
  int counter = 1;
  Serial.print("MQTT - Conectando.. ");
  Serial.println(SERVER);
  client.setServer(SERVER, PORT); //server  e porta do ubidots
  
  //Conecta no ubidots com o Device id e o token, o password Ã© informado como vazio
  while(!client.connect(DEVICE_ID, TOKEN, "")) 
  {
      Serial.print("MQTT - Connect error ");
      //Serial.println (counter);
      counter++;
      
      if (counter > 20) //tenta se conectar 10 vezes
        break;
      delay (1000);
  }
  if (!client.connect(DEVICE_ID, TOKEN, "")){
    Serial.println("MQTT - Error");
  }
  delay (1000);
}

//fica verificando se esta conectado e reconectar
void reconnectmqtt() 
{  
  //Loop atÃ© que o MQTT esteja conectado
  int counter = 1;
  //reconnectwifi();
  while (!client.connected()) 
  {     
    //Tenta conectar
/*    if (client.connect(DEVICE_ID, TOKEN,""))
      { 
        Serial.println("MQTT Reconnected");
      }
    else 
    {
      Serial.print("failed Reconnected MQTT, rc=");
      //Serial.print(client.state());
      //Serial.println("Tentativa: " + String(counter));
    }*/
      delay(500);
      counter++;
      //reconnectwifi();
      if (counter > 10){
        //Serial.println("Tentando ReConectar Falhou MQTT..");
        break;
      }
    } 
}

//Envia valores por mqtt
//Envia um valor de um tipo para o Ubidots em formato json
void sendValuesmqtt(float valor, char tipo [15])
{
  char json[250]; //dado que sera enviado para o Ubidots.
  whatdogtimerreset();
 
  sprintf(json,  "{\"%s\":{\"value\":%02.02f, \"%s\":{\"%s\":%02.02f}}}", tipo, valor, tipo, tipo, valor);  
  Serial.print("json: "); 
  Serial.println(json);

  //Se o esp foi desconectado do ubidots
  if(!client.connected())
    reconnectmqtt();
  
  if(!client.publish(TOPIC, json)){
    Serial.println("MQTT Transmissao falhou"); 
  }
  whatdogtimerreset();
}

//Envia valores por mqtt String
//Exemplo: {"temperature":{"value":24.50, "context":{"temperature":24.50, "humidity":57.20}}}
//Envia um valor de um tipo para o Ubidots em formato json
//envio de uma string como valor
void sendValuesmqtts(float contador, char valor [50] , char tipo [15])
{
  char json[250]; //dado quechar sera enviado para o Ubidots.
  whatdogtimerreset();
  
 //ira subistituir % pelos na sequencia informada no final do comando 
    sprintf(json,  "{\"%s\":{\"value\":%02.0f, \"context\":{\"%s\":\"%s\"}}}", tipo, contador, tipo, valor); 
  //Serial.print("Tipo: ");
  //Serial.println(tipo);
  Serial.print("json: "); 
  Serial.println(json);

  //Se o esp foi desconectado do ubidots
  if(!client.connected())
    reconnectmqtt();

  if(!client.publish(TOPIC, json)){
    Serial.println("MQTT Transmissao falhou"); 
  }
  whatdogtimerreset();
}

//envia dois valores no formato Json
void sendValuesmqttmapa(float contador,float valor1, float valor2, char titulo [15], char tipo1 [15], char tipo2 [15])
//bool sendValues(float temperature, float humidity)
//Exemplo: {"temperature":{"value":24.50, "context":{"temperature":24.50, "humidity":57.20}}}
//{"value":1, context:{"lat": 6.4, "lng":-75.2}}
{
  char json[250];

  whatdogtimerreset();
  //Atribui para a cadeia de caracteres "json" os valores referentes a temperatura e os envia para a variÃ¡vel do ubidots correspondente
  //sprintf(json,  "{\"value\":%02.02f, \"context\":{\"%s\":%02.06f, \"%s\":%02.02f}}", contador, tipo1, valor1, tipo2, valor2);  
  sprintf(json,  "{\"%s\":{\"value\":%02.0f, \"context\":{\"%s\":%02.08f, \"%s\":%02.08f}}}", titulo, contador, tipo1, valor1, tipo2, valor2);  
  Serial.print("json: "); 
  Serial.println(json);

  //Se o esp foi desconectado do ubidots
  if(!client.connected())
    reconnectmqtt();
  
  if(!client.publish(TOPIC, json)){
    Serial.println("MQTT Transmissao falhou"); 
  }
  whatdogtimerreset();
}

void mqttinithttp(){

  if (ubidots.connect(SERVER, PORT)) {
    Serial.println("connected to Ubidots cloud http");
  } else {
    Serial.println("could not connect to Ubidots cloud http");
  }
  delay (1000);
}

void SendToUbidotsHTTP(char* payload) {

  int contentLength = strlen(payload);
  int contador = 0;
  /* Connecting the client */
  while (!ubidots.connect(SERVER, PORT)) {
    ubidots.stop();
    ubidots.connect(SERVER, PORT);
    delay (200);
    contador++;
    if (contador > 10) {
      ubidots.stop();
      ubidots.connect(SERVER, PORT);
      break;
    }
  }
  if (ubidots.connected()) {
    /* Builds the request POST - Please reference this link to know all the request's structures https://ubidots.com/docs/api/ */
    ubidots.print(F("POST /api/v1.6/devices/"));
    ubidots.print(DEVICE_LABEL);
    ubidots.print(F(" HTTP/1.1\r\n"));
    ubidots.print(F("Host: "));
    ubidots.print(SERVER);
    ubidots.print(F("\r\n"));
    ubidots.print(F("User-Agent: "));
    ubidots.print(USER_AGENT);
    ubidots.print(F("/"));
    ubidots.print(VERSION);
    ubidots.print(F("\r\n"));
    ubidots.print(F("X-Auth-Token: "));
    ubidots.print(TOKEN);
    ubidots.print(F("\r\n"));
    ubidots.print(F("Connection: close\r\n"));
    ubidots.print(F("Content-Type: application/json\r\n"));
    ubidots.print(F("Content-Length: "));
    ubidots.print(contentLength);
    ubidots.print(F("\r\n\r\n"));
    ubidots.print(payload);
    ubidots.print(F("\r\n"));
  } else {
    Serial.println("Connection Failed ubidots - Try Again");
  }

  /* Reach timeout when the server is unavailable */
  int timeout = 0;
  while (!ubidots.available() && timeout < 5000) {
    timeout++;
    delay(1);
    if (timeout >= 5000) {
      Serial.println(F("Error, max timeout reached"));
      break;
    }
  }
  
  /* Reads the response from the server */
  //Serial.println(F("\nUbidots' Server response:\n"));
  while (ubidots.available()) {
    char c = ubidots.read();
    //Serial.print(c); // Uncomment this line to visualize the response on the Serial Monitor
  }

  /* Disconnecting the client */
  //ubidots.stop();
}
/* esta rotina ainda nao esta pronta
void envioubidots(void * parameter){
  int inicio, fim,i;
  char msglog[250];
  char ROTINA_EXECUTADA [15] = "torreenvio";
    
  for(;;){ //loop infinito da rotina
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    inicio = filaenvio - 1; //fila envio e ate onde ja foi enviado para o concentrador
    fim = filapendente;  //fila pendente e o que falta enviar que esta na varial buf[]

  if (inicio > fim) {
    fim = 51;
    filapendente =0;
  } 
  Serial.print("concetrador envio ");
  Serial.print(" filaenvio: ");
  Serial.print(filaenvio);
  Serial.print(" filapendente: ");
  Serial.print(filapendente); 
  Serial.print(" inicio: ");
  Serial.print(inicio); 
  Serial.print(" fim: ");
  Serial.println(fim); 
  //if (inicio >= 0 ){ //somente vou enviar se tiver dado
    for(i=inicio;i<fim;i++)        //sort the analog from small to large
    {
      buf [i].toCharArray(msglog,150);
      SendToUbidotsHTTP(msglog);
      Serial.print("concetrador envio ");
      Serial.print(" i: ");
      Serial.println(i);
      filaenvio = i - 1;
    }
  //}
  vTaskDelay (1000); //1min
  }  //loop
}*/

//Envia valores float por mqtt versao http
void sendValuesmqtthttp(float valor, char tipo [15])
{
  char payload[250];
  char str_val_1[30];
  
  whatdogtimerreset();
  dtostrf(valor, 4, 2, str_val_1);
  sprintf(payload, "{\"");        
  sprintf(payload, "%s%s\":%s", payload, tipo, str_val_1);       
  sprintf(payload, "%s}", payload);  

  Serial.print("payload: "); 
  Serial.println(payload);
  
  SendToUbidotsHTTP(payload);
  /* preciso depois continuar os testes para fila. por hora nao da volta o que ja foi bem testado
  buf [filapendente] = payload;
  filapendente++;
  if (filapendente > 50) {
    filapendente=0;}
   */ 
  whatdogtimerreset();
}

//Envia valores por mqtt String
//envio de uma string como valor
void sendValuesmqttshttp(float contador, char valor [150] , char tipo [15])
{
  char payload[350];
  char str_val_1[250];
  
  whatdogtimerreset();

  sprintf(str_val_1,  "{\"value\":%01.0f, \"context\":{\"%s\":\"%s\"}}", contador, tipo, valor); 
 
  sprintf(payload, "{\"");        
  sprintf(payload, "%s%s\":%s", payload, tipo, str_val_1);       
  sprintf(payload, "%s}", payload);  

  Serial.print("payload: "); 
  Serial.println(payload);
  
  SendToUbidotsHTTP(payload);
  whatdogtimerreset();
}

//envia dois valores para o mapa
void sendValuesmqttmapahttp(float contador,float valor1, float valor2, char titulo [15], char tipo1 [15], char tipo2 [15])
{
  char payload[250];
  char str_val_1[200];

  whatdogtimerreset();

  sprintf(str_val_1,  "{\"value\":%01.0f, \"context\":{\"%s\":%02.08f, \"%s\":%02.08f}}", contador, tipo1, valor1, tipo2, valor2); 

  sprintf(payload, "{\"");        
  sprintf(payload, "%s%s\":%s", payload, titulo, str_val_1);       
  sprintf(payload, "%s}", payload);

  Serial.print("payload: "); 
  Serial.println(payload);
  
  SendToUbidotsHTTP(payload);
 
  whatdogtimerreset();
}


float GetToUbidotsHTTP(char* variable_label) {
//https://help.ubidots.com/en/articles/513294-connect-a-linkit-one-to-ubidots-using-wi-fi-over-http
  int value_index;
  String value_string;
  String value;
  int contador = 0;
  int timeout = 0;
  String response = "";
  char payload[250];

  /* Connecting the client */
  while (!ubidots.connect(SERVER, PORT)) {
    ubidots.stop();
    ubidots.connect(SERVER, PORT);
    delay (200);
    contador++;
    if (contador > 10) {
      ubidots.stop();
      ubidots.connect(SERVER, PORT);
      break;
    }
  }
  if (ubidots.connected()) {
    // Build HTTP GET request
    ubidots.print(F("GET /api/v1.6/devices/"));
    ubidots.print(DEVICE_LABEL);
    ubidots.print(F("/"));
    ubidots.print(variable_label);
    ubidots.print(F("/values/?page_size=1&token="));
    ubidots.print(TOKEN);
    ubidots.println(F(" HTTP/1.1"));
    ubidots.println(F("Content-Type: application/json"));
    ubidots.print(F("Host: "));
    ubidots.println(SERVER);
    ubidots.println();
    int count = 0;
    while (!ubidots.available()  && count < 10000) {
      count++;
      delay(1);
    }

    int v;
    while (ubidots.available()) {
      v = ubidots.read();
      if (ubidots < 0) {
        Serial.println("No response.");
        break;
      }
      //Serial.println((char)v);
      response.concat((char)v);
    }
    //Serial.println("Printing response:");
    value_index = response.indexOf("\"value\": ");
    value_string = response.substring(value_index);
    value = value_string.substring(9, value_string.indexOf(","));
    //Serial.println(value);
    return atof(value.c_str());
  }
  else {
    Serial.println("Couldn't connect to Ubidots");
    return -1;
  } 
  whatdogtimerreset();
}
