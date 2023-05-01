#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
// #include <Wire.h>
#define EEPROM_SIZE 64


RTC_DS3231 rtc;

const char* ssid = "CXR";  // Enter SSID here
const char* password = "2021mcdl";  //Enter Password here

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);


uint8_t LED1pin = 18;
bool LED1status = LOW;

uint8_t LED2pin = 18;
bool LED2status = LOW;

char datestring[20];
int novo_horario=0;
int novo_relogio=0;
int relhoraLiga=0;
int relminutoLiga=0;
int vet_horaliga[24];
int vet_minutoliga[24];
int vet_horadesliga[24];
int vet_minutodesliga[24];
int qtde_de_horarios=0;
int lido =0;
int inserido=0;
int botao_liga=0, botao_desliga=0;
bool ledState = false;
const int pin = 18; //Equivalente ao D2 no NodeMCU
uint8_t rele = 18;
uint8_t addr_inicial = 8;
int  horaLiga=0;
int  minutoLiga=0;
int  horaDesl=0;
int  minutoDesl=0;




String SendHTML(){
//Iniciando o buffer que ira conter a pagina HTML que sera enviada para o browser.
	String buf = "";  
	buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
	buf = "<html lang=\"port\">";


	buf += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=yes\"/>\r\n";
	buf =+ "<meta http-equiv=\"refresh\" content=\"10\">";
	buf += "<title>Dispositivo</title>";


  	if (LED1status==true) {
		buf += "<div class=\"panel-heading\"><font size='6'><font color='blue'><b>LIGADO</b></font></div>";    
  	}else{
    	buf += "<div class=\"panel-heading\"><font size='6'><font color='green'><b>DESLIGADO</b></font></div>";   
	}
  
	buf += "<style>.c{text-align: center;} div,input{padding:10px;font-size:1em;} input{width:100%;} body{text-align: center;font-family:verdana;font-size:1em;}</style>";
	buf += "</head>";

	//  buf += "<body onload='startTime()'><div class=\"panel panel-primary\">";
	buf += "<div class=\"panel-heading\"><font size='6'><b>Dispositivo Remoto</b></font></div>";
	buf += "<div class=\"panel-body\">";
	buf += "<div id=\"txt\" style=\"font-weight:bold;\"></div>";
	buf += "</p><div class='container'>";
	buf += "<h4>";
	buf += datestring;
	buf += "</h4>";


	buf += "<div class='btn-group'>";
	buf += "<a href=\"liga\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='blue'>LIGAR AGORA</b></button></a>";
	buf += "<a href=\"desliga\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='green'>DESLIGAR AGORA</b></button></a><br>";
	buf += "<a href=\"limpa\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='red'>LIMPAR MEM</button></a>";
	buf += "<a href=\"ler\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='red'>LER MEM</b></button></a>";


  	if (novo_horario == 0) {
    	buf += "<a href=\"novo\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='blue'>NOVO HOR&AacuteRIO</b><font color='black'></button></a>";
  	}
	buf += "</div> ";
	buf += "<a href=\"relogio\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='black'>Atualizar Rel&oacute;gio</b></button></a>";

	char date[10] = "hh:mm:ss";
	rtc.now().toString(date);


	buf += date;
	buf += " * ";

  	if (novo_relogio == 1) {
		buf += "<div class='btn-group'>";
		buf += "<p>Rel&oacute;gio: <span class=\"label label-success\">";
		buf += "<font size='4' color='blue'><b>";
		buf += String(relhoraLiga);
		buf += " : ";
		buf += String(relminutoLiga);
		buf += "</b></font>";
		buf += "<a href=\"setrHLu\"><button type='button' class='btn btn-info' style='margin: 3px'>+1 h</button></a>";
		buf += "<a href=\"setrHLd\"><button type='button' class='btn btn-info' style='margin: 3px'>-1 h</button></a>";
		buf += "<a href=\"setrMLu\"><button type='button' class='btn btn-info' style='margin: 3px'>+5 m</button></a>";
		buf += "<a href=\"setrMLd\"><button type='button' class='btn btn-info' style='margin: 3px'>-5 m</button></a>";
		buf += "<a href=\"setr1MLu\"><button type='button' class='btn btn-info' style='margin: 3px'>+1 m</button></a>";
		buf += "<a href=\"setr1MLd\"><button type='button' class='btn btn-info' style='margin: 3px'>-1 m</button></a>";
		buf += "<a href=\"salva_relogio\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='green'>Salvar</b><font color='black'></button></a>";
		buf += "<a href=\"cancelar\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='red'>Cancelar</b><font color='black'></button></a>";
		buf += "</div> ";
  	}
  
	Serial.println("novo_horario1: "+novo_horario);

	if (novo_horario == 1) {
		Serial.println("novo_horario2: "+novo_horario);    
		buf += "<div class='btn-group'><p align='center'><font color='green'><b><p>Rel&oacute;gio liga: </b><font color='black'><span class=\'label label-success\'>";
		buf += "<font size='4' color='blue'><b>";
		buf += String(relhoraLiga);
		buf += " : ";
		buf += String(relminutoLiga);
		buf += "</b></font>";
		buf += "<a href=\"setHLu\"><button type='button' class='btn btn-info' style='margin: 3px'>+1 h</button></a>";
		buf += "<a href=\"setHLd\"><button type='button' class='btn btn-info' style='margin: 3px'>-1 h</button></a>";
		buf += "<a href=\"setMLu\"><button type='button' class='btn btn-info' style='margin: 3px'>+5 m</button></a>";
		buf += "<a href=\"setMLd\"><button type='button' class='btn btn-info' style='margin: 3px'>-5 m</button></a>";
		buf += "<a href=\"set1MLu\"><button type='button' class='btn btn-info' style='margin: 3px'>+1 m</button></a>";
		buf += "<a href=\"set1MLd\"><button type='button' class='btn btn-info' style='margin: 3px'>-1 m</button></a>";
		buf += "</div> ";
		buf += "<div class='btn-group'><font color='red'><b><p>Rel&oacute;gio desliga: </b><font color='black'><span class=\'label label-success\'>";
		buf += "<font size='4' color='blue'><b>";
		buf += String(horaDesl);
		buf += " : ";
		buf += String(minutoDesl);
		buf += "</b></font>";
		buf += "<a href=\"setHDu\"><button type='button' class='btn btn-info' style='margin: 3px'>+1 h</button></a>";
		buf += "<a href=\"setHDd\"><button type='button' class='btn btn-info' style='margin: 3px'>-1 h</button></a>";
		buf += "<a href=\"setMDu\"><button type='button' class='btn btn-info' style='margin: 3px'>+5 m</button></a>";
		buf += "<a href=\"setMDd\"><button type='button' class='btn btn-info' style='margin: 3px'>-5 m</button></a>";
		buf += "<a href=\"set1MDu\"><button type='button' class='btn btn-info' style='margin: 3px'>+1 m</button></a>";
		buf += "<a href=\"set1MDd\"><button type='button' class='btn btn-info' style='margin: 3px'>-1 m</button></a><br>";
		buf += "<a href=\"inserir\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='green'>Salvar</b><font color='black'></button></a>";
		buf += "<a href=\"cancelar\"><button type='button' class='btn btn-info' style='margin: 3px'><b><font color='red'>Cancelar</b><font color='black'></button></a>";    
		buf += "</div>";//container
	}
	if (lido == 1 && qtde_de_horarios > 0) {
		buf += "<p>HOR&AacuteRIOS";
		buf += "<table border='1' align='center'>";
		buf += "<tr> <th><font color='blue'>  LIGADO   </font></th><th><font color='red'> DESLIGADO </font></th> </tr>";
		int x;
		for (x = 0; x < qtde_de_horarios; x++) {
			buf += "<tr> <td align='center'><font color='blue'>" + String(vet_horaliga[x]);
			buf += ":" + String(vet_minutoliga[x]);
			buf += "</td> <td align='center'></font><font color='red'>" + String(vet_horadesliga[x]);
			buf += ":" + String(vet_minutodesliga[x]);
			buf += "</td> </tr></font>";
		}
		buf += "</table>";
		buf += "</p>";
		if (inserido == 1) {
			buf += "<a href=\"salvar\"><button type='button' class='btn btn-info' style='margin: 5px'>Salvar</button></a>";
		}
	}
  buf += "</div> ";
  buf += "</body>";
  buf += "</html>\n";

  return buf;
}

