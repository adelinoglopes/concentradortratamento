// Estacao tratamento UBIDOTS

//Nome do Dispositivo para o OTA
String NOME_DISPOSITIVO = "Concentrador";
String VERSAO = "160919";
#define CODIGO_DISPOSITIVO 0xFD  

//Wifi
//#define WIFISSID "ArduinoOber" //Coloque seu SSID de WiFi aqui
//#define PASSWORD "Arduino2019" //Coloque sewrqu password de WiFi aqui
#define WIFISSID "gordo" //Coloque seu SSID de WiFi aqui
#define PASSWORD "pulguento" //Coloque sewrqu password de WiFi aqui
boolean IPfixo =  false;
IPAddress local_IP(192, 168, 0, 50);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8); //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

//Includes Padrao -  Se alterar tem que alterar em todos os projetos
#include "ParametrosPadrao.h"

//NTP e Relogio - tempo
TaskHandle_t Task5; //ponteiro que Ã© alocado a rotina no procesador 0 que fica em loop NTP
TaskHandle_t Task6; //ponteiro que Ã© alocado a rotina no procesador 0 que fica em loop Relogio


float temperatura_enviada, distancia_enviada, ph_enviada, nenvio_temperatura, nenvio_distancia, nenvio_ph, nenvio_vazao, vazao_enviada = 0;
float turbides_enviada,nenvio_turbides, cinza_enviada, nenvio_cinza, nenvio_ronda =0;
float  distancia,distancia2;
String ronda_enviada  = "";

String  buf[50];
int filapendente,filaenvio = 0;


void setup() {
  Serial.begin(9600);
  Serial.println();

  Serial.println("Inicio...Concentrador");

    //conect lora
   lorainit();
   
  //conecta Wifi
  wifiinit();

  //Setup OTA
  //setupOTAWEB();
  SetubWEBServer();
  
  // conecta MQTT
  //mqttinit(); //versao antiga 
  mqttinithttp(); //versao que envia por http e nao por mqtt tem mais estabilidade
 
  //ntpserver
  //ntpinitserver();

  //relogio interno
  //relogioinit();

 //wifiserver
 wifiserverinit ();
 
  //iniciawhatdog
  whatdoginit();
/*
  //enviaubidots como task
     xTaskCreatePinnedToCore(
      envioubidots, // Function to implement the task 
      "envioubidots", // Name of the task 
      8192,  // Stack size in words 
      NULL,  // Task input parameter 
      10,  // Priority of the task 
      &Task14,  // Task handle. 
      0); // Core where the task should run 
  */    
  
  Serial.println("Concentrador...Fim Setup");
}

