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

int novo_horario=0;
int novo_relogio=0;
int relhoraLiga=0;
int relminutoLiga=0;
int qtde_de_horarios=0;
int lido =0;
int inserido=0;
int botao_liga=0, botao_desliga=0;

bool temHorarioConfigurado = false;
bool dispositivoEstaLigado = false;

int horaLiga = 0;
int minutoLiga = 0;
int horaDesliga = 0;
int minutoDesliga = 0;


const int pin = 18; //Equivalente ao D2 no NodeMCU
uint8_t rele = 18;
uint8_t addr_inicial = 8;
int  horaDesl=0;
int  minutoDesl=0;

void escreverValorInteiroNaEEPROM(int endereco1, int endereco2, int valor){

  EEPROM.begin(512);    

  int primeiroCaractereDoValor = valor/10;
  int segundoCaractereDoValor = (valor%10);

  if(primeiroCaractereDoValor == 0){
    EEPROM.write(endereco1, 0);
    EEPROM.write(endereco2, segundoCaractereDoValor);
  } else {
    EEPROM.write(endereco1, primeiroCaractereDoValor);
    EEPROM.write(endereco2, segundoCaractereDoValor);
  }

  delay(1000);
  EEPROM.end();    


}

int lerValorInteiroDaEEPROM(int endereco1, int endereco2){

  EEPROM.begin(512);    

  int primeiroCaractereDoValor = EEPROM.read(endereco1);
  int segundoCaractereDoValor = EEPROM.read(endereco2);

  EEPROM.end();    

  if(primeiroCaractereDoValor == 0){
    return segundoCaractereDoValor;
  } else {
    return primeiroCaractereDoValor * 10 + segundoCaractereDoValor;
  }
}


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
  buf += "          margin: 4rem;";
  buf += "          height: fit-content;";
  buf += "          padding: 4rem;";
  buf += "          background-color: #D9D9D9;";
  buf += "          border-radius: 10px;";
  buf += "          display: flex;";
  buf += "          align-items: center;";
  buf += "          flex-direction: column;";
  buf += "          gap: 4rem;";
  buf += "      }";
  buf += "      a:hover{";
  buf += "          transform: scale(1.05);";
  buf += "          opacity: 0.7;";
  buf += "      }";
  buf += "      a {";
  buf += "          margin-top: 1rem;";
  buf += "          border: none;";
  buf += "          cursor: pointer;";
  buf += "          transition: 200ms;";
  buf += "          font-size: 2.5rem;";
  buf += "          font-weight: 700;";
  buf += "          border-radius: 10px;";
  buf += "          padding: 1rem;";
  buf += "          text-decoration: none;";
  buf += "          background: #C4C4C4;";
  buf += "          box-shadow: 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "          border-radius: 10px;";
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
  buf += "          width: 100%;";
  buf += "      }";
  buf += "      #desligado {";
  buf += "          color: #D45C5C;";
  buf += "      }";
  buf += "      #botaodesligado {";
  buf += "          background-color: #8AC880;";
  buf += "          color: #ffffff;";
  buf += "      }";
  buf += "      #ligado {";
  buf += "          color: #8AC880;";
  buf += "      }";
  buf += "      #botaoligado {";
  buf += "          color: #ffffff;";
  buf += "          background-color: #D45C5C;";
  buf += "      }";

  buf += "      .horario {";
  buf += "          display: flex;";
  buf += "          justify-content: center;";
  buf += "          gap: 1rem;";
  buf += "          flex-direction: row;";
  buf += "          margin-top: 1rem;";
  buf += "      }";
  buf += "      .horario div {";
  buf += "          display: flex;";
  buf += "          justify-content: center;";
  buf += "          align-items: center;";
  buf += "          background-color: #CDCDCD;";
  buf += "          height: 7rem;";
  buf += "          width: 7rem;";
  buf += "          border-radius: 10px;";
  buf += "          font-size: 4rem;";
  buf += "          font-weight: 500;";
  buf += "          margin-bottom: 2rem;";
  buf += "      }";
  buf += "      .separador-de-hora {";
  buf += "          font-size: 4rem;";
  buf += "          align-self: center;";
  buf += "      }";
  buf += "        #ligar {";
  buf += "            color: #8AC880;";
  buf += "        }";
  buf += "        #desligar {";
  buf += "            color: #B55454;";
  buf += "        }";
  buf += "        .footer-horarios {";
  buf += "            display: flex;";
  buf += "            flex-direction: column;";
  buf += "            align-items: center;";
  buf += "            gap: 1rem;";
  buf += "        }";
  buf += "        .footer-horarios a {";
  buf += "            width: 28rem;";
  buf += "        }";
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

  buf += "                <div>";
  if(temHorarioConfigurado){
    buf += "                    <div>";
    buf += "                        <h1>O sistema esta configurado para <span id='ligar'>ligar</span></h1>";
    buf += "                        <div class='horario'>";
    buf += "                            <div>";
    buf += horaLiga;
    buf += "                            </div><label class='separador-de-hora'>:</label>";
    buf += "                            <div>";
    buf += minutoLiga;
    buf += "                          </div>";
    buf += "                        </div>";
    buf += "                    </div>";
    buf += "                    <div>";
    buf += "                        <h1>e <span id='desligar'>desligar</span></h1>";
    buf += "                        <div class='horario'>";
    buf += "                            <div>";
    buf += horaDesliga;
    buf += "                        </div><label class='separador-de-hora'>:</label>";
    buf += "                            <div>";
    buf += minutoDesliga;
    buf += "              </div>";
    buf += "                        </div>";
    buf += "                    </div>";
  } else {
    buf += "<h1>Você não configurou um horario para <span id='ligar'>ligar</span> ou <span id='desligar'>desligar</span></h1>";
  }

  buf += "                    <div class='footer-horarios'>";

  if(temHorarioConfigurado){
    buf += "                        <a href='/configurar-horario'>editar horario</a>";
    buf += "                        <a href='/limpa'>apagar horario</a>";
  } else {
    buf += "                        <a href='/configurar-horario'>configurar horario</a>";
  }
  buf += "                    </div>";


  buf += "                </div>";


	// char date[10] = "hh:mm:ss";
	char hora[10] = "hh";
	char minuto[10] = "mm";
	char segundos[10] = "ss";
	rtc.now().toString(hora);
	rtc.now().toString(minuto);
	rtc.now().toString(segundos);

  buf += "                <div>";
  buf += "                    <h1>horario atual:</h1>";
  buf += "                    <div class='horario'>";
  buf += "                        <div id='hora-atual'>";
  buf += hora;
  buf += "                        </div> <label class='separador-de-hora'>:</label>";

  buf += "                        <div id='minuto-atual'>";
  buf += minuto;
  buf += "                        </div>";
  buf += "                    </div>";
  buf += "                  <a href='/configurar-relogio'>configurar relogio</a>";
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

