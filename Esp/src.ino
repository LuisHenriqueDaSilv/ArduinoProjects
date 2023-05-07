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

bool dispositivoEstaLigado = false;

const int pin = 18; //Equivalente ao D2 no NodeMCU
uint8_t rele = 18;
uint8_t addr_inicial = 8;
int  horaLiga=0;
int  minutoLiga=0;
int  horaDesl=0;
int  minutoDesl=0;



String SendHTML(){
//Iniciando"; o buffer que ira conter a pagina HTML que sera enviada para o browser.
  String buf = ""; 
  buf += "<html lang='pt-BR'>";
  buf += "  <head>";
  buf += "      <meta charset='UTF-8'>";
  buf += "      <meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  buf += "      <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  buf += "      <title>Dispositivo remoto</title>";
  buf += "      <link rel='preconnect' href='https://fonts.googleapis.com'>";
  buf += "      <link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>";
  buf += "      <link href='https://fonts.googleapis.com/css2?family=Inter:wght@200;500;600;700&display=swap' rel='stylesheet'>";
  buf += "  </head>";
  buf += "  <style>";
  buf += "      * {";
  buf += "          box-sizing: border-box;";
  buf += "          margin: 0;";
  buf += "          padding: 0;";
  buf += "          font-family: 'Inter', sans-serif;";
  buf += "          text-align: center;";
  buf += "          color: #403937;";
  buf += "      }";
  buf += "      :root {";
  buf += "          font-size: 62.5%;";
  buf += "      }";
  buf += "      body {";
  buf += "          width: 100vw;";
  buf += "          height: 100vh;";
  buf += "          background: linear-gradient(107.56deg, #5D5F28 0%, rgba(32, 90, 38, 0.557143) 32.29%, rgba(26, 87, 32, 0.264286) 53.65%, rgba(30, 30, 30, 0) 100%);";
  buf += "          background-color: #1E1E1E;";
  buf += "          font-size: 1.6rem;";
  buf += "      }";
  buf += "      .wrapper {";
  buf += "          display: flex;";
  buf += "          height: 100vh;";
  buf += "          width: 100vw;";
  buf += "          align-items: center;";
  buf += "          justify-content: center;";
  buf += "      }";
  buf += "      .app {";
  buf += "          width: 90vw;";
  buf += "          height: fit-content;";
  buf += "          padding: 2rem;";
  buf += "          background-color: #D9D9D9;";
  buf += "          border-radius: 10px;";
  buf += "          display: flex;";
  buf += "          align-items: center;";
  buf += "          flex-direction: column;";
  buf += "          gap: 4rem;";
  buf += "      }";
  buf += "      a:hover{";
  buf += "          transform: scale(1.02);";
  buf += "          opacity: 0.7;";
  buf += "      }";
  buf += "      a {";
  buf += "          color: #ffffff;";
  buf += "          margin-top: 1rem;";
  buf += "          border: none;";
  buf += "          cursor: pointer;";
  buf += "          transition: 200ms;";
  buf += "          font-size: 2.5rem;";
  buf += "          font-weight: 700;";
  buf += "          border-radius: 10px;";
  buf += "          padding: 1rem;";
  buf += "          text-decoration: none;";
  buf += "      }";
  buf += "      .status {";
  buf += "          font-weight: 700;";
  buf += "          -webkit-text-stroke: 1px #403937;";
  buf += "      }";
  buf += "      .header {";
  buf += "          display: flex;";
  buf += "          flex-direction: column;";
  buf += "          justify-content: space-between;";
  buf += "          max-width: 97.4rem;";
  buf += "      }";
  buf += "      #desligado {";
  buf += "          color: #D45C5C;";
  buf += "      }";
  buf += "      #botaodesligado {";
  buf += "          background-color: #8AC880;";
  buf += "      }";
  buf += "      #ligado {";
  buf += "          color: #8AC880;";
  buf += "      }";
  buf += "      #botaoligado {";
  buf += "          background-color: #D45C5C;";
  buf += "      }";

  buf += "      .horario-atual {";
  buf += "          display: flex;";
  buf += "          justify-content: center;";
  buf += "          gap: 1rem;";
  buf += "          flex-direction: row;";
  buf += "          margin-top: 1rem;";
  buf += "      }";
  buf += "      .horario-atual div {";
  buf += "          display: flex;";
  buf += "          justify-content: center;";
  buf += "          align-items: center;";
  buf += "          background-color: #CDCDCD;";
  buf += "          height: 7rem;";
  buf += "          width: 7rem;";
  buf += "          border-radius: 10px;";
  buf += "          font-size: 4rem;";
  buf += "          font-weight: 500;";
  buf += "      }";
  buf += "      .separador-de-hora {";
  buf += "          font-size: 4rem;";
  buf += "          align-self: center;";
  buf += "      }";



  buf += "      @media (min-width: 952px) {";
  buf += "          .header {";
  buf += "              flex-direction: row;";
  buf += "          }";
  buf += "      }";
  buf += "  </style>";
  buf += "  <body>";
  buf += "      <div class='wrapper'>";
  buf += "          <div class='app'>";
  buf += "              <header class='header'>";
  buf += "                  <div>";

  if(dispositivoEstaLigado){
    buf += "                      <h1>O dispositivo está <strong class='status' id='ligado'>LIGADO</strong> </h1>";
    buf += "                      <p id='headerp'>E permanecera assim até o horario configurado para desligar ou o usuario desligar manualmente.</p>";
  } else {
    buf += "                      <h1>O dispositivo está <strong class='status' id='desligado'>Desligado</strong> </h1>";
    buf += "                      <p id='headerp'>E permanecera assim até o horario configurado para ligar ou o usuario ligar manualmente.</p>";
  }

  buf += "                  </div>";

  if(dispositivoEstaLigado){
    buf += "                  <a href='/desliga' id='botaoligado'>";
    buf += "                      desligar manualmente";
    buf += "                  </a>";
  }else {
    buf += "                  <a href='/liga' id='botaodesligado'>";
    buf += "                      ligar manualmente";
    buf += "                  </a>";
  }
  buf += "              </header>";

	// char date[10] = "hh:mm:ss";
	char hora[10] = "hh";
	char minuto[10] = "mm";
	char segundos[10] = "ss";
	rtc.now().toString(hora);
	rtc.now().toString(minuto);
	rtc.now().toString(segundos);

  buf += "                <div>";
  buf += "                    <h1>horario atual:</h1>";
  buf += "                    <div class='horario-atual'>";
  buf += "                        <div id='hora-atual'>";
  buf += hora;
  buf += "                        </div> <label class='separador-de-hora'>:</label>";

  buf += "                        <div id='minuto-atual'>";
  buf += minuto;
  buf += "                        </div>";
  buf += "                    </div>";
  buf += "                </div>";

  buf += "          </div>";
  buf += "      </div>";
  
  buf += "  </body>";
  buf += "  <script>";
  buf += "    setInterval(async() => {\n";
  buf += "      const respostaHoraAtual = await fetch('/relogio')\n";
  buf += "      const response = await respostaHoraAtual.json()\n";

  buf += "      console.log(response)\n";


  buf += "      const visorHora = window.document.getElementById('hora-atual')\n";
  buf += "      const visorMinuto = window.document.getElementById('minuto-atual')\n";

  buf += "      visorHora.innerHTML = response.hora\n";
  buf += "      visorMinuto.innerHTML = response.minuto\n";
  buf += "    }, 15000)";
  buf += "  </script>";
  buf += "</html>";

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
    dispositivoEstaLigado=true;    
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
    dispositivoEstaLigado=false;    
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
    // novo_relogio=1;
//    server.send(200, "text/html", SendHTML(LED1status,true)); 

    char horas[10] = "hh";
    char minutos[10] = "mm";
    rtc.now().toString(horas);   
    rtc.now().toString(minutos);   

    String buf = "{ \"hora\" : \""+ String(horas) + "\", \"minuto\" : \""+String(minutos)+"\" }";

    server.send(200, "text/json", buf); 
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
	if(rtc.lostPower()){
		Serial.println("DS3231 OK!");
		// rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //CAPTURA A DATA E HORA EM QUE O SKETCH É COMPILADO
		rtc.adjust(DateTime(2023, 5, 06, 15, 00, 00)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
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
	// server.send(200, "text/html", SendHTML()); 

}