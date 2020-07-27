//Includes Padrao -  Se alterar tem que alterar em todos os projetos
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h> 
#include <NTPClient.h> //atualiza tempo NTP https://github.com/taranais/NTPClient
#include "TimeLib.h" //mantem relogio https://github.com/geekfactory/TimeLib
//#include "SSD1306.h" // alias for #include "SSD1306Wire.h" desliguei oled nao usando
#include <PubSubClient.h> //Biblioteca para as publicações MQTT https://github.com/knolleary/pubsubclient
#include <SPI.h> //para o lora
#include <LoRa.h> //radio lora
//#include <ArduinoJson.h> //nao usando no processo JSON
#include <HardwareSerial.h> //Comunicação UART com o módulo gps
#include <TinyGPS++.h> //Biblioteca que configura o modem GSM
#include <WebServer.h> //servidor webserver OTA
#include <ESPmDNS.h> //registro dns OTA
#include <Update.h> //atualizar pela web OTA
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"				 						

//Definicoes padrao - Se alterar tem que alterar nos outros projetos
UBaseType_t uxHighWaterMark;
portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

//WIFI 
#define SERVER_PORT 5000
#define MAX_SRV_CLIENTS 20
char ConcentradorIP [16] = "";
String ConcentradorDNS = "Concentrador"; 
String IPWifi = "";
WiFiServer serverwifi(SERVER_PORT);
TaskHandle_t Task11; //registro do nome MDNS de forma continua MDNSCLIENT
TaskHandle_t Task12; //procura o ip do concentrador para o lora torre
TaskHandle_t Task15; //envia mensagem recebida por wifi

//MQTT - Ubidots Padrao
#//define TOKEN "BBFF-XI1x7xp8VjUxbdPEdoTwJuH0znpgWh" //Coloque seu TOKEN do Ubidots aqui
#define DEVICE_ID "Ober" //ID do dispositivo (Device id, também chamado de client name)
#//define SERVER "industrial.api.ubidots.com" //Servidor do Ubidots (broker) antes things.ubidots.com" 169.55.61.243 industrial.api.ubidots.com 50.23.124.68 things.ubidots.com
#//define PORT 1883 //porta que o UBIDOTS trabalha  antes 1883
#define TOPIC "/v1.6/devices/lora1" //Tópico aonde serão feitos os publish, "esp32-dht" é o DEVICE_LABEL
char const * SERVER = "industrial.api.ubidots.com";
const int PORT = 80;
char const * USER_AGENT = "ESP32";
char const * VERSION = "2.0";
char const * TOKEN = "BBFF-XI1x7xp8VjUxbdPEdoTwJuH0znpgWh"; 
char const * DEVICE_LABEL = "lora1"; 												   
//cada medidor tem uma variavel criada
char VARIABLE_LABEL_DISTANCIA [15] = "distancia"; //Label referente a variável de vazao criada no ubidots
char VARIABLE_LABEL_CORPO [15] = "corpo"; //Label referente a variável de temperatura criada no ubidots
char VARIABLE_LABEL_OBJETO [15] = "objeto"; //temperatura da agua medida pelo corpo
char VARIABLE_LABEL_PH [15] = "ph";
char VARIABLE_LABEL_LATITUDE [15] = "lat"; //gps da bicileta
char VARIABLE_LABEL_LONGITUDE [15] = "lng"; // gps da bicicleta
char VARIABLE_LABEL_GPS [15] = "bicicleta";
char VARIABLE_LABEL_TURBIDES [15] = "turbides";
char VARIABLE_LABEL_CHAMINECINZA [15] = "chaminecinza";
char VARIABLE_LABEL_RONDA [15] = "ronda";
char VARIABLE_LABEL_HORA [15] = "hora";
char VARIABLE_LABEL_WHATDOG [15] = "whatdog";
char VARIABLE_LABEL_VAZAOPOCO1 [15] = "poco1";
char VARIABLE_LABEL_VAZAOPOCO2 [15] = "poco2";
char VARIABLE_LABEL_TORRE1 [15] = "torre1";
char VARIABLE_LABEL_TORRE2 [15] = "torre2";
char VARIABLE_LABEL_TORRE3 [15] = "torre3";										   
char VARIABLE_LABEL_LOGCON [15] = "logcon";
char VARIABLE_LABEL_HEAP [15] = "heap";
char VARIABLE_LABEL_LORAERR [15] = "loraerr";
char VARIABLE_LABEL_VAZAOSAIDA [15] = "vazaosaida";
char VARIABLE_LABEL_VALVULAESTACAO [15] = "valvulaestacao";
char VARIABLE_LABEL_RFIDPORTARIA [15] = "rfide";														   
WiFiClient ubidots; //inicializacao da biblioteca para conectar com o Ubidots
PubSubClient client(ubidots); // publica a comunicacao