void handleRoot(){
    server.send(200, "text/html", SendHTML()); 
}

void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED){
		Serial.print('.');
		delay(1000);
    }
    Serial.println(WiFi.localIP());
}

void Clear_Data() {
  EEPROM.begin(512);  
  Serial.println("Limpando EEPROM!");
  for (int i = 0; i <= 255; i++) {
    EEPROM.write(i, 0);
    EEPROM.end();    
  }
  Serial.println("EEPROM apagada!");
  EEPROM.write(addr_inicial, (byte) 0);
  EEPROM.commit();
  lido=0;
  qtde_de_horarios=0;  
}

void liga()  {
    digitalWrite(pin, LOW);
    novo_horario=0;   
    lido=1;    
    ledState=false;    
    LED1status = true;
    server.send(200, "text/html", SendHTML()); 
    botao_liga=1;
    botao_desliga=1;
    digitalWrite(rele, HIGH);     
}
void desliga()  {
    digitalWrite(pin, HIGH); 
    LED1status = false;
    novo_horario=0; 
    lido=1;    
    ledState=true;    
    server.send(200, "text/html", SendHTML()); 
    botao_liga=0;
    botao_desliga=0;
    digitalWrite(rele, LOW);     
}

void limpa()  {
    Clear_Data();
    novo_horario=0;   
    lido=1;  
    botao_liga=0, botao_desliga=0;
    server.send(200, "text/html", SendHTML()); 
}

