#define countof(a) (sizeof(a) / sizeof(a[0]))
#include <EEPROM.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
// #include <Wire.h>

RTC_DS3231 rtc;

#define EEPROM_SIZE 64

//******* EEPROM ********//
// Endereços reservados na memória
uint8_t addr = 5;    // programado ou não
uint8_t addr0 = 6;    // horaliga
uint8_t addr1 = 7;   // minutoLiga
uint8_t addr2 = 8;   // horaDesl
uint8_t addr3 = 9;   // minutoDesl
uint8_t addr4 = 10;  // stateRelay
uint8_t addr5 = 11;  // status_auto

int Relay = 8;                // Pino Utilizado
uint8_t status_gpio = 8;      // Define condição para GPIO
uint8_t status_auto;          // Define status do botão auto
boolean stateRelay;           // Estado do pino Relay

//******* Função Temporizada ********//
int  horaLiga=0;
int  minutoLiga=0;
int  horaDesl=0;
int  minutoDesl=0;

int vet_horaliga[24];
int vet_minutoliga[24];
int vet_horadesliga[24];
int vet_minutodesliga[24];

int qtde_de_horarios=0;
int novo_horario=0;

int novo_relogio=0;
int relhoraLiga=0;
int relminutoLiga=0;

uint8_t addr_inicial = 8;

int lido =0;

int inserido=0;

//Pino do NodeMCU que estara conectado ao rele
const int pin = 4; //Equivalente ao D2 no NodeMCU

char datestring[20];
                                    
bool ledState = false;

int stateLED = LOW;

uint8_t rele = 23;

/* Put your SSID & Password */
const char* ssid = "ESP32";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

uint8_t LED1pin = 23;
boolean LED1status = false;

uint8_t LED2pin = 27;
bool LED2status = LOW;

char hora[10] = "hh:mm:ss";

int botao_liga=0, botao_desliga=0;

void setup() {
  Serial.begin(115200);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
//  rtc.adjust(DateTime(2021, 1, 21, 3, 0, 0));
  
//  rtc.adjust(DateTime(F(_DATE), F(TIME_)));    
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
  //  rtc.adjust(DateTime(F(_DATE), F(TIME_)));
  //rtc.adjust(DateTime(2021, 1, 21, 3, 0, 0));      
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(_DATE), F(TIME_)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

    //we don't need the 32K Pin, so disable it
    rtc.disable32K();
  
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);
  pinMode(rele, OUTPUT);  

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
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
  Serial.println("HTTP server started");
//  char hora[10] = "hh:mm:ss";
//  rtc.now().toString(hora);    
//  Serial.print("Hora: ");
//  Serial.println(hora);
  

      
  //**EEPROM**
  if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM"); 
    delay(10000);
  }
//  delay(250);

 //Read_Data(); 
 Serial.println("Dados lidos");
 delay(10000); 
}

int cont=0;

void loop() {
//  server.send(200, "text/html", SendHTML(LED1status,true)); 
//  String str="15:22:00";

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
//  Serial.println(hora_atual);
  
  server.handleClient();
  if(LED1status){
    digitalWrite(LED1pin, HIGH);
    digitalWrite(rele, HIGH);  
  }else{
    digitalWrite(LED1pin, LOW);
    digitalWrite(rele, LOW);      
  }    
//  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_OnConnect() {
  LED1status = false;
  LED2status = LOW;
  Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
}

void handle_led1on() {
  LED1status = true;
  Serial.println("GPIO4 Status: ON");
  server.send(200, "text/html", SendHTML(true,LED2status)); 
}

void handle_led1off() {
  LED1status = false;
  Serial.println("GPIO4 Status: OFF");
  server.send(200, "text/html", SendHTML(false,LED2status)); 
}