void loop() {

  String msgrecebida, msgrecebidalora, msgrecebidawifi = "";
  String identificador = "";
  String valorstring = "";
  String valorstring1 = "";
  String valorstringenvio = "";
  String contadorlorastring = "";
  int virgula, virgula1 =  0;
  char valorstringronda [100] = "";
  char msglog [150] = "";
  float valor = 0;
  char valorstringchar [15];
  

  //Serial.println("Concentrador...Loop");
/*
    // receber msg lora
  msgrecebidalora = lorareceive(LABORATORIOLORA);
  if (msgrecebidalora != "") {
    Serial.print("Mensagem Lora Recebida: "); 
    Serial.println(msgrecebidalora);
    }

  msgrecebidawifi = wifirecive();
  if (msgrecebidawifi != "") {
    Serial.print("Mensagem Wifi Recebida: "); 
    Serial.println(msgrecebidawifi);
    }
*/
  msgrecebidawifi = wifirecive();
  if ((msgrecebidalora != "") || (msgrecebidawifi != "")) {
    //if (msgrecebidalora != "") 
      //msgrecebida = msgrecebidalora;
    if (msgrecebidawifi != "") 
      msgrecebida = msgrecebidawifi;
    Serial.print("Mensagem Recebida: ");
    Serial.println(msgrecebida);  
    
    msgrecebida.toCharArray(msglog,150);
    sendValuesmqttshttp (contador, msglog, VARIABLE_LABEL_LOGCON);
      
    virgula = msgrecebida.indexOf(',');
    identificador =  msgrecebida.substring(0,virgula);
    virgula1 = msgrecebida.indexOf(',', virgula +1);
    valorstring = msgrecebida.substring(virgula + 1,virgula1);
    contadorlorastring = msgrecebida.substring(virgula1 + 1);
    
    if (identificador == VARIABLE_LABEL_OBJETO) {
      valor = valorstring.toFloat(); // temperatura do objeto
      if (valor != temperatura_enviada) {
        if ((valor >= 15) && (valor <=90)) {
          sendValuesmqtthttp(valor, VARIABLE_LABEL_OBJETO);
          googlesend(VARIABLE_LABEL_OBJETO, valor);
          temperatura_enviada = valor;
          nenvio_temperatura = 0;
        }
      }else {
          nenvio_temperatura = nenvio_temperatura + 1;
          if (nenvio_temperatura > 5) {
            if ((valor >= 15) && (valor <=90)) {
              sendValuesmqtthttp(valor, VARIABLE_LABEL_OBJETO);
              googlesend(VARIABLE_LABEL_OBJETO, valor);
              temperatura_enviada = valor;
              nenvio_temperatura = 0;
            }           
          }
      }
    } 

    if (identificador == VARIABLE_LABEL_DISTANCIA) {
      valor = valorstring.toFloat(); // centimetro
      if (valor != distancia_enviada) {
       if ((valor >= 5) && (valor <=900)){
        sendValuesmqtthttp(valor, VARIABLE_LABEL_DISTANCIA);
        googlesend(VARIABLE_LABEL_DISTANCIA, valor);
        distancia_enviada = valor;
        nenvio_distancia = 0;
       }       
      } else {
          nenvio_distancia = nenvio_distancia + 1;
          if (nenvio_distancia > 5) {
            if ((valor >= 5) && (valor <=900)){
              sendValuesmqtthttp(valor, VARIABLE_LABEL_DISTANCIA);
              googlesend(VARIABLE_LABEL_DISTANCIA, valor);
              distancia_enviada = valor;
              nenvio_distancia = 0;
            }
          }
      }
    } 
    
    if (identificador == VARIABLE_LABEL_PH) {
      valor = valorstring.toFloat(); // ph
      if (valor != ph_enviada ) {
        //if ((valor > 0 ) && (valor < 15)){
          sendValuesmqtthttp(valor, VARIABLE_LABEL_PH);
          googlesend(VARIABLE_LABEL_PH, valor);
          ph_enviada = valor;
          nenvio_ph =0;
        //}        
      }else {
        nenvio_ph = nenvio_ph + 1;
        if (nenvio_ph > 5) {
           sendValuesmqtthttp(valor, VARIABLE_LABEL_PH);
           googlesend(VARIABLE_LABEL_PH, valor);
          ph_enviada = valor;
          nenvio_ph =0;         
        }
      }
    }

    if (identificador == VARIABLE_LABEL_TURBIDES) {
      valor = valorstring.toFloat(); // turbides
      if (valor != turbides_enviada ) {
        //if ((valor > 0 ) && (valor < 15)){
          sendValuesmqtthttp(valor, VARIABLE_LABEL_TURBIDES);
          googlesend(VARIABLE_LABEL_TURBIDES, valor);
          turbides_enviada = valor;
          nenvio_turbides =0;
        //}        
      }else {
        nenvio_turbides = nenvio_turbides + 1;
        if (nenvio_turbides > 5) {
           sendValuesmqtthttp(valor, VARIABLE_LABEL_TURBIDES);
           googlesend(VARIABLE_LABEL_TURBIDES, valor);
          turbides_enviada = valor;
          nenvio_turbides =0;         
        }
      }
    }

    if (identificador == VARIABLE_LABEL_CHAMINECINZA) {
      valor = valorstring.toFloat(); // turbides
      if (valor != cinza_enviada ) {
        //if ((valor > 0 ) && (valor < 15)){
          sendValuesmqtthttp(valor, VARIABLE_LABEL_CHAMINECINZA);
          cinza_enviada = valor;
          nenvio_cinza =0;
        //}        
      }else {
        nenvio_cinza = nenvio_cinza + 1;
        if (nenvio_cinza > 5) {
           sendValuesmqtthttp(valor, VARIABLE_LABEL_CHAMINECINZA);
          cinza_enviada = valor;
          nenvio_cinza =0;         
        }
      }
    }
       
    if (identificador == VARIABLE_LABEL_GPS) {
      //Serial.println("Entrei GPS");
      //Lora send: bicicleta,lat,-22.771526,lng,-47.318192
      //Mensagem Lora Recebida: bicicleta,lat,-22.771551,lng,-47.318245
      virgula = msgrecebida.indexOf(',');
      virgula = msgrecebida.indexOf(',', virgula + 1); //acha fim palavra lat
      virgula1 = msgrecebida.indexOf(',', virgula + 1); //acha fim lat
      valorstring = msgrecebida.substring(virgula + 1, virgula1 + 1); //pega valor lat
      latitude = valorstring.toFloat();
      virgula = msgrecebida.indexOf(',', virgula1 + 1); //acha inicio lng
      valorstring = msgrecebida.substring(virgula + 1); //pega valor lng
      longitude = valorstring.toFloat();
      //distancia = getDistance(latitude, longitude,latitude_enviada, longitude_enviada);
      distancia2 = getDistance(latitude, longitude,LATITUDE_REFERENCE, LONGITUDE_REFERENCE);
      //envio = String ("concentradorgps") + "," + String (VARIABLE_LABEL_LATITUDE) + "," +String(latitude,6) + "," + String (VARIABLE_LABEL_LONGITUDE) + "," + String(longitude,6) + String (" distancia ") + "," + String(distancia2,6);
      //lorasend(String(envio), BICICLETALORA,LABORATORIOLORA);
      if ((distancia2 < MAX_DISTANCE) && (latitude != 0) && (longitude !=0)) 
      { //nao pode estar muito longe da ober portaria se nao e dado incorreto
        char ROTINA_EXECUTADA [15] = "gps";
        sendValuesmqttmapahttp(contador,latitude, longitude, ROTINA_EXECUTADA,VARIABLE_LABEL_LATITUDE, VARIABLE_LABEL_LONGITUDE);
      }
    }  

    if (identificador == VARIABLE_LABEL_RONDA) {
      //Serial.println("Entrei Ronda");
      //Serial.println("Entrei Ronda Contador: " + String (contador) + " nenvio_ronda: " + String (nenvio_ronda) );
      valorstring.toCharArray(valorstringronda,50);
      if (valorstring != ronda_enviada ) {
          sendValuesmqttshttp (contador, valorstringronda, VARIABLE_LABEL_RONDA);
          ronda_enviada = valorstring;
          nenvio_ronda = 0;
      } else {
        nenvio_ronda = nenvio_ronda + 1;
        if (nenvio_ronda > 5) {
           sendValuesmqttshttp (contador, valorstringronda, VARIABLE_LABEL_RONDA);
          ronda_enviada = valorstring;
          nenvio_ronda = 0;         
        }
      }
    }

    if (identificador == VARIABLE_LABEL_WHATDOG) {
      //Serial.println("Entrei Whatdog");
      //Serial.println("Entrei Ronda Contador: " + String (contador) + " Whatdog: " + String (valorstring) );
      valorstring.toCharArray(valorstringchar,15);
      sendValuesmqttshttp (contador, valorstringchar, VARIABLE_LABEL_WHATDOG);
    }
    if (identificador == VARIABLE_LABEL_HEAP) {
      virgula = msgrecebida.indexOf(',');
      virgula1 = msgrecebida.indexOf(',', virgula + 1); 
      valorstring = msgrecebida.substring(virgula + 1, virgula1); //peguei ph    
      virgula = msgrecebida.indexOf(',', virgula1 + 1); 
      valorstring = valorstring + "," + msgrecebida.substring(virgula1 + 1, virgula); //peguei whatdog    
      virgula1 = msgrecebida.indexOf(',', virgula + 1); 
      valorstring1 =msgrecebida.substring(virgula + 1, virgula1); //pegar valor
      valor = valorstring1.toFloat();
      valorstring.toCharArray(valorstringchar,15);
      sendValuesmqttshttp (valor, valorstringchar, VARIABLE_LABEL_HEAP);
    }
   
    if (identificador == VARIABLE_LABEL_VAZAOPOCO) {
      valor = valorstring.toFloat(); // centimetro
      if (valor != vazao_enviada) {
       if ((valor >= 0) && (valor <=900)){
        sendValuesmqtthttp(valor, VARIABLE_LABEL_VAZAOPOCO);
        vazao_enviada = valor;
        nenvio_vazao = 0;
       }       
      } else {
          nenvio_distancia = nenvio_distancia + 1;
          if (nenvio_vazao > 5) {
            if ((valor >= 0) && (valor <=900)){
              sendValuesmqtthttp(valor, VARIABLE_LABEL_DISTANCIA);
              vazao_enviada = valor;
              nenvio_vazao = 0;
            }
          }
      }
    } 
    contador++;
  }
  delay (100); 
}