void order_data(){

  int aux_horaliga;
  int aux_minutoliga;
  int aux_horadesliga;
  int aux_minutodesliga;
  
  boolean desordenado = true;
  
  while(desordenado){
    desordenado = false;
    int x;
    for(x=0;x<qtde_de_horarios-1;x++){
      if(vet_horaliga[x]>vet_horaliga[x+1]){
        aux_horaliga=vet_horaliga[x];
        aux_minutoliga=vet_minutoliga[x];
        aux_horadesliga=vet_horadesliga[x];
        aux_minutodesliga=vet_minutodesliga[x];        
        vet_horaliga[x]=vet_horaliga[x+1];
        vet_minutoliga[x]=vet_minutoliga[x+1];
        vet_horadesliga[x]=vet_horadesliga[x+1];
        vet_minutodesliga[x]=vet_minutodesliga[x+1];        
        vet_horaliga[x+1]=aux_horaliga;
        vet_minutoliga[x+1]=aux_minutoliga;
        vet_horadesliga[x+1]=aux_horadesliga;
        vet_minutodesliga[x+1]=aux_minutodesliga;        
        desordenado = true;
      }
    }
  }
}
void Read_Data(){
  qtde_de_horarios = EEPROM.read(addr_inicial);
  Serial.println("qtde_de_horarios: "+qtde_de_horarios);
  int x;
  uint8_t end_ini=addr_inicial;
  for(x=0;x<qtde_de_horarios;x++){
    end_ini++; 
    vet_horaliga[x] = EEPROM.read(end_ini);
    end_ini++;
    vet_minutoliga[x] = EEPROM.read(end_ini);
    end_ini++;    
    vet_horadesliga[x] = EEPROM.read(end_ini);
    end_ini++;      
    vet_minutodesliga[x] = EEPROM.read(end_ini);   
  }
  order_data();
  lido=1;
  novo_horario=0;   
}
void ler()  {
    Read_Data();
    novo_horario=0;   
    lido=1;  
    botao_liga=0, botao_desliga=0;
    server.send(200, "text/html", SendHTML()); 
} 
void novo()  {
    lido = 0;
    novo_horario=1;
    botao_liga=0, botao_desliga=0;    
    server.send(200, "text/html", SendHTML()); 
}
void setrHLu() {
    relhoraLiga++; 
    if (relhoraLiga > 23) {
      relhoraLiga = 00;
    } 
    server.send(200, "text/html", SendHTML()); 
} 