void handle_led2on() {
  LED2status = HIGH;
  Serial.println("GPIO5 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handle_led2off() {
  LED2status = LOW;
  Serial.println("GPIO5 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status,false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat,uint8_t led2stat){
//Iniciando o buffer que ira conter a pagina HTML que sera enviada para o browser.
String buf = "";  
  buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
  buf = "<html lang=\"port\">";

  buf += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=yes\"/>\r\n";
  buf =+ "<meta http-equiv=\"refresh\" content=\"10\">";
  buf += "<title>Dispositivo</title>";


  if (LED1status==true) {
  buf += "LIGADO";
  }else{
  buf += "DESLIGADO";
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
//p  buf += String(rtc.getTimeStr(FORMAT_SHORT));

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
  //  buf += "</div> ";
  
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
//  buf += "</div> ";
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
  //************
  // buf += "<p>Pagina atualizada as "; // DIV para hora
  //  buf += String(hora);
  buf += "</body>";
  buf += "</html>\n";

  return buf;
//  return server.send(200, "text/html", SendHTML(LED1status,true)); 
}


String SendHTML2(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Access Point(AP) Mode</h3>\n";
  
   if(led1stat)
  {ptr +="<p>LED1 Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";}
  else
  {ptr +="<p>LED1 Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>\n";}

  if(led2stat)
  {ptr +="<p>LED2 Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";}
  else
  {ptr +="<p>LED2 Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
//  return server.send(200, "text/html", SendHTML(LED1status,true)); 
}

char* string_substring(char str[], int start, int end) {
    int i, j;
    char *sub; 
     
    // Verifica valores incompatíveis e 
    // retorna NULL
    if(start >= end || end > strlen(str)) {
        return NULL;
    }
     
    // Aloca memória para a substring
    sub = (char *) malloc(sizeof(char) * (end - start + 1));
     
    // Copia a substring para a variável
    for(i = start, j = 0; i < end; i++, j++) {
        sub[j] = str[i];
    }
     
    // Terminador de string
    sub[j] = '&#092;&#048;';
     
    return sub;
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

void handleRoot() {
    server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handleLedOff() {
  stateLED = HIGH;
  digitalWrite(pin, stateLED);
  LED1status = true;      
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void handleLedOn() {
  stateLED = LOW;
  digitalWrite(pin, stateLED);
  LED1status = false;      
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}


void setHLu() {
    relhoraLiga++; 
    if (relhoraLiga > 23) {
      relhoraLiga = 00;
    } 
  horaDesl=relhoraLiga;
  server.send(200, "text/html", SendHTML(LED1status,true));     
}

void setHLd() {
    relhoraLiga--; 
    if (relhoraLiga < 00) {
      relhoraLiga = 23;
    }  
    horaDesl=relhoraLiga;    
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}


void setMLu() {
    relminutoLiga = relminutoLiga + 5;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }
    minutoDesl=relminutoLiga;
  server.send(200, "text/html", SendHTML(LED1status,true)); 
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
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void set1MLu() {
    relminutoLiga = relminutoLiga + 1;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }    
           minutoDesl=relminutoLiga;
  server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void set1MLd() {
    relminutoLiga = relminutoLiga -1;
    if (relminutoLiga < 0) {
      relminutoLiga = 59;
    }          
    minutoDesl=relminutoLiga;    
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void salva_relogio()  {
  //novo_horario==1
    save_relogio(relhoraLiga, relminutoLiga);   
    novo_relogio=0;     
  //server.send(200, "text/html", SendHTML(LED1status,true)); 
    server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void save_relogio(int relhoraliga, int relminutoliga) {
//p  rtc.setTime(relhoraliga, relminutoliga, 0);    //Define o horario
  rtc.adjust(DateTime(2021, 1, 21, relhoraliga, relminutoliga, 0));
  novo_relogio = 0;
  // rtc.setDate(5, 3, 2018);   //Define o dia, mes e ano
//  server.send(200, "text/html", SendHTML(LED1status,true)); 
    server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void setHDu() {
    horaDesl++;
    if (horaDesl > 23) {
      horaDesl = 00;
    }   
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void setHDd() {
    horaDesl--;
    if (horaDesl < 00) {
      horaDesl = 23;
    }
    if(horaDesl<relhoraLiga){
      horaDesl++;
    }
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void setMDu() {
    minutoDesl = minutoDesl + 5;
    if (minutoDesl > 59) {
      minutoDesl = 00;
    }
  server.send(200, "text/html", SendHTML(LED1status,true));     
  } 

void setMDd() {
    minutoDesl = minutoDesl - 5;
    if (minutoDesl < 00) {
      minutoDesl = 55;
    }
    if (minutoDesl < 10) {
      minutoDesl = 05;
    }    
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void set1MDu() {
    minutoDesl = minutoDesl + 1;
    if (minutoDesl > 59) {
      minutoDesl = 00;
    }
  server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void set1MDd() {
    minutoDesl = minutoDesl - 1;
    if (minutoDesl < 00) {
      minutoDesl = 59;
    }    
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}


void rele_on() {
    stateRelay = false;
    status_gpio = 0;
  LED1status = true;    
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

void rele_off() {
    stateRelay = true;
    status_gpio = 1;
      LED1status = false;    
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}

  //Analisando a requisicao recebida para decidir se liga ou desliga a lampada
void liga()  {
    digitalWrite(pin, LOW);
    novo_horario=0;   
    lido=1;    
    ledState=false;    
    LED1status = true;
    server.send(200, "text/html", SendHTML(true,LED2status)); 
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
    server.send(200, "text/html", SendHTML(false,LED2status)); 
    botao_liga=0;
    botao_desliga=0;
    digitalWrite(rele, LOW);     
}

void limpa()  {
    Clear_Data();
    novo_horario=0;   
    lido=1;  
    botao_liga=0, botao_desliga=0;
    server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void ler()  {
    Read_Data();
    novo_horario=0;   
    lido=1;  
    botao_liga=0, botao_desliga=0;
    server.send(200, "text/html", SendHTML(LED1status,true)); 
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
  server.send(200, "text/html", SendHTML(LED1status,true)); 
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
    server.send(200, "text/html", SendHTML(LED1status,true)); 
}  

void novo()  {
    lido = 0;
    novo_horario=1;
    botao_liga=0, botao_desliga=0;    
    server.send(200, "text/html", SendHTML(LED1status,true)); 
}

/*
void relogio()  {
    char horaRel[5];
//p    strcpy(horaRel,rtc.getTimeStr(FORMAT_SHORT)); 
    char horas[2];
//p    strcpy(horas,string_substring(rtc.getTimeStr(FORMAT_SHORT), 0, 3));
    relhoraLiga=atoi(horas);
    char minutos[2];
//p    strcpy(minutos,string_substring(rtc.getTimeStr(FORMAT_SHORT), 3, 5));
    relminutoLiga=atoi(minutos); 
    novo_relogio=1;
  server.send(200, "text/html", SendHTML(LED1status,true)); 
} 
*/

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
    server.send(200, "text/html", SendHTML(LED1status,LED2status)); 
} 

void setrHLu() {
    relhoraLiga++; 
    if (relhoraLiga > 23) {
      relhoraLiga = 00;
    } 
    server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void setrHLd() {
    relhoraLiga--; 
    if (relhoraLiga < 00) {
      relhoraLiga = 23;
    }      
    server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void setrMLu(){
    relminutoLiga = relminutoLiga + 5;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }  
    server.send(200, "text/html", SendHTML(LED1status,true)); 
}  

void setrMLd() {
    relminutoLiga = relminutoLiga -5;
    if (relminutoLiga < 0) {
      relminutoLiga = 55;
    }       
    server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void setr1MLu() {
    relminutoLiga = relminutoLiga + 1;
    if (relminutoLiga > 59) {
      relminutoLiga = 00;
    }       
    server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void setr1MLd() {
    relminutoLiga = relminutoLiga -1;
    if (relminutoLiga < 0) {
      relminutoLiga = 59;
    }        
    server.send(200, "text/html", SendHTML(LED1status,true)); 
} 

void cancelar()  {
  lido = 1;
  novo_horario=0;
  qtde_de_horarios--;  
  novo_relogio = 0;
  server.send(200, "text/html", SendHTML(LED1status,true)); 
}
  
void handleNotFound() {
//p  digitalWrite ( LED_BUILTIN, 0 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  //p digitalWrite ( LED_BUILTIN, 1 ); //turn the built in LED on pin DO of NodeMCU off
}

void response(){
  //String response(){
  //Iniciando o buffer que ira conter a pagina HTML que sera enviada para o browser.
  String buf = "";  
  buf = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
  buf = "<html lang=\"port\">";

  buf += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\r\n";
  buf += "<title>Dispositivo</title>";

  /*
  if (digitalRead(ledPin)==false) {
  buf += "LIGADO";
  }else{
  buf += "DESLIGADO";
  }
  */
  buf += "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:100%;} body{text-align: center;font-family:verdana;}</style>";
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

//  buf += String(rtc.getDateStr(FORMAT_SHORT));
  buf += " * ";
//p  buf += String(rtc.getTimeStr(FORMAT_SHORT));

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
  //  buf += "</div> ";
  
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
//  buf += "</div> ";
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
  //************
  // buf += "<p>Pagina atualizada as "; // DIV para hora
  //  buf += String(hora);
  buf += "</body>";
  buf += "</html>\n";
//  return buf;
  server.send(200, "text/html", buf);
}


void setup_old() {

//Após piscar 10x sem erros, irá entrar no loop e o LED começara a piscar
// rapidamente indicando que não ocorreu erros.
 
//Este código efetua o feed do HW WDT, então logo após as 10 piscadas,
// começara a piscar rapidamente pois entrou no loop.
  
//p  pinMode ( ledPin, OUTPUT );
//p  digitalWrite ( ledPin, 1 );
  
  delay(1000);
  Serial.begin(9600);
  Serial.println();
  Serial.println("Configuring access point...");

  //set-up the custom IP address
 // WiFi.mode(WIFI_AP_STA);
//  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  

/*
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  
  // You can remove the password parameter if you want the AP to be open. 
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //Preparando o pino, que esta lidago ao rele
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);    
  ledState=false;
  delay(10000);  
  digitalWrite(pin, HIGH);    
  ledState=true;
*/
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

  //**EEPROM**
  EEPROM.begin(512);
//  delay(250);

  Read_Data();    
}

void loop_old() {
/*   
  //ledState = digitalRead(ledPin);

//  String str=rtc.getTimeStr();
  //String str2="14:38:00";

String str="14:38:00";
  if(ligado(str)){
    digitalWrite(pin, LOW);
    novo_horario=0;   
    lido=1;    
    ledState=false;    
 //   server.send(200, "text/html", SendHTML(LED1status,true)); 
  }else{
    digitalWrite(pin, HIGH);
    novo_horario=0; 
    lido=1;    
    ledState=true;    
  // server.send(200, "text/html", SendHTML(LED1status,true)); 
  }
     server.send(200, "text/html", SendHTML(LED1status,true)); 

  //Aguarda 1 segundo e repete o processo
  delay (1000);  
  server.handleClient();
  */
}


boolean ligarAgora(String ligar){
//    Serial.print("LIGA : "+ligar);  
    int x;
    String hora;
//    qtde_de_horarios = EEPROM.read(addr_inicial);   
//    Serial.print(qtde_de_horarios);
    for(x=0;x<qtde_de_horarios;x++){
      if(vet_minutoliga[x]<10){
        hora=String(vet_horaliga[x])+":0"+String(vet_minutoliga[x])+":00";
      }else{
        hora=String(vet_horaliga[x])+":"+String(vet_minutoliga[x])+":00";
      }
      //strcat(hora,vet_horaliga[x]);
//      Serial.println(hora);
      if(ligar==hora){
        return true;
      }                           
    }  
 return false;
}

boolean desligarAgora(String ligar){
//    Serial.print("DESLIGA : "+ligar);  
    int x;
    String hora;
//    qtde_de_horarios = EEPROM.read(addr_inicial);   
//    Serial.print(qtde_de_horarios);
    for(x=0;x<qtde_de_horarios;x++){
      if(vet_minutodesliga[x]<10){
        hora=String(vet_horadesliga[x])+":0"+String(vet_minutodesliga[x])+":00";
      }else{
        hora=String(vet_horadesliga[x])+":"+String(vet_minutodesliga[x])+":00";
      }      
      //strcat(hora,vet_horaliga[x]);
//      Serial.println(hora);
      if(ligar==hora){
        return true;
      }                           
    }  
 return false;
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
//    Serial.print("DESLIGA : "+ligar);  
    int x;
    String horaDesliga, horaLiga;
//    qtde_de_horarios = EEPROM.read(addr_inicial);   
//    Serial.print(qtde_de_horarios);
    for(x=0;x<qtde_de_horarios;x++){
      if(vet_minutodesliga[x]<10){
//        horaDesliga=String(vet_horadesliga[x])+":0"+String(vet_minutodesliga[x])+":00";
//        horaLiga=String(vet_horaliga[x])+":0"+String(vet_minutoliga[x])+":00";        
        horaDesliga=(String(vet_horadesliga[x])+"0"+String(vet_minutodesliga[x])+"00");
        horaLiga=(String(vet_horaliga[x])+"0"+String(vet_minutoliga[x])+"00");       
      }else{
//        horaDesliga=String(vet_horadesliga[x])+":"+String(vet_minutodesliga[x])+":00";
//        horaLiga=String(vet_horaliga[x])+":"+String(vet_minutoliga[x])+":00";        
        horaDesliga=(String(vet_horadesliga[x])+""+String(vet_minutodesliga[x])+"00");
        horaLiga=(String(vet_horaliga[x])+""+String(vet_minutoliga[x])+"00");        
      }      
      //strcat(hora,vet_horaliga[x]);
//      Serial.println(hora);
//      Serial.println(horaLiga);
//      Serial.println(horaDesliga);

//      int horaliga_int=atoi(horaLiga);
//      char horas[6];
//      strcpy(horas,string_substring(horaLiga,0, 6));
//      horaLiga=string_substring(horaLiga,0, 6);
      int horaLiga_int = stringToInt(horaLiga);
      //atoi(horas);
      int horaDesliga_int=stringToInt(horaDesliga);
      if(hora>=horaLiga_int&&hora<=horaDesliga_int){
//      if(hora>=horaLiga&&hora<=horaDesliga){
//      Serial.println(hora);
//     Serial.println(horaLiga);
//      Serial.println(horaDesliga);        
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
