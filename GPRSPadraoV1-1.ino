
double getDistance(float lat, float lon,float lat1, float lon1)
{
  double dist = 60 * ((acos(sin(lat1*(PI/180)) * sin(lat*(PI/180)) + cos(lat1*(PI/180)) * cos(lat*(PI/180)) * cos(abs((lon-lon1)) * (PI/180)))) * (180/PI));
  return dist*1852;
}

//Verifica se o dispositivo ultrapassou o limite de distância
bool deviceIsTooFar(float lat, float lon, float lat1, float lon1, String *distance)
{
  double dist = getDistance(lat, lon, lat1, lon1);

  *distance = String(dist);

  if(dist > MAX_DISTANCE)
      return true;
        
  return false;
}

//rotina que inicializar o gps
void gpsinit(){


   //rotina gpsping. ira fazer em outro processador uma rotina que a cada 1 min posisao
     xTaskCreatePinnedToCore(
      gpsPing, // Function to implement the task 
      "GPSPing", // Name of the task 
      8192,  // Stack size in words 
      NULL,  // Task input parameter 
      10,  // Priority of the task 
      &Task4,  // Task handle. 
      0); // Core where the task should run 
/*
      //rotina gpswahtdog. ira fazer em outro processador uma rotina que a cada 1 min posisao
     xTaskCreatePinnedToCore(
      gpsWhatdog, // Function to implement the task 
      "GPSWhatdog", // Name of the task 
      8192,  // Stack size in words 
      NULL,  // Task input parameter 
      10,  // Priority of the task 
      &Task8,  // Task handle. 
      0); // Core where the task should run 
 */   
}

//envia a posição GPS a cada 60 segundos
void gpsPing(void * parameter){
  char ROTINA_EXECUTADA [15] = "gpsPing";
  
  for(;;){ //loop infinito da rotina
    float distancia3;
	  uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    for(int i=0;i<3;i++)        //envia 3 vezes
    {  
      envio = String (NOME_DISPOSITIVO) + "," +  String (VERSAO) + "," + String ("GPS Ping! ") + "," + IPWifi + "," + String (" Heap: ") + String (uxHighWaterMark) + "," + String (VARIABLE_LABEL_LATITUDE) + "," +String(latitude,6) + "," + String (VARIABLE_LABEL_LONGITUDE) + "," + String(longitude,6);
      lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
      bootCount_anterior = bootCount;
      distancia3 = getDistance(latitude, longitude,LATITUDE_REFERENCE, LONGITUDE_REFERENCE); //distancia ate a portaria
      if (distancia3 < MAX_DISTANCE) {
        envio = String (VARIABLE_LABEL_GPS) + "," +  String (VARIABLE_LABEL_LATITUDE) + "," +String(latitude,6) + "," + String (VARIABLE_LABEL_LONGITUDE) + "," + String(longitude,6);
        lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
      }
                  
    } //envia 3 vezes
  vTaskDelay (60000); //1 minutos
  }  //loop
}

//envia a posição GPS Whatdog
void gpsWhatdog(void * parameter){
  char ROTINA_EXECUTADA [15] = "gpsWhatdog";
  for(;;){ //loop infinito da rotina
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    sendHeap( ROTINA_EXECUTADA, uxHighWaterMark);
    //envio = String ("Entrei no IF latitude_enviada:") + String (latitude_enviada) + String (" longitude_enviada: ") + String (longitude_enviada) + String (" longitude_whatdog: ") + String (longitude_whatdog) + String (" latitudee_whatdog: ") + String (latitude_whatdog); //tentar enviar que deu whatdog
    //lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
    if ((latitude_enviada = 0.00) || (longitude_enviada = 0.00) || (latitude_enviada = latitude_whatdog) || (longitude_enviada = longitude_whatdog))
    {
      Serial.println("Reeset GPS IF");
      envio = String (String (NOME_DISPOSITIVO) + "," +  String (VERSAO) + "," + String (" Heap: ") + String (uxHighWaterMark) + "," + String("Whatdog latitude_enviada: ") + String (latitude_enviada) + String (" longitude_enviada: ") + String (longitude_enviada) + String (" longitude_whatdog: ") + String (longitude_whatdog) + String (" latitudee_whatdog: ") + String (latitude_whatdog)); //tentar enviar que deu whatdog
      lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
      contador_whatdog_gps++;
      if (contador_whatdog_gps > 10) // se em 10 minutos nao mudou a posicao reset
      {
        //Serial.println("Reeset GPS");
        envio = String (VARIABLE_LABEL_WHATDOG) + "," + String (NOME_DISPOSITIVO) + String (VERSAO) + "," + IPWifi; //tentar enviar que deu whatdog
        for(int i=0;i<3;i++)        //envia 3 vezes
        {
          lorasend(String(envio), CODIGO_DISPOSITIVO,LABORATORIOLORA);
        }
        reinicia();  //tells the SDK to reboot, so its a more clean reboot. ESP.reset() is a hard reset   
      }
    }
    else
    {
      latitude_whatdog = latitude_enviada;
      longitude_whatdog = longitude_enviada;
      }
  vTaskDelay (30000); //5min
  }  //loop
}
            