void setrHLd() {
    relhoraLiga--; 
    if (relhoraLiga < 00) {
      relhoraLiga = 23;
    }      
    server.send(200, "text/html", SendHTML()); 
} 

void setrMLu(){
    relminutoLiga = relminutoLiga + 5;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }  
    server.send(200, "text/html", SendHTML()); 
}  

void setrMLd() {
    relminutoLiga = relminutoLiga -5;
    if (relminutoLiga < 0) {
      relminutoLiga = 55;
    }       
    server.send(200, "text/html", SendHTML()); 
} 

void setr1MLu() {
    relminutoLiga = relminutoLiga + 1;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }       
    server.send(200, "text/html", SendHTML()); 
} 

void setr1MLd() {
    relminutoLiga = relminutoLiga -1;
    if (relminutoLiga < 0) {
      relminutoLiga = 59;
    }        
    server.send(200, "text/html", SendHTML()); 
} 

void relogio()  {
/*    char horaRel[5];
    char horas[10] = "hh:mm:ss";
    rtc.now().toString(horas);    
    strcpy(horas,string_substring(horas, 0, 3));
    relhoraLiga=atoi(horas);
    char minutos[2];
    strcpy(horas,string_substring(horas, 3, 2));
    relminutoLiga=atoi(horas); 
*/    
    novo_relogio=1;
//    server.send(200, "text/html", SendHTML(LED1status,true)); 
    server.send(200, "text/html", SendHTML()); 
} 

void save_relogio(int relhoraliga, int relminutoliga) {
//p  rtc.setTime(relhoraliga, relminutoliga, 0);    //Define o horario
  rtc.adjust(DateTime(2021, 1, 21, relhoraliga, relminutoliga, 0));
  novo_relogio = 0;
  // rtc.setDate(5, 3, 2018);   //Define o dia, mes e ano
//  server.send(200, "text/html", SendHTML(LED1status,true)); 
    server.send(200, "text/html", SendHTML()); 
}
void salva_relogio()  {
  //novo_horario==1
    save_relogio(relhoraLiga, relminutoLiga);   
    novo_relogio=0;     
  //server.send(200, "text/html", SendHTML(LED1status,true)); 
    server.send(200, "text/html", SendHTML()); 
}
void Save_Data() {
  order_data();
  EEPROM.begin(512);
  uint8_t end_ini = addr_inicial;
  EEPROM.write(addr_inicial, (byte) qtde_de_horarios);
  int x;
  Serial.println("qtde_de_horarios: " + qtde_de_horarios);
  for (x = 0; x < qtde_de_horarios; x++) {
    end_ini++;;
    EEPROM.write(end_ini, (byte)vet_horaliga[x]);
    end_ini++;
    EEPROM.write(end_ini, (byte)  vet_minutoliga[x]);
    end_ini++;
    EEPROM.write(end_ini, (byte) vet_horadesliga[x]);
    end_ini++;
    EEPROM.write(end_ini, (byte) vet_minutodesliga[x]);
  }
  EEPROM.commit();
  Serial.println("Dados salvos na EEPROM");

  novo_horario = 0;
  novo_relogio = 0;
} 
void salvar()  {
    Save_Data();
    novo_horario=0;  
    Read_Data();  
    lido=1;   
    inserido=0; 
 //   qtde_de_horarios++;             
    server.send(200, "text/html", SendHTML()); 
}  
void cancelar()  {
  lido = 1;
  novo_horario=0;
  qtde_de_horarios--;  
  novo_relogio = 0;
  server.send(200, "text/html", SendHTML()); 
}
void setHLu() {
    relhoraLiga++; 
    if (relhoraLiga > 23) {
      relhoraLiga = 00;
    } 
  horaDesl=relhoraLiga;
  server.send(200, "text/html", SendHTML());     
}