//NTP e Relogio - tempo
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "a.ntp.br", -10800, 60000); //timezone -3 brasilia
struct timelib_tm tinfo; //biblioteca do relogio interno
timelib_t now, initialt; //a variavel now e a hora que esta o timestamp interno;
String formattedDate; //onde saldo a data que veio do NTP
String dayStamp;
String timeStamp;
String horario,dia,ano,mes,hora,minuto,segundo = "";
TaskHandle_t Task10; //Atualiza o relogio local ao receber pelo Lora

//Lora - Padrao
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO ok
#define MOSI    27   // GPIO27 -- SX127x's MOSI ok
#define SS      18   // GPIO18 -- SX127x's CS - ok
#define RSTL    14   // GPIO14 -- SX127x's RESET este e p GPIO23 
#define DI0     26   // GPIO26 -- SX127x's IRQ(Interrupt Request) - ok
#define BAND    433E6  //// BAND Frequencia do radio - podemos utilizar ainda : 433E6, 868E6, 915E6 nao pode trabalar em 433E6 pois nao tem no V2
#define PABOOST true
#define SPREADINGFACTOR 8 // ranges from 6-12,default 7 see API docs
#define SYNCWORD 0xF3           // ranges from 0-0xFF, default 0x34, see API docs
//cada lora tem um ID. Sera usado nas rotinas de transmissao e recepcao
#define BROADCAST 0xFF //lora brodcast todos irao receber
#define LABORATORIOLORA 0xFD     // address Laboratorio
#define TRATAMENTOTEMPLORA 0xBB      // address Tramaneto Agua temperatura e PH
#define TRATAMENTOVAZAOLORA 0xBC  // address Tratamento Agua Vazao Calha Parchal
#define BICICLETALORA 0XBD  // Bicicleta Eletrica GPS
#define TURBIDES 0XBE // turbides unidade tratamento
#define CINZA 0XBF // turbides unidade tratamento
#define POCO1 0XC0 // poco1
#define REPETER1 0XC1 // lora torre1
#define REPETER2 0XC2 // lora torre2
#define REPETER3 0XC6 // lora torre3									
#define PH 0XC3 // lora PH
#define VAZAOSAIDA 0XC4 // saida DAEE
#define POCO2 0XC5 //poco2
#define VALVULASAIDA 0XC6 //valuvula saida estacao tratamento
#define RFIDPORTARIA 0XC7 //rfid da portaria											
TaskHandle_t Task3; //ponteiro que é alocado a rotina no procesador 0 que fica em loop
byte msgCount = 0; 
String envio = "";  
boolean recebendolora = false;
boolean transmitindolora = false;

/* desliguei oled nao usando
//OLED - Padrao
#define SDA    4
#define SCL   15
#define RST   16 //RST must be set by software
#define V2     1
#ifdef V2 //WIFI Kit series V1 not support Vext control
  #define Vext  21
#endif
//SSD1306  display(0x3c, SDA, SCL, RST);
SSD1306  display(0x3c, SDA, SCL); //inicializa o display
*/

//GPS da Placa TTGO
HardwareSerial ss(1);
TinyGPSPlus gps; 
//https://github.com/LilyGO/TTGO-T-Beam
//https://github.com/LilyGO/TTGO-T-Beam
//https://codebender.cc/library/TinyGPS++#keywords.txt
//http://arduiniana.org/libraries/tinygpsplus/
//http://arduiniana.org/libraries/tinygpsplus/
const double LATITUDE_REFERENCE = -22.771200; //Portaria da Ober 
const double LONGITUDE_REFERENCE = -47.317932; //Portaria da Ober
double latitude_whatdog = 0;
double longitude_whatdog = 0;
double latitude, longitude, latitude_enviada, longitude_enviada =0;
int contador_whatdog_gps;
const double MAX_DISTANCE = 2000; //Distância limite usada para disparar SMS de alerta de distanciamento, medida em metros
const double MIN_DISTANCE = 1; //Distância limite usada para disparar SMS de alerta de distanciamento, medida em metros
//Pino serial RX que deve ser conectado no TX do 
static const int RXPin = 12, TXPin = 15;
const int BAUD_RATE = 9600;
TaskHandle_t Task4; //gpsping
TaskHandle_t Task8; //gpswahtdog

//Vazao
TaskHandle_t Task9; //ponteiro que é alocado a rotina no procesador 0 que fica em loop