String SendHTMLConfigurarRelogio(){
  String buf = "";
  buf += "<!DOCTYPE html>";
  buf += "<html lang='pt-BR'>";
  buf += "<head>";
  buf += "    <meta charset='UTF-8'>";
  buf += "    <meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  buf += "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  buf += "    <title>Dispositivo remoto</title>";
  buf += "    <link rel='preconnect' href='https://fonts.googleapis.com'>";
  buf += "    <link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>";
  buf += "    <link href='https://fonts.googleapis.com/css2?family=Inter:wght@200;500;600;700&display=swap' rel='stylesheet'>";
  buf += "</head>";
  buf += "<style>";
  buf += "    * {";
  buf += "        box-sizing: border-box;";
  buf += "        margin: 0;";
  buf += "        padding: 0;";
  buf += "        font-family: 'Inter', sans-serif;";
  buf += "        text-align: center;";
  buf += "        color: #403937;";
  buf += "    }";
  buf += "    :root {";
  buf += "        font-size: 62.5%;";
  buf += "    }";
  buf += "    body {";
  buf += "        width: 100vw;";
  buf += "        height: 100vh;";
  buf += "        background: linear-gradient(107.56deg, #5D5F28 0%, rgba(32, 90, 38, 0.557143) 32.29%, rgba(26, 87, 32, 0.264286) 53.65%, rgba(30, 30, 30, 0) 100%);";
  buf += "        background-color: #1E1E1E;";
  buf += "        font-size: 1.6rem;";
  buf += "    }";
  buf += "    .wrapper {";
  buf += "        display: flex;";
  buf += "        height: 100vh;";
  buf += "        width: 100vw;";
  buf += "        align-items: center;";
  buf += "        justify-content: center;";
  buf += "    }";
  buf += "    .app {";
  buf += "        width: 90vw;";
  buf += "        margin: 4rem;";
  buf += "        height: fit-content;";
  buf += "        padding: 2rem;";
  buf += "        background-color: #D9D9D9;";
  buf += "        border-radius: 10px;";
  buf += "        display: flex;";
  buf += "        align-items: center;";
  buf += "        flex-direction: column;";
  buf += "        gap: 4rem;";
  buf += "    }";
  buf += "    .timer-container {";
  buf += "        display: flex;";
  buf += "        flex-direction: column;";
  buf += "        gap: 2.1rem;";
  buf += "    }";
  buf += "    button {";
  buf += "        width: 7rem;";
  buf += "        height: 7rem;";
  buf += "        background: #8AC880;";
  buf += "        border: none;";
  buf += "        border-radius: 10px;";
  buf += "        box-shadow: 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "        font-weight: 900;";
  buf += "        font-size: 36px;";
  buf += "        color: #ffffff;";
  buf += "        cursor: pointer;";
  buf += "        transition: 200ms;";
  buf += "    }";
  buf += "    button:hover{";
  buf += "        transform: scale(1.05);";
  buf += "    }";
  buf += "    .timer-container div {";
  buf += "        display: flex;";
  buf += "        justify-content: center;";
  buf += "        align-items: center;";
  buf += "        width: 7rem;";
  buf += "        height: 7rem;";
  buf += "        background: #CDCDCD;";
  buf += "        box-shadow: inset 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "        border-radius: 1rem;";
  buf += "        font-size: 4rem;";
  buf += "        ";
  buf += "    }";
  buf += "    .timer {";
  buf += "        display: flex;";
  buf += "        flex-direction: row;";
  buf += "        justify-content: center;";
  buf += "        align-items: center;";
  buf += "        gap: 1.1rem;";
  buf += "    }";
  buf += "    .timer p {";
  buf += "        font-size: 4rem;";
  buf += "    }";
  buf += "    .footer {";
  buf += "        display: flex;";
  buf += "        width: 100%;";
  buf += "        max-width: 97.4rem;";
  buf += "    }";
  buf += "    .footer button {";
  buf += "        border: 0;";
  buf += "        border-radius: 10px 0 0 10px;";
  buf += "        width: 100%;";
  buf += "        padding: 1rem;";
  buf += "        background-color: #3B64B2;";
  buf += "        color: #FFFFFF;";
  buf += "        font-weight: 700;";
  buf += "        font-size: 3rem;";
  buf += "    }";
  buf += "    #cancel-button {";
  buf += "        border-radius: 0 10px 10px 0;";
  buf += "        background-color: #B2433B;";
  buf += "    }";
  buf += "</style>";
  buf += "<body>";
  buf += "    <div class='wrapper'>";
  buf += "        <div class='app'>";
  buf += "            <h1>configure o relogio com o horario atual</h1>";
  buf += "            <h1>horario atual:</h1>";
  buf += "            <div class='timer'>";
  buf += "                <div class='timer-container'>";
  buf += "                    <button onclick='somarHora(5)'>+5</button>";
  buf += "                    <button onclick='somarHora(1)'>+1</button>";
  buf += "                    <div id='hora'>00</div>";
  buf += "                    <button onclick='somarHora(-1)' style='background-color: #D45C5C;'>-1</button>";
  buf += "                    <button onclick='somarHora(-5)' style='background-color: #D45C5C;'>-5</button>";
  buf += "                </div>";
  buf += "                <p>:</p>";
  buf += "                <div class='timer-container'>";
  buf += "                    <button onclick='somarMinutos(5)'>+5</button>";
  buf += "                    <button onclick='somarMinutos(1)'>+1</button>";
  buf += "                    <div id='minuto'>00</div>";
  buf += "                    <button onclick='somarMinutos(-1)' style='background-color: #D45C5C;'>-1</button>";
  buf += "                    <button onclick='somarMinutos(-5)' style='background-color: #D45C5C;'>-5</button>";
  buf += "                </div>";
  buf += "            </div>";
  buf += "              <div class='footer'>";
  buf += "                <button onclick='confirmar()'>confirmar</button>";
  buf += "                <button onclick='cancelar()' id='cancel-button'>cancelar</button>";
  buf += "            </div>";
  buf += "        </div>";
  buf += "    </div>";
  buf += "</body>";
  buf += "<script>";
  buf += "  let horas = 0\n";
  buf += "  let minutos = 0\n";
  buf += "  function atualizarDisplay() {\n";
  buf += "      const displayMinutos = window.document.getElementById('minuto')\n";
  buf += "      const displayHoras = window.document.getElementById('hora')\n";
  buf += "      displayMinutos.innerHTML = minutos\n";
  buf += "      displayHoras.innerHTML = horas\n";
  buf += "  }\n";
  buf += "  function somarHora(valor) {\n";
  buf += "      if (horas + valor > 23) {\n";
  buf += "          horas = 0\n";
  buf += "      } else if (horas + valor < 0) {\n";
  buf += "          horas = 23\n";
  buf += "      } else {\n";
  buf += "          horas = horas + valor\n";
  buf += "      }\n";
  buf += "      atualizarDisplay()\n";
  buf += "  }\n";
  buf += "  function somarMinutos(valor) {\n";
  buf += "      if (minutos + valor > 59) {\n";
  buf += "          minutos = 0\n";
  buf += "      } else if (minutos + valor < 0) {\n";
  buf += "          minutos = 59\n";
  buf += "      } else {\n";
  buf += "          minutos = minutos + valor\n";
  buf += "      }\n";
  buf += "      atualizarDisplay()\n";
  buf += "  }";
  buf += "  function cancelar(){\n";
  buf += "      window.location.replace('/')\n";
  buf += "  }\n";
  buf += "  function confirmar(){\n";
  buf += "      window.location.replace(`/setar-relogio?hora=${horas}&minuto=${minutos}`)\n";
  buf += "  }\n";
  buf += "</script>";
  buf += "</html>";
  return buf;
}