void setHLd() {
    relhoraLiga--; 
    if (relhoraLiga < 00) {
      relhoraLiga = 23;
    }  
    horaDesl=relhoraLiga;    
  server.send(200, "text/html", SendHTML()); 
}
void setMLu() {
    relminutoLiga = relminutoLiga + 5;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }
    minutoDesl=relminutoLiga;
  server.send(200, "text/html", SendHTML()); 
}
void setMLd() {
    relminutoLiga = relminutoLiga -5;
    if (relminutoLiga < 0) {
      relminutoLiga = 55;
    }
    if (minutoDesl < 10) {
      minutoDesl = 05;
    }     
    minutoDesl=relminutoLiga;        
  server.send(200, "text/html", SendHTML()); 
}
void set1MLu() {
    relminutoLiga = relminutoLiga + 1;
    if (relminutoLiga > 59) {
    	relminutoLiga = 00;
    }    
    minutoDesl=relminutoLiga;
  	server.send(200, "text/html", SendHTML()); 
} 
void set1MLd() {
    relminutoLiga = relminutoLiga -1;
    if (relminutoLiga < 0) {
      relminutoLiga = 59;
    }          
    minutoDesl=relminutoLiga;    
  server.send(200, "text/html", SendHTML()); 
}
void setHDu() {
    horaDesl++;
    if (horaDesl > 23) {
      horaDesl = 00;
    }   
  server.send(200, "text/html", SendHTML()); 
}

void setHDd() {
    horaDesl--;
    if (horaDesl < 00) {
      horaDesl = 23;
    }
    if(horaDesl<relhoraLiga){
      horaDesl++;
    }
  server.send(200, "text/html", SendHTML()); 
}

void setMDu() {
    minutoDesl = minutoDesl + 5;
    if (minutoDesl > 59) {
      minutoDesl = 00;
    }
  server.send(200, "text/html", SendHTML());     
  } 

void setMDd() {
    minutoDesl = minutoDesl - 5;
    if (minutoDesl < 00) {
      minutoDesl = 55;
    }
    if (minutoDesl < 10) {
      minutoDesl = 05;
    }    
  server.send(200, "text/html", SendHTML()); 
}

void set1MDu() {
    minutoDesl = minutoDesl + 1;
    if (minutoDesl > 59) {
      minutoDesl = 00;
    }
  server.send(200, "text/html", SendHTML()); 
} 

void set1MDd() {
    minutoDesl = minutoDesl - 1;
    if (minutoDesl < 00) {
      minutoDesl = 59;
    }   
  server.send(200, "text/html", SendHTML()); 
}

void save_array(int horaliga, int minutoliga, int horadesliga, int minutodesliga, int pos) {
  vet_horaliga[pos] = horaliga;
  vet_minutoliga[pos] = minutoliga;
  vet_horadesliga[pos] = horadesliga;
  vet_minutodesliga[pos] = minutodesliga;
  lido = 1;
}

void inserir()  {
 if((horaLiga<horaDesl)||((horaLiga==horaDesl)&&(minutoLiga<minutoDesl))){
    save_array(relhoraLiga, relminutoLiga, horaDesl, minutoDesl, qtde_de_horarios);
    qtde_de_horarios++;    
//    Save_Data();
    novo_horario=0;  
//    Read_Data();  
    order_data();  
    inserido=1;     
    Serial.println("salvo: ");    
	}
	else{
	novo_horario=1;
	inserido=0;    
	}
    lido = 1;
    novo_horario=0;
  server.send(200, "text/html", SendHTML()); 
}   

int stringToInt(String value) {
	int outLong=0;
	int inLong=1;
	int c = 0;
	int idx=value.length()-1;
	for(int i=0;i<=idx;i++){

		c=(int)value[idx-i];
		outLong+=inLong*(c-48);
		inLong*=10;
	}

	return outLong;
}