//google drive salva as medicoes em uma planilha d
//usuario obersaiot@gmail.com
//senha Ober1@3$5
//https://portal.vidadesilicio.com.br/banco-de-dados-com-google-planilhas-com-esp/
#define numerosensores 8
String planilhagoogle[numerosensores][2] = {
           {"objeto","GET /forms/d/e/1FAIpQLSdZtJgyZcESHnHzRaqkXvJmu4zSTtWNs1a1vkmxLgjpoNWSfA/formResponse?ifq&entry.1283193040="}, 
           {"distancia","GET /forms/d/e/1FAIpQLSerpdnwO9b47wNwwPsQ7d0YdJTyFnB9edxTDRz5K-H1KbWswA/formResponse?ifq&entry.811133558="},
           {"ph","GET /forms/d/e/1FAIpQLSdPyPxykwZ0lb682MPxm2x7TLXRAr-X3mgIyNYshkWPvGVb-Q/formResponse?ifq&entry.369290149="},
           {"turbides","GET /forms/d/e/1FAIpQLSdgQXWLY2vX09srrdh5uJLAIXA9TxBFWpSu6tnYWAafKvVvKw/formResponse?ifq&entry.2078479405="},
           {"poco1","GET /forms/d/e/1FAIpQLSdaJoDJnNRuKOYfGKa0p3eUYT75p9rso1nu5RmQFo5trrzICA/formResponse?ifq&entry.266798070="},
           {"vazaosaida","GET /forms/d/1zgvO9hPD04f1jz1atSZDb6Ogm7PXWsbmjGikN-OJkiA/formResponse?ifq&entry.295441093="},  
           {"poco2","GET /forms/d/e/1FAIpQLSeBXuHzia4FkFMJ1OvwSmofeNPdw4_ZGv20VtxQ8wPW5kgM9w/formResponse?ifq&entry.229113309="},
           {"rfide","GET /forms/d/e/1FAIpQLSc4bliFie8h794-wJBxm_Je-MOO9uKoBqgrQMgWe5mMDIyIew/formResponse?ifq&entry.87164232="} 		 
      }; 
WiFiClientSecure google;

//OTA 
TaskHandle_t Task1; //ponteiro que é alocado a rotina no procesador 0 que fica em loop
String USUARIO_OTA  = "AdminOber";
String SENHA_OTA  = "1@3$5";
//pagina principal
String loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>" + NOME_DISPOSITIVO + VERSAO + "</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='123')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
//pagina do update 
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";
 
WebServer server(80); //inicaliza o servidor na porta 80 '


//Whatdog
boolean reinicializaesp = true;
float   timereset = 120000; //verifica se estou ativo a cada 2 minuto
float loopTime = 0;
float timeligou;
float timeligado = 86400000; //isso e 24h ou seja neste horario vai reinicalar
TaskHandle_t Task7;

//DeepSleep para adormecer
TaskHandle_t Task2; //ponteiro que é alocado a rotina no procesador 0 que fica em loop
RTC_DATA_ATTR int bootCount, bootCount_anterior = 0; //RTC_DATA_ATTR aloca a variável na memoria RTC
#define uS_TO_S_FACTOR 1000000  //* //fator de conversão de microsegundos para segundos
#define TIME_TO_SLEEP   30000         //era 110000 * 15 minutos tempo que o ESP32 entrara em sleep(in miliseconds) 
#define TIME_TO_WAKEUP  1800        //* 30 minutos tempo que o ESP32 ficará em modo sleep Time ESP32 will go to sleep (in seconds) 
#define WAKEUP_PIN GPIO_NUM_34

//variaveis padroes
float contador = 1;
float loaddelay = 0;
TaskHandle_t Task13; //torreenvio
TaskHandle_t Task14; //concentrador envio

/*
Lista de Task rodam no core 0
task1 OTAWEBServer "ServidorOta"
task2 deepsleep MotiDeslocamento "MotiroaDeslocamento",
task3 LoraPing "LoaraPing"
task4 gpsPing "GPSPing"
task5 NTP pega hora
task6 NTP atualiza
task7 whatdog "Whatdog"
task8 gpsWhatdog "GPS
task9 leituravazao "Leitura Vazao" 
task10 NTP Relocio Recebe
task11 MDNSCLIENT 
task12 CONCENTRADORIP
task13 torreenvio
task14; //concentrador envio ainda nao esta pronta a rotina
task15; envia wifi

Pinos
4- OLED - SDA
5- Lora - SCK 
12- GPS - RX
12- Ultrason -  Triger
13- Ultrason - Echo
14- Lora - RSTL
15- OLED - SCL
15- GPS - TX
16- OLED - RST
18- Lora - SS
19- Lora - MISO 
21- Temperatura - SDA
22- Temperadura - SCL
26- Lora - DIO
27- Lora - MOSI 
32- Turbides - analogico
33- Rele - Whatdog - Digital
34 - PH - SensorPin
34- Vazao - Porta Digital
34 - Chamine CInza - analogico
35 - LedStatus - Digital
*/