String SendHTMLConfigurarHorario(){
  String buf = "";
  buf += "<!DOCTYPE html>";
  buf += "<html lang='pt-BR'>";
  buf += "<head>";
  buf += "    <meta charset='UTF-8'>";
  buf += "    <meta http-equiv='X-UA-Compatible' content='IE=edge'>";
  buf += "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  buf += "    <title>Dispositivo remoto</title>";
  buf += "";
  buf += "    <link rel='preconnect' href='https://fonts.googleapis.com'>";
  buf += "    <link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>";
  buf += "    <link href='https://fonts.googleapis.com/css2?family=Inter:wght@200;500;600;700&display=swap' rel='stylesheet'>";
  buf += "</head>";
  buf += "<style>";
  buf += "    * {";
  buf += "        box-sizing: border-box;";
  buf += "        margin: 0;";
  buf += "        padding: 0;";
  buf += "        font-family: 'Inter', sans-serif;";
  buf += "        text-align: center;";
  buf += "        color: #403937;";
  buf += "    }";
  buf += "    :root {";
  buf += "        font-size: 62.5%;";
  buf += "    }";
  buf += "    body {";
  buf += "        width: 100vw;";
  buf += "        height: 100vh;";
  buf += "        background: linear-gradient(107.56deg, #5D5F28 0%, rgba(32, 90, 38, 0.557143) 32.29%, rgba(26, 87, 32, 0.264286) 53.65%, rgba(30, 30, 30, 0) 100%);";
  buf += "        background-color: #1E1E1E;";
  buf += "        font-size: 1.6rem;";
  buf += "    }";
  buf += "    .wrapper {";
  buf += "        display: flex;";
  buf += "        height: 100vh;";
  buf += "        width: 100vw;";
  buf += "        align-items: center;";
  buf += "        justify-content: center;";
  buf += "    }";
  buf += "    .app {";
  buf += "        width: 90vw;";
  buf += "        margin: 4rem;";
  buf += "        height: fit-content;";
  buf += "        padding: 2rem;";
  buf += "        background-color: #D9D9D9;";
  buf += "        border-radius: 10px;";
  buf += "        display: flex;";
  buf += "        align-items: center;";
  buf += "        flex-direction: column;";
  buf += "        gap: 4rem;";
  buf += "    }";
  buf += "    button {";
  buf += "        width: 7rem;";
  buf += "        height: 7rem;";
  buf += "        background: #8AC880;";
  buf += "        border: none;";
  buf += "        border-radius: 10px;";
  buf += "        box-shadow: 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "        font-weight: 900;";
  buf += "        font-size: 36px;";
  buf += "        color: #ffffff;";
  buf += "        cursor: pointer;";
  buf += "        transition: 200ms;";
  buf += "    }";
  buf += "    button:hover {";
  buf += "        transform: scale(1.05);";
  buf += "    }";
  buf += "    a:hover {";
  buf += "        transform: scale(1.05);";
  buf += "    }";
  buf += "    .timer-container div {";
  buf += "        display: flex;";
  buf += "        justify-content: center;";
  buf += "        align-items: center;";
  buf += "        width: 7rem;";
  buf += "        height: 7rem;";
  buf += "        background: #CDCDCD;";
  buf += "        box-shadow: inset 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "        border-radius: 1rem;";
  buf += "        font-size: 4rem;";
  buf += "    }";
  buf += "    .timer {";
  buf += "        display: flex;";
  buf += "        flex-direction: row;";
  buf += "        justify-content: center;";
  buf += "        align-items: center;";
  buf += "        gap: 1.1rem;";
  buf += "    }";
  buf += "    .timer p {";
  buf += "        font-size: 4rem;";
  buf += "    }";
  buf += "    .footer {";
  buf += "        display: flex;";
  buf += "        width: 100%;";
  buf += "        max-width: 97.4rem;";
  buf += "    }";
  buf += "    .footer button {";
  buf += "        border: 0;";
  buf += "        border-radius: 10px 0 0 10px;";
  buf += "        width: 100%;";
  buf += "        padding: 1rem;";
  buf += "        background-color: #3B64B2;";
  buf += "        color: #FFFFFF;";
  buf += "        font-weight: 700;";
  buf += "        font-size: 3rem;";
  buf += "        text-decoration: none;";
  buf += "        transition: 200ms;";
  buf += "    }";
  buf += "    #cancel-button {";
  buf += "        border-radius: 0 10px 10px 0;";
  buf += "        background-color: #B2433B;";
  buf += "    }";
  buf += "    .timers-container {";
  buf += "        display: flex;";
  buf += "        flex-direction: row;";
  buf += "        gap: 5rem;";
  buf += "    }";
  buf += "    .timer {";
  buf += "        display: flex;";
  buf += "        flex-direction: column;";
  buf += "    }";
  buf += "    .timer-selector {";
  buf += "        display: flex;";
  buf += "        flex-direction: row;";
  buf += "        align-items: center;";
  buf += "        gap: 1rem;";
  buf += "    }";
  buf += "    .timer-container {";
  buf += "        display: flex;";
  buf += "        flex-direction: column;";
  buf += "        gap: 2.1rem;";
  buf += "    }";
  buf += "</style>";
  buf += "<body>";
  buf += "    <div class='wrapper'>";
  buf += "        <div class='app'>";
  buf += "            <h1>configure o horario para ligar e desligar o aparelho</h1>";
  buf += "";
  buf += "            <div class='timers-container'>";
  buf += "                <div class='timer'>";
  buf += "                    <h1>liga:</h1>";
  buf += "                    <div class='timer-selector'>";
  buf += "                        <div class='timer-container'>";
  buf += "                            <button onclick='somarHoraLiga(5)'>+5</button>";
  buf += "                            <button onclick='somarHoraLiga(1)'>+1</button>";
  buf += "                            <div id='horaLiga'>0</div>";
  buf += "                            <button onclick='somarHoraLiga(-1)' style='background-color: #D45C5C;'>-1</button>";
  buf += "                            <button onclick='somarHoraLiga(-5)' style='background-color: #D45C5C;'>-5</button>";
  buf += "                        </div>";
  buf += "                        <p>:</p>";
  buf += "                        <div class='timer-container'>";
  buf += "                            <button onclick='somarMinutosLiga(5)'>+5</button>";
  buf += "                            <button onclick='somarMinutosLiga(1)'>+1</button>";
  buf += "                            <div id='minutoLiga'>0</div>";
  buf += "                            <button onclick='somarMinutosLiga(-1)' style='background-color: #D45C5C;'>-1</button>";
  buf += "                            <button onclick='somarMinutosLiga(-5)' style='background-color: #D45C5C;'>-5</button>";
  buf += "                        </div>";
  buf += "                    </div>";
  buf += "                </div>";
  buf += "                <div class='timer'>";
  buf += "                    <h1>desliga:</h1>";
  buf += "                    <div class='timer-selector'>";
  buf += "                        <div class='timer-container'>";
  buf += "                            <button onclick='somarHoraDesliga(5)'>+5</button>";
  buf += "                            <button onclick='somarHoraDesliga(1)'>+1</button>";
  buf += "                            <div id='horaDesliga'>0</div>";
  buf += "                            <button onclick='somarHoraDesliga(-1)' style='background-color: #D45C5C;'>-1</button>";
  buf += "                            <button onclick='somarHoraDesliga(-5)' style='background-color: #D45C5C;'>-5</button>";
  buf += "                        </div>";
  buf += "                        <p>:</p>";
  buf += "                        <div class='timer-container'>";
  buf += "                            <button onclick='somarMinutosDesliga(5)'>+5</button>";
  buf += "                            <button onclick='somarMinutosDesliga(1)'>+1</button>";
  buf += "                            <div id='minutoDesliga'>0</div>";
  buf += "                            <button onclick='somarMinutosDesliga(-1)' style='background-color: #D45C5C;'>-1</button>";
  buf += "                            <button onclick='somarMinutosDesliga(-5)' style='background-color: #D45C5C;'>-5</button>";
  buf += "                        </div>";
  buf += "                    </div>";
  buf += "                </div>";
  buf += "            </div>";
  buf += "            <div class='footer'>";
  buf += "                <button onclick='confirmar()'>confirmar</button>";
  buf += "                <button onclick='cancelar()' id='cancel-button' href='/'>cancelar</a>";
  buf += "            </div>";
  buf += "        </div>";
  buf += "    </div>";
  buf += "</body>";
  buf += "<script>";
  buf += "    let horasLiga = 0\n";
  buf += "    let minutosLiga = 0\n";
  buf += "    function atualizarDisplayLiga() {\n";
  buf += "        const displayMinutos = window.document.getElementById('minutoLiga')\n";
  buf += "        const displayHoras = window.document.getElementById('horaLiga')\n";
  buf += "        displayMinutos.innerHTML = minutosLiga\n";
  buf += "        displayHoras.innerHTML = horasLiga\n";
  buf += "    }\n";
  buf += "    function somarHoraLiga(valor) {\n";
  buf += "        if (horasLiga + valor > 23) {\n";
  buf += "            horasLiga = 0\n";
  buf += "        } else if (horasLiga + valor < 0) {\n";
  buf += "            horasLiga = 23\n";
  buf += "        } else {\n";
  buf += "            horasLiga = horasLiga + valor\n";
  buf += "        }\n";
  buf += "        atualizarDisplayLiga()\n";
  buf += "    }\n";
  buf += "    function somarMinutosLiga(valor) {\n";
  buf += "        if (minutosLiga + valor > 59) {\n";
  buf += "            minutosLiga = 0\n";
  buf += "        } else if (minutosLiga + valor < 0) {\n";
  buf += "            minutosLiga = 59\n";
  buf += "        } else {\n";
  buf += "            minutosLiga = minutosLiga + valor\n";
  buf += "        }\n";
  buf += "        atualizarDisplayLiga()\n";
  buf += "    }\n";
  buf += "    let horasDesliga = 0\n";
  buf += "    let minutosDesliga = 0\n";
  buf += "    function atualizarDisplayDesliga() {\n";
  buf += "        const displayMinutos = window.document.getElementById('minutoDesliga')\n";
  buf += "        const displayHoras = window.document.getElementById('horaDesliga')\n";
  buf += "        displayMinutos.innerHTML = minutosDesliga\n";
  buf += "        displayHoras.innerHTML = horasDesliga\n";
  buf += "    }\n";
  buf += "    function somarHoraDesliga(valor) {\n";
  buf += "        if (horasDesliga + valor > 23) {\n";
  buf += "            horasDesliga = 0\n";
  buf += "        } else if (horasDesliga + valor < 0) {\n";
  buf += "            horasDesliga = 23\n";
  buf += "        } else {\n";
  buf += "            horasDesliga = horasDesliga + valor\n";
  buf += "        }\n";
  buf += "        atualizarDisplayDesliga()\n";
  buf += "    }\n";
  buf += "    function somarMinutosDesliga(valor) {\n";
  buf += "        if (minutosDesliga + valor > 59) {\n";
  buf += "            minutosDesliga = 0\n";
  buf += "        } else if (minutosDesliga + valor < 0) {\n";
  buf += "            minutosDesliga = 59\n";
  buf += "        } else {\n";
  buf += "            minutosDesliga = minutosDesliga + valor\n";
  buf += "        }\n";
  buf += "        atualizarDisplayDesliga()\n";
  buf += "    }\n";
  buf += "    function cancelar() {\n";
  buf += "        window.location.replace('/')\n";
  buf += "    }\n";
  buf += "    function confirmar() {\n";
  buf += "        window.location.replace(`/configurar-horario-liga-e-desliga?horaliga=${horasLiga}&minutoliga=${minutosLiga}&horaDesliga=${horasDesliga}&minutoDesliga=${minutosDesliga}`)\n";
  buf += "    }\n";
  buf += "</script>";
  buf += "</html>";

  return buf;

}

void lerHorariosDaMemoria(){
  horaLiga = lerValorInteiroDaEEPROM(0, 1);
  minutoLiga = lerValorInteiroDaEEPROM(2, 3);
  horaDesliga = lerValorInteiroDaEEPROM(4, 5);
  minutoDesliga = lerValorInteiroDaEEPROM(6, 7);

  if(minutoDesliga <= 0){
    temHorarioConfigurado = false;
  } else {
    temHorarioConfigurado = true;
  }
}

void handleRoot(){
    lerHorariosDaMemoria();
    server.send(200, "text/html", SendHTML()); 
}

void renderizarTelaParaConfigurarRelogio(){
  server.send(200, "text/html", SendHTMLConfigurarRelogio()); 
}

void renderizarTelaParaConfigurarHorarioDeLigaEDesliga(){
  server.send(200, "text/html", SendHTMLConfigurarHorario()); 
}

void configurarRelogio(){
  String horas = server.arg(0);
  String minutos = server.arg(1);

  int horasInt = atoi(horas.c_str());
  int minutosInt = atoi(minutos.c_str());

  rtc.adjust(DateTime(2021, 1, 21, horasInt, minutosInt, 0));

  server.sendHeader("Location", "/",true); 
  server.send(302, "text/plain", "");

}

void configurarHorarioLigaEDesliga(){
  String horaLiga1 = server.arg(0);
  String minutoLiga1 = server.arg(1);
  String horaDesliga1 = server.arg(2);
  String minutoDesliga1 = server.arg(3);


  int horaLigaInt = atoi(horaLiga1.c_str());
  int minutoLigaInt = atoi(minutoLiga1.c_str()); 
  int horaDesligaInt = atoi(horaDesliga1.c_str()); 
  int minutoDesligaInt = atoi(minutoDesliga1.c_str()); 

  escreverValorInteiroNaEEPROM(0, 1, horaLigaInt);
  escreverValorInteiroNaEEPROM(2, 3, minutoLigaInt);
  escreverValorInteiroNaEEPROM(4, 5, horaDesligaInt);
  escreverValorInteiroNaEEPROM(6, 7, minutoDesligaInt);
  
  // lerHorariosDaMemoria();

  server.sendHeader("Location", "/", true); 
  server.send(302, "text/plain", "");
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
    server.sendHeader("Location", "/",true); 
    server.send(302, "text/plain", "");

    // server.send(200, "text/html", SendHTML()); 
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
    server.sendHeader("Location", "/",true); 
    server.send(302, "text/plain", "");

    // server.send(200, "text/html", SendHTML()); 
    botao_liga=0;
    botao_desliga=0;
    digitalWrite(rele, LOW);     
}

void limpa()  {
    Clear_Data();
    novo_horario=0;   
    lido=1;  
    botao_liga=0, botao_desliga=0;
    lerHorariosDaMemoria();
    server.sendHeader("Location", "/",true);
    server.send(302, "text/plain", "");
}

void ler()  {
    // Read_Data();
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

void relogio()  {

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

void salvar()  {
    // Save_Data();
    // novo_horario=0;  
    // Read_Data();  
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


void setup() {
  Serial.begin(115200);
	pinMode(LED1pin, OUTPUT);
	pinMode(LED2pin, OUTPUT);
	pinMode(rele, OUTPUT);  

  initWiFi();

	server.on("/", handleRoot);
	server.on("/configurar-relogio", renderizarTelaParaConfigurarRelogio);
	server.on("/configurar-horario", renderizarTelaParaConfigurarHorarioDeLigaEDesliga);
	server.on("/setar-relogio", configurarRelogio);
	server.on("/configurar-horario-liga-e-desliga", configurarHorarioLigaEDesliga);
	server.on("/liga", liga);
	server.on("/desliga", desliga);
	server.on("/limpa", limpa);

	server.on("/ler", ler);   
	server.on("/novo", novo);     
	// server.on("/inserir", inserir);      
	server.on("/salvar", salvar);  
	server.on("/relogio", relogio);    
	server.on("/salva_relogio", salva_relogio);     
	// server.on("/Save_Data", Save_Data);   
	// server.on("/Read_Data", Read_Data);


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

  EEPROM.begin(EEPROM_SIZE);

  lerHorariosDaMemoria();
}

int cont=0;

void loop(){

	if(cont==0){
		cont++;
		// Read_Data();    
	}
	
	DateTime now = rtc.now();  
	int hora = now.hour();
	int minuto = now.minute();
	String hora_str;
	hora_str=(String(hora)+String(minuto)+"00").c_str(); 
	int hora_atual=atoi(hora_str.c_str());  
	// ligado(hora_atual);
	
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