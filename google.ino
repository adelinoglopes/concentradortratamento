//Rotinas para gravar no Google Drive em uma planilha

boolean googlesend(String medidor, float medida){

  String textFix =  "";
  //int counter = 0;

  for (int i = 0; i < numerosensores; ++i) {
    if (medidor == planilhagoogle [i][0]) {
      textFix = planilhagoogle [i][1];
      break;
    }
  }
  if (textFix == "") {
     return false;
    }
    
  if (google.connect("docs.google.com", 443) == 1)//Tenta se conectar ao servidor do Google docs na porta 443 (HTTPS)
    {
        String toSend = textFix;//Atribuimos a String auxiliar na nova String que sera enviada
        toSend += medida;//Adicionamos um valor 
        toSend += "&submit=Submit HTTP/1.1";//Completamos o metodo GET para nosso formulario.
        google.println(toSend);//Enviamos o GET ao servidor-
        google.println("Host: docs.google.com");//-
        google.println();//-
        google.stop();//Encerramos a conexao com o servidor
        Serial.println("googledrive: " + String(medidor) + " " + String(medida));  
        return true;
    }
    else
    {
        Serial.println("Erro ao se conectar googledrive");//Se nao for possivel conectar no servidor, ira avisar no monitor.
        return false;
    }
}