boolean ligado(int hora){
    int x;
    String horaDesliga, horaLiga;
    for(x=0;x<qtde_de_horarios;x++){
      if(vet_minutodesliga[x]<10){
        horaDesliga=(String(vet_horadesliga[x])+"0"+String(vet_minutodesliga[x])+"00");
        horaLiga=(String(vet_horaliga[x])+"0"+String(vet_minutoliga[x])+"00");       
      }else{
        horaDesliga=(String(vet_horadesliga[x])+""+String(vet_minutodesliga[x])+"00");
        horaLiga=(String(vet_horaliga[x])+""+String(vet_minutoliga[x])+"00");        
      }      

      int horaLiga_int = stringToInt(horaLiga);
      int horaDesliga_int=stringToInt(horaDesliga);
      if(hora>=horaLiga_int&&hora<horaDesliga_int){
        if(botao_desliga==0){
          LED1status=true;
        }
        return true;
      }                           
    }  
 if(botao_liga==0){
  LED1status=false;   
 }
 return false;
}


void setup() {
    Serial.begin(115200);
	pinMode(LED1pin, OUTPUT);
	pinMode(LED2pin, OUTPUT);
	pinMode(rele, OUTPUT);  

    initWiFi();

	server.on("/", handleRoot);
	server.on("/liga", liga);
	server.on("/desliga", desliga);
	server.on("/limpa", limpa);  
	server.on("/ler", ler);   
	server.on("/novo", novo);     
	server.on("/setHLu", setHLu);  
	server.on("/setHLd", setHLd);  
	server.on("/setMLu", setMLu);  
	server.on("/setMLd", setMLd);  
	server.on("/set1MLu", set1MLu);  
	server.on("/set1MLd", set1MLd);  
	server.on("/setHDu", setHDu);  
	server.on("/setHDd", setHDd);  
	server.on("/setMDu", setMDu);  
	server.on("/setMDd", setMDd);  
	server.on("/set1MDu", set1MDu);  
	server.on("/set1MDd", set1MDd);  
	server.on("/inserir", inserir);      
	server.on("/cancelar", cancelar);    
	server.on("/salvar", salvar);  
	server.on("/setrHLu", setrHLu);  
	server.on("/setrHLd", setrHLd);  
	server.on("/setrMLu", setrMLu);  
	server.on("/setrMLd", setrMLd);  
	server.on("/setr1MLu", setr1MLu);  
	server.on("/setr1MLd", setr1MLd);  
	server.on("/relogio", relogio);    
	server.on("/salva_relogio", salva_relogio);     
	server.on("/Save_Data", Save_Data);   
	server.on("/Read_Data", Read_Data);


    server.begin();
    Serial.println("HTTP server beginned");

	if(!rtc.begin()) {
		while(1){
			Serial.println("DS3231 não encontrado");
		};
	}
	if(rtc.lostPower()){ //SE RTC FOI LIGADO PELA PRIMEIRA VEZ / FICOU SEM ENERGIA / ESGOTOU A BATERIA, FAZ
		Serial.println("DS3231 OK!"); //IMPRIME O TEXTO NO MONITOR SERIAL
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
		//rtc.adjust(DateTime(2018, 9, 29, 15, 00, 45)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
	}

    Serial.println(WiFi.localIP());
    Serial.println();

	if (!EEPROM.begin(EEPROM_SIZE)){
    	Serial.println("failed to initialise EEPROM"); 
    	delay(10000);
  	}
  
}

int cont=0;

void loop(){


	if(cont==0){
		cont++;
		Read_Data();    
	}
	
	DateTime now = rtc.now();  
	int hora = now.hour();
	int minuto = now.minute();
	String hora_str;
	hora_str=(String(hora)+String(minuto)+"00").c_str(); 
	int hora_atual=atoi(hora_str.c_str());  
	ligado(hora_atual);
	
	server.handleClient();

	if(!LED1status){
		digitalWrite(LED1pin, HIGH);
		digitalWrite(rele, HIGH);  
	}else{
		digitalWrite(LED1pin, LOW);
		digitalWrite(rele, LOW);
	}
	server.send(200, "text/html", SendHTML()); 

}