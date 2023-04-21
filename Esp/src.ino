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
int  horaDesl=0;
int  minutoDesl=0;
int vet_horaliga[24];
int vet_minutoliga[24];
int vet_horadesliga[24];
int vet_minutodesliga[24];
int qtde_de_horarios=0;
int lido =0;
int inserido=0;




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

	// char date[10] = "hh:mm:ss";
	// rtc.now().toString(date);


	// buf += date;
	// buf += " * ";

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

void setup() {
    Serial.begin(115200);

    initWiFi();

    server.on("/", handleRoot);

    server.begin();
    Serial.println("HTTP server beginned");
  
}

void loop(){
    Serial.println(WiFi.localIP());
    Serial.println();

    server.handleClient();
    delay(2);

}