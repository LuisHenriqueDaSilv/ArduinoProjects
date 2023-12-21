#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
// #include <Wire.h>
#define EEPROM_SIZE 64

RTC_DS3231 rtc;

// const char* ssid = "CXR";  // Enter SSID here
// const char* password = "2021mcdl";  //Enter Password here

const char* ssid     = "sistema_de_irrigacao";
const char* password = "123456789";

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);


bool dispositivoEstaLigado = false;
bool dispositivoFoiLigadoManualmente = false;
bool dispositivoFoiDesativadoManualmente = false;

String horariosParaLigarEDesligar = "";
int quantidadeDeHorariosConfigurados = 0;

bool error = false;
String mensagemdeErro = "";

#define NUM_RELES 6
struct Rele {
  int porta;
  bool esta_ligado;
  int id;
  bool ligadoManualmente;
  bool desligadoManualmente;
};

Rele reles[NUM_RELES];

uint8_t addr_inicial = 8;

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

String numeroComDoisDigitos(int valor){
  if(valor<10){
    return "0"+String(valor);
  } else {
    return String(valor);
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
  buf += "  </head>";
  buf += "  <style>";
  buf += "	* {";
  buf += "		box-sizing: border-box;";
  buf += "		margin: 0;";
  buf += "		padding: 0;";
  buf += "		font-family: sans-serif;";
  buf += "		text-align: center;";
  buf += "		color: #403937;";
  buf += "	}";
  buf += "	:root {";
  buf += "		font-size: 62.5%;";
  buf += "	}";
  buf += "	body {";
  buf += "		width: 100vw;";
  buf += "		height: 100vh;";
  buf += "		background: linear-gradient(107.56deg, #5D5F28 0%, rgba(32, 90, 38, 0.557143) 32.29%, rgba(26, 87, 32, 0.264286) 53.65%, rgba(30, 30, 30, 0) 100%);";
  buf += "		background-color: #1E1E1E;";
  buf += "		font-size: 1.6rem;";
  buf += "		overflow-x: hidden;";
  buf += "	}";
  buf += "	.wrapper {";
  buf += "		display: flex;";
  buf += "		align-items: center;";
  buf += "		justify-content: center;";
  buf += "	}";
  buf += "	.app {";
  buf += "		width: 95vw;";
  buf += "		height: fit-content;";
  buf += "		padding: 4rem;";
  buf += "		background-color: #D9D9D9;";
  buf += "		border-radius: 10px;";
  buf += "		display: flex;";
  buf += "		align-items: center;";
  buf += "		flex-direction: column;";
  buf += "		gap: 4rem;";
  buf += "	}";
  buf += "	a:hover {";
  buf += "		transform: scale(1.05);";
  buf += "		opacity: 0.7;";
  buf += "	}";
  buf += "	.botao-excluir-horario:hover {";
  buf += "		transform: scale(1.05);";
  buf += "		opacity: 0.7;";
  buf += "	}";
  buf += "	a {";
  buf += "		margin-top: 1rem;";
  buf += "		border: none;";
  buf += "		cursor: pointer;";
  buf += "		transition: 200ms;";
  buf += "		font-size: 2rem;";
  buf += "		font-weight: 700;";
  buf += "		border-radius: 10px;";
  buf += "		padding: 0.5rem;";
  buf += "		text-decoration: none;";
  buf += "		background: #C4C4C4;";
  buf += "		box-shadow: 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "		border-radius: 10px;";
  buf += "	}";
  buf += "	.botao-excluir-horario {";
  buf += "		border: none;";
  buf += "		cursor: pointer;";
  buf += "		transition: 200ms;";
  buf += "		font-size: 2.5rem;";
  buf += "		font-weight: 700;";
  buf += "		border-radius: 10px;";
  buf += "		padding: 1rem;";
  buf += "		text-decoration: none;";
  buf += "		background: #B55454;";
  buf += "		box-shadow: 0px 4px 4px rgba(0, 0, 0, 0.25);";
  buf += "		border-radius: 10px;";
  buf += "		height: 4rem;";
  buf += "		width: 4rem;";
  buf += "		justify-content: center;";
  buf += "		align-items: center;";
  buf += "		display: flex;";
  buf += "	}";
  buf += "	.header {";
  buf += "		display: flex;";
  buf += "		flex-direction: column;";
  buf += "		justify-content: space-between;";
  buf += "		max-width: 97.4rem;";
  buf += "		width: 100%;";
  buf += "		gap: 1rem;";
  buf += "	}";
  buf += "	.header p {";
  buf += "		color: #403937;";
  buf += "		font-weight: 600;";
  buf += "		font-size: 1.2rem;";
  buf += "	}";
  buf += "	.containerBotoesManual {";
  buf += "		display: grid;";
  buf += "		grid-template-columns: 1fr 1fr 1fr;";
  buf += "	}";
  buf += "	.containerBotoesManual .button {";
  buf += "		border: 2px solid #ffffff;";
  buf += "		background-color: ;";
  buf += "		padding: 0.5rem;";
	buf += "    margin: 0;";
	buf += "    border-radius: 0;";
	buf += "    color: #ffffff;";
  buf += "	}";
  buf += "  .porta-ligada {";
  buf += "    background-color: #8AC87F";
  buf += "  }";
  buf += "  .porta-desligada {";
  buf += "    background-color: #A64D4D";
  buf += "}";

  buf += "	.containerBotoesManual .button:nth-child(1) {";
  buf += "		border-top-left-radius: 12px;";
  buf += "	}";
  buf += "	.containerBotoesManual .button:nth-child(3) {";
  buf += "		border-top-right-radius: 12px;";
  buf += "	}";
  buf += "	.containerBotoesManual .button:last-child {";
  buf += "		border-bottom-right-radius: 12px;";
  buf += "	}";
  buf += "	.containerBotoesManual .button p {";
  buf += "		font-size: 1.5rem;";
  buf += "		color: #ffffff;";
  buf += "		font-weight: 600;";
  buf += "	}";
  buf += "	.containerBotoesManual .button h1 {";
  buf += "		font-size: 4rem;";
  buf += "		color: #ffffff;";
  buf += "	}";
  buf += "	.containerBotoesManual .button label {";
  buf += "		color: #ffffff;";
  buf += "		font-size: 1.6rem;";
  buf += "	}";
  buf += "	#botaodesligado {";
  buf += "		background-color: #8AC880;";
  buf += "		color: #ffffff;";
  buf += "	}";
  buf += "	.horario-relogio {";
  buf += "		display: flex;";
  buf += "		justify-content: center;";
  buf += "		gap: 0.5rem;";
  buf += "		flex-direction: row;";
  buf += "		margin-top: 1rem;";
  buf += "	}";
  buf += "	.horario-relogio div {";
  buf += "		display: flex;";
  buf += "		justify-content: center;";
  buf += "		align-items: center;";
  buf += "		background-color: #CDCDCD;";
  buf += "		height: 5rem;";
  buf += "		width: 5rem;";
  buf += "		border-radius: 10px;";
  buf += "		font-size: 2rem;";
  buf += "		font-weight: 500;";
  buf += "		margin-bottom: 2rem;";
  buf += "	}";
  buf += "	.separador-de-hora {";
  buf += "		font-size: 2rem;";
  buf += "		align-self: center;";
  buf += "	}";
  buf += "	.footer-horarios {";
  buf += "		display: flex;";
  buf += "		flex-direction: column;";
  buf += "		align-items: center;";
  buf += "		gap: 1rem;";
  buf += "	}";
  buf += "	.footer-horarios a {";
  buf += "		width: 28rem;";
  buf += "	}";

  buf += "	@media (min-width: 952px) {";
  buf += "		.containerBotoesManual {";
  buf += "			grid-template-columns: 1fr 1fr 1fr 1fr 1fr 1fr;";
  buf += "		}";
  buf += "		.containerBotoesManual button:nth-child(3) {";
  buf += "			border-top-right-radius: 0px;";
  buf += "		}";
  buf += "		.containerBotoesManual button:nth-child(6) {";
  buf += "			border-top-right-radius: 12px;";
  buf += "		}";
  buf += "	}";

  buf += "	.container-de-horarios {";
  buf += "		gap: 1rem;";
  buf += "		display: flex;";
  buf += "		flex-direction: column;";
  buf += "		align-items: center;";
  buf += "		width: 100%;";
  buf += "		margin-top: 1.5rem;";
  buf += "		margin-bottom: 3rem;";
  buf += "	}";
  buf += "	.horario {";
  buf += "		display: flex;";
  buf += "		align-items: center;";
  buf += "		gap: 2rem;";
  buf += "	}";
  buf += "	.horario input[type=checkbox]{";
  buf += "		border: 0px;";
  buf += "		width: 3rem;";
  buf += "    height: 3rem;";
  buf += "    vertical-align: middle;";
  buf += "	}";
  buf += "	.horario h1 {";
  buf += "		font-size: 2rem;";
  buf += "		align-self: center;";
  buf += "	}";
  buf += "	.horario p {";
  buf += "		font-size: 2rem;";
  buf += "	}";
  buf += "	.horario p label {";
  buf += "		font-weight: bolder;";
  buf += "	}";
  buf += "	.identificador-de-porta-horario { ";
  buf += "		display: flex;";
  buf += "		flex-direction: column;";
  buf += "		justify-content: center;";
  buf += "		align-items: center;";
  buf += "		border-radius: 10px;";
  buf += "		width: 4rem;";
  buf += "		height: 4rem;";
  buf += "  }";
  buf += "	.identificador-de-porta-horario p {";
  buf += "		color: #ffffff;";
  buf += "		font-size: 2rem;";
  buf += "	}";
  buf += "	.identificador-de-porta-horario h1 {";
  buf += "		color: #ffffff;";
  buf += "		font-size: 3rem;";
  buf += "	}";
  buf += "h1 {";
	buf += "	font-size: 2.1rem;";
	buf += "}";
  buf += "	@media (max-width: 952px) {";
  buf += "		.app {";
  buf += "			width: 100vw;";
  buf += "		}";
  buf += "	}";

  buf += "  </style>";
  buf += "  <body>";
  buf += "      <div class='wrapper'>";
  buf += "          <div class='app'>";
	buf += "        		<header class='header'>";
	buf += "        			<div class='containerBotoesManual'>";
  
  for (int i = 0; i < NUM_RELES; i++) {
    buf += "        				<a  class='button ";
    buf += reles[i].esta_ligado? "porta-ligada": "porta-desligada";
    buf += "'  href=";
    if(reles[i].esta_ligado){
      buf += "'/desligar-manualmente?id=";
      buf += i;
      buf += "'";
    } else {
      buf += "'/ligar-manualmente?id=";
      buf += i;
      buf += "'";
    }
    buf+= "id='porta-"; 
    buf += i;
    buf += "'";
    buf+= ">";
    buf += "        					<p>porta</p>";
    buf += "        					<h1>";
    buf += reles[i].id;
    buf += "</h1>";
    buf += "        					<label>";
    buf += reles[i].esta_ligado? "ligado": "desligado";
    buf += "</label>";
    buf += "        				</a>";
  }

	buf += "        			</div>";
	buf += "        			<p>toque em uma das portas para alterar seus estados manualmente(Alterando manualmente, a porta";
	buf += "        				permanecera naquele estado até que uma das configurações altere ou seja alterado manualmente";
	buf += "        				novamente)</p>";
	buf += "		        </header>";

  buf += "                <div>";
  if(quantidadeDeHorariosConfigurados >= 1){
    buf += "                <div>";
    buf += "                    <h1>O sistema esta configurado para ficar ligado nos seguintes horarios:</h1>";
    buf += "                    <div class='container-de-horarios'>";

    for(int i = 0; i < quantidadeDeHorariosConfigurados; i = i + 1){

      int horaLiga = atoi(horariosParaLigarEDesligar.substring(i*12, i*12+2).c_str());
      int minutoLiga = atoi(horariosParaLigarEDesligar.substring(i*12+2, i*12+4).c_str());
      int horaDesliga = atoi(horariosParaLigarEDesligar.substring(i*12+4, i*12+6).c_str());
      int minutoDesliga = atoi(horariosParaLigarEDesligar.substring(i*12+6, i*12+8).c_str());
      int indexRele = atoi(horariosParaLigarEDesligar.substring(i*12+9, i*12+10).c_str());
      int horarioAtivo = atoi(horariosParaLigarEDesligar.substring(i*12+11, i*12+12).c_str());

			buf += "		<div class='horario'>";
      buf += "    <input onchange='ativaDesativaHorario(\"";
      buf += numeroComDoisDigitos(horaLiga) + numeroComDoisDigitos(minutoLiga);
      buf += "-";
      buf += numeroComDoisDigitos(horaDesliga) + numeroComDoisDigitos(minutoDesliga);
      buf += "\")' type='checkbox' ";
      buf += horarioAtivo? "checked":"";
      buf +=" />";
      
			buf += "			<div class='identificador-de-porta-horario ";
      buf += reles[indexRele].esta_ligado && !reles[indexRele].ligadoManualmente? "porta-ligada": "porta-desligada";
      buf+= "'>";
			buf += "				<h1>";
      buf += reles[indexRele].id;
      buf += "</h1>";
			buf += "			</div>";
			buf += "			<p>";
      buf += numeroComDoisDigitos(horaLiga);
      buf += ":";
      buf += numeroComDoisDigitos(minutoLiga);
      buf += "<label> até </label>"; 
      buf+= numeroComDoisDigitos(horaDesliga);
      buf += ":"; 
      buf += numeroComDoisDigitos(minutoDesliga);
      buf += "</p>";
      buf += "  <button onclick='excluirHorario(\"";
      buf += numeroComDoisDigitos(horaLiga) + numeroComDoisDigitos(minutoLiga);
      buf += "-";
      buf += numeroComDoisDigitos(horaDesliga) + numeroComDoisDigitos(minutoDesliga);
      buf += "\")' class='botao-excluir-horario'>";
      buf += "🗑</button";
			buf += "		</div>";
			buf += "		</div>";
    }

    buf += "                    </div>";
    buf += "                </div>";
  } else {
    buf += "<h1>Você não configurou um horario para <span id='ligar'>ligar</span> ou <span id='desligar'>desligar</span></h1>";
  }
  buf += "                    <div class='footer-horarios'>";
  buf += "                        <a href='/configurar-horario'>adicionar horario</a>";
  if(quantidadeDeHorariosConfigurados >= 1){
    buf += "                        <a href='/limpar-horarios'>apagar horarios</a>";
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
  buf += "                    <div class='horario-relogio'>";
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
  if(error){
    error = false;
    buf += "alert('";
    buf += mensagemdeErro;
    buf += "')\n";
  }
	buf += "function ativaDesativaHorario(horario){\n";
	buf += "	const [horarioLiga, horarioDesliga] = horario.split('-')\n";
	buf += "	const horarioLigaSplitado = horarioLiga.split('')\n";
	buf += "	const horarioDesligaSplitado = horarioDesliga.split('')\n";
	buf += "	const usuarioConfirmouAExclusao = confirm(`deseja mesmo desativar o horario de ${horarioLigaSplitado[0]}${horarioLigaSplitado[1]}:${horarioLigaSplitado[2]}${horarioLigaSplitado[3]} ate ${horarioDesligaSplitado[0]}${horarioDesligaSplitado[1]}:${horarioDesligaSplitado[2]}${horarioDesligaSplitado[3]}?`)\n";
	buf += "	if(usuarioConfirmouAExclusao){\n";
	buf += "		const horarioFormatadoParaEnvio = `${horarioLigaSplitado[0]}${horarioLigaSplitado[1]}${horarioLigaSplitado[2]}${horarioLigaSplitado[3]}${horarioDesligaSplitado[0]}${horarioDesligaSplitado[1]}${horarioDesligaSplitado[2]}${horarioDesligaSplitado[3]}`\n";
	buf += "		window.location.replace(`/alterar-estado-horario?horario=${horarioFormatadoParaEnvio}`)\n";
	buf += "	}\n";
	buf += "}\n";

  buf += "    setInterval(async() => {\n";
  buf += "      const respostaHoraAtual = await fetch('/relogio')\n";
  buf += "      const respostaStatus = await fetch('/status')\n";
  buf += "      const datahoraAtual = await respostaHoraAtual.json()\n";
  buf += "      const dataStatus = await respostaStatus.json()\n";

  buf += "      const visorHora = window.document.getElementById('hora-atual')\n";
  buf += "      const visorMinuto = window.document.getElementById('minuto-atual')\n";

  buf += "      dataStatus.reles.map((rele) => {\n";
  buf += "        const botao = window.document.getElementById(`porta-${rele.index}`)\n";
  buf += "        const filho = botao.querySelector('label')\n";

  buf += "      if(rele.ligado == 1){\n";
  buf += "          filho.innerHTML = 'ligado'\n";
  buf += "          botao.classList.remove('porta-desligada')\n";
  buf += "          botao.classList.add('porta-ligada')\n";
  buf += "          botao.href = `/desligar-manualmente?id=${rele.index}`\n";
  buf += "      } else {\n";
  buf += "          filho.innerHTML = 'desligado'\n";
  buf += "          botao.classList.add('porta-desligada')\n";
  buf += "          botao.classList.remove('porta-ligada')\n";
  buf += "          botao.href = `/ligar-manualmente?id=${rele.index}`\n";
  buf += "      }\n";
  buf += "    })\n";

  buf += "      visorHora.innerHTML = datahoraAtual.hora\n";
  buf += "      visorMinuto.innerHTML = datahoraAtual.minuto\n";
  buf += "    }, 5000)\n";

  buf += "      function excluirHorario(horario){\n";
  buf += "          const [horarioLiga, horarioDesliga] = horario.split('-')\n";
  buf += "          const horarioLigaSplitado = horarioLiga.split('')\n";
  buf += "          const horarioDesligaSplitado = horarioDesliga.split('')\n";
  buf += "          const usuarioConfirmouAExclusao = confirm(`deseja mesmo excluir o horario de ${horarioLigaSplitado[0]}${horarioLigaSplitado[1]}:${horarioLigaSplitado[2]}${horarioLigaSplitado[3]} ate ${horarioDesligaSplitado[0]}${horarioDesligaSplitado[1]}:${horarioDesligaSplitado[2]}${horarioDesligaSplitado[3]}?`)\n";
  buf += "          if(usuarioConfirmouAExclusao){\n";
  buf += "              const horarioFormatadoParaEnvio = `${horarioLigaSplitado[0]}${horarioLigaSplitado[1]}${horarioLigaSplitado[2]}${horarioLigaSplitado[3]}${horarioDesligaSplitado[0]}${horarioDesligaSplitado[1]}${horarioDesligaSplitado[2]}${horarioDesligaSplitado[3]}`\n";
  buf += "              window.location.replace(`/excluir-horario?horario=${horarioFormatadoParaEnvio}`)\n";
  buf += "          }\n";
  buf += "      }\n";
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
  buf += "</head>";
  buf += "<style>";
  buf += "    * {";
  buf += "        box-sizing: border-box;";
  buf += "        margin: 0;";
  buf += "        padding: 0;";
  buf += "        font-family: sans-serif;";
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
  buf += "        align-items: center;";
  buf += "        justify-content: center;";
  buf += "    }";
  buf += "    .app {";
  buf += "        width: 90vw;";
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
  buf += "    @media (max-width: 952px) {";
  buf += "        .app {";
  buf += "            width: 100vw;";
  buf += "        }";
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
  if(error){
    error = false;
    buf += "alert('";
    buf += mensagemdeErro;
    buf += "')\n";
  }
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
  buf += "        align-items: center;";
  buf += "        justify-content: center;";
  buf += "    }";
  buf += "    .app {";
  buf += "        width: 90vw;";
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
  buf += "        width: 6rem;";
  buf += "        height: 6rem;";
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
  buf += "    @media (max-width: 952px) {";
  buf += "        .app {";
  buf += "            width: 100vw;";
  buf += "        }";
  buf += "    }";
  buf += "    .portas-container {";
  buf += "        display: grid;";
  buf += "        grid-template-columns: 1fr 1fr 1fr 1fr 1fr;";
  buf += "    }";
  buf += "    .porta {";
  buf += "        padding: 1rem;";
  buf += "        border: 2px solid #ffffff;";
  buf += "        border-radius: 0;";
  buf += "        height: auto;";
  buf += "        width: auto;";
  buf += "    }";
  buf += "    p {";
  buf += "        font-weight: bolder;";
  buf += "    }";
  buf += "    .porta h1 {";
  buf += "        font-size: 4rem;";
  buf += "        color: #ffffff;";
  buf += "    }";
  buf += "    .porta p {";
  buf += "        font-size: 2rem;";
  buf += "        color: #ffffff;";
  buf += "        ";
  buf += "    }";
  buf += "	.portas-container .porta:nth-child(1) {";
  buf += "		border-top-left-radius: 12px;";
  buf += "	}";
  buf += "	.portas-container .porta:nth-child(5) {";
  buf += "		border-top-right-radius: 12px;";
  buf += "	}";
  buf += "	.portas-container .porta:last-child {";
  buf += "		border-bottom-right-radius: 12px;";
  buf += "	}";
  buf += "  #porta-selecionada {";
  buf += "    background-color: #4D4698;";
  buf += "  }";  
  buf += "</style>";
  buf += "<body>";
  buf += "    <div class='wrapper'>";
  buf += "        <div class='app'>";
  buf += "            <h1>configurar horario</h1>";
  buf += "            <div>";
  buf += "                <p>selecione a porta desejada para a configuração</p>";
  buf += "                <div class='portas-container'>";
  
  for (int i = 0; i < NUM_RELES; i++) {
    buf += "                    <button class='porta porta-";
    buf += i;
    buf+= "' onclick='selecionarporta(";
    buf += i; 
    buf+= ")'>";
    buf += "                        <p>porta</p>";
    buf += "                        <h1>"; 
    buf += reles[i].id;
    buf += "</h1>";
    buf += "                    </button>";
  }
  buf += "                </div>";
  buf += "            </div>";
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
  if(error){
    error = false;
    buf += "alert('";
    buf += mensagemdeErro;
    buf += "')\n";
  }
  buf += "    let portaselecionada0\n";
  buf += "    function selecionarporta(posicao){\n";
  buf += "        const selecionadoAnterior = document.getElementById('porta-selecionada');\n";
  buf += "        if(selecionadoAnterior)selecionadoAnterior.id = '';\n";
  buf += "        const elemento = document.getElementsByClassName(`porta-${posicao}`)[0]\n";
  buf += "        elemento.id = 'porta-selecionada'\n";
  buf += "        portaselecionada = posicao\n";
  buf += "    }\n";
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
  buf += "        window.location.replace(`/adicionar-horario?horaliga=${horasLiga}&minutoliga=${minutosLiga}&horaDesliga=${horasDesliga}&minutoDesliga=${minutosDesliga}&porta=${portaselecionada}`)\n";
  buf += "    }\n";
  buf += "</script>";
  buf += "</html>";

  return buf;

}
void lerHorariosDaMemoria(){

  horariosParaLigarEDesligar = "";
  quantidadeDeHorariosConfigurados = 0;

  int contadorDeEnderecoDaMemoria = 0;

  bool chegouAoFinalDaMemoria = false;

  while (
    !chegouAoFinalDaMemoria
  ){

    int ultimaHoraParaLigar = lerValorInteiroDaEEPROM(contadorDeEnderecoDaMemoria, contadorDeEnderecoDaMemoria+1);
    int ultimoMinutoParaLigar = lerValorInteiroDaEEPROM(contadorDeEnderecoDaMemoria+2, contadorDeEnderecoDaMemoria+3);
    int ultimaHoraParaDesligar = lerValorInteiroDaEEPROM(contadorDeEnderecoDaMemoria+4, contadorDeEnderecoDaMemoria+5);
    int ultimaMinutoParaDesligar = lerValorInteiroDaEEPROM(contadorDeEnderecoDaMemoria+6, contadorDeEnderecoDaMemoria+7);
    int indexPorta = lerValorInteiroDaEEPROM(contadorDeEnderecoDaMemoria+8, contadorDeEnderecoDaMemoria+9);
    int ativo = lerValorInteiroDaEEPROM(contadorDeEnderecoDaMemoria+10, contadorDeEnderecoDaMemoria+11);

    if(
      ultimaHoraParaLigar == 0 &&
      ultimoMinutoParaLigar == 0 &&
      ultimaHoraParaDesligar == 0 &&
      ultimaMinutoParaDesligar == 0 &&
      indexPorta == 0 && 
      ativo == 0
    ) {
      chegouAoFinalDaMemoria = true;
    } else {
      horariosParaLigarEDesligar += (
        numeroComDoisDigitos(ultimaHoraParaLigar)+
        numeroComDoisDigitos(ultimoMinutoParaLigar) + 
        numeroComDoisDigitos(ultimaHoraParaDesligar) +
        numeroComDoisDigitos(ultimaMinutoParaDesligar) +
        numeroComDoisDigitos(indexPorta) + 
        numeroComDoisDigitos(ativo)
      );
      quantidadeDeHorariosConfigurados += 1;
      contadorDeEnderecoDaMemoria = contadorDeEnderecoDaMemoria + 12;
    }
  }

}

void initWiFi() {
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(ssid, password);
    // Serial.print("Connecting to WiFi ..");
    // while (WiFi.status() != WL_CONNECTED){
		// Serial.print('.');
		// delay(1000);
    // }
    // Serial.println(WiFi.localIP());

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

}
void LimparEEPROM() {

  EEPROM.begin(512);  

  Serial.println("Limpando EEPROM!");
  for (int i = 0; i <= 255; i++) {
    EEPROM.write(i, 0);
  }
  Serial.println("EEPROM apagada!");
  EEPROM.write(addr_inicial, (byte) 0);
  EEPROM.commit();
  EEPROM.end();
}
void ligarDispositivo(int index){
  
  dispositivoEstaLigado = true;
  reles[index].esta_ligado = true;
  digitalWrite(reles[index].porta, HIGH);     
}
void desligarDispositivo(int index){
  dispositivoEstaLigado = false;
  reles[index].esta_ligado = false;
  digitalWrite(reles[index].porta, LOW);
}

bool verificarSeDeveEstarLigado(
  int minutoDoDiaAtual, 
  int minutoDoDiaParaLigarSistema,
  int minutoDoDiaParaDesligarSistema,
  int indexRele
){

    if(minutoDoDiaAtual == minutoDoDiaParaDesligarSistema || minutoDoDiaAtual == minutoDoDiaParaLigarSistema){
      reles[indexRele].ligadoManualmente = false;
      reles[indexRele].desligadoManualmente = false;
    }

    if(reles[indexRele].ligadoManualmente){
      return true;
    }

    if(reles[indexRele].desligadoManualmente){
      return false;
    }

    if(minutoDoDiaParaDesligarSistema > minutoDoDiaParaLigarSistema){
      if(minutoDoDiaAtual >= minutoDoDiaParaLigarSistema && minutoDoDiaAtual < minutoDoDiaParaDesligarSistema){
        return true;
      }else {
        return false;
      }
    }else if(minutoDoDiaParaDesligarSistema < minutoDoDiaParaLigarSistema) {
      int tempoQueODispositivoVaiFicarLigado = (minutoDoDiaParaDesligarSistema + 1440) - minutoDoDiaParaLigarSistema;
      int minutosLigadoNoPrimeiroDia = 1440 - minutoDoDiaParaLigarSistema; // 1440 é a quantidade de minutos em 24 horas
      int minutosLigadoNoSegundoDia =  tempoQueODispositivoVaiFicarLigado - minutosLigadoNoPrimeiroDia;


      if(minutoDoDiaAtual >= minutoDoDiaParaLigarSistema || minutoDoDiaAtual < minutosLigadoNoSegundoDia){
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
} 


void carregarRaiz(){
  lerHorariosDaMemoria();
  server.send(200, "text/html", SendHTML()); 
}
void carregarTelaParaConfigurarRelogio(){
  server.send(200, "text/html", SendHTMLConfigurarRelogio()); 
}
void carregarTelaParaConfigurarHorarioDeLigaEDesliga(){
  server.send(200, "text/html", SendHTMLConfigurarHorario()); 
}
void handleConfigurarRelogio(){
  String horas = server.arg(0);
  String minutos = server.arg(1);

  int horasInt = atoi(horas.c_str());
  int minutosInt = atoi(minutos.c_str());

  rtc.adjust(DateTime(2021, 1, 21, horasInt, minutosInt, 0));

  server.sendHeader("Location", "/",true); 
  server.send(302, "text/plain", "");

}
void handleAdicionarHorarioLigaEDesliga(){
  String horaLiga1 = server.arg(0);
  String minutoLiga1 = server.arg(1);
  String horaDesliga1 = server.arg(2);
  String minutoDesliga1 = server.arg(3);
  String indexRele = server.arg(4);

  int horaLigaInt = atoi(horaLiga1.c_str());
  int minutoLigaInt = atoi(minutoLiga1.c_str()); 
  int horaDesligaInt = atoi(horaDesliga1.c_str()); 
  int minutoDesligaInt = atoi(minutoDesliga1.c_str()); 
  int indexReleInt = atoi(indexRele.c_str());

  escreverValorInteiroNaEEPROM(quantidadeDeHorariosConfigurados*12, quantidadeDeHorariosConfigurados*12 +1, horaLigaInt);
  escreverValorInteiroNaEEPROM(quantidadeDeHorariosConfigurados*12 +2, quantidadeDeHorariosConfigurados*12 +3, minutoLigaInt);
  escreverValorInteiroNaEEPROM(quantidadeDeHorariosConfigurados*12 +4, quantidadeDeHorariosConfigurados*12 +5, horaDesligaInt);
  escreverValorInteiroNaEEPROM(quantidadeDeHorariosConfigurados*12 +6, quantidadeDeHorariosConfigurados*12 +7, minutoDesligaInt);
  escreverValorInteiroNaEEPROM(quantidadeDeHorariosConfigurados*12 +8, quantidadeDeHorariosConfigurados*12 +9, indexReleInt);
  escreverValorInteiroNaEEPROM(quantidadeDeHorariosConfigurados*12 +10, quantidadeDeHorariosConfigurados*12 +11, 1); //horario ativo
  server.sendHeader("Location", "/", true); 
  server.send(302, "text/plain", "");
  
}
void handleLigarDispositivoManualmente(){
  
  String indexRele = server.arg(0);
  int indexReleInt = atoi(indexRele.c_str());

  ligarDispositivo(indexReleInt);
  reles[indexReleInt].ligadoManualmente = true;
  reles[indexReleInt].desligadoManualmente = false;
  server.sendHeader("Location", "/",true); 
  server.send(302, "text/plain", "");
}
void handleDesligarDispositivoManualmente()  {

  String indexRele = server.arg(0);
  int indexReleInt = atoi(indexRele.c_str());

  desligarDispositivo(indexReleInt);
  reles[indexReleInt].ligadoManualmente = false;
  reles[indexReleInt].desligadoManualmente = true;
  server.sendHeader("Location", "/",true); 
  server.send(302, "text/plain", "");
}
void handleLimparHorarios()  {
    LimparEEPROM();
    lerHorariosDaMemoria();

    server.sendHeader("Location", "/",true);
    server.send(302, "text/plain", "");
}
void handleConsultarRelogio(){

    char horas[10] = "hh";
    char minutos[10] = "mm";
    rtc.now().toString(horas);   
    rtc.now().toString(minutos);   

    String buf = "{ \"hora\" : \""+ String(horas) + "\", \"minuto\" : \""+String(minutos)+"\" }";
    server.send(200, "text/json", buf); 

} 
void handleConsultarStatus(){

    String buf = "  {";
    buf += "    \"reles\": [";
    for (int i = 0; i < NUM_RELES; i++) {      
      buf += "        {";
      buf += "            \"id\":";
      buf +=  reles[i].id;
      buf +=",";
      buf += "            \"ligado\":";
      buf += String(reles[i].esta_ligado);
      buf += ",";
      buf += "            \"index\":";
      buf += i;
      buf += "        }";

      if(i != NUM_RELES -1){
        buf+= ",";
      }

    }
    buf += "    ]";
    buf += "  }";

    server.send(200, "text/json", buf); 
}
void handleExcluirHorario(){
  String horarioParaExclusao = server.arg(0);

  int posicaoDoHorarioParaExclusao = horariosParaLigarEDesligar.indexOf(horarioParaExclusao);
  horariosParaLigarEDesligar.remove(posicaoDoHorarioParaExclusao, 12); 
  int tamanhoDaString = horariosParaLigarEDesligar.length();

  LimparEEPROM();

  for(int i = 0; i < tamanhoDaString; i = i + 1){

    int valorInt = atoi(horariosParaLigarEDesligar.substring(i,i+1).c_str());

    EEPROM.begin(512);
    EEPROM.write(i, valorInt);
    EEPROM.end();

  }

  lerHorariosDaMemoria();

  server.sendHeader("Location", "/",true); 
  server.send(302, "text/plain", "");
}

void handleAlterarEstadoHorario(){
  String horarioParaExclusao = server.arg(0);

  int posicaoDoHorarioParaAlteracao = horariosParaLigarEDesligar.indexOf(horarioParaExclusao);
  horariosParaLigarEDesligar[posicaoDoHorarioParaAlteracao+11] = horariosParaLigarEDesligar[posicaoDoHorarioParaAlteracao+11] == '1'? '0':'1';

  LimparEEPROM();
  int tamanhoDaString = horariosParaLigarEDesligar.length();

  for(int i = 0; i < tamanhoDaString; i = i + 1){

    int valorInt = atoi(horariosParaLigarEDesligar.substring(i,i+1).c_str());

    EEPROM.begin(512);
    EEPROM.write(i, valorInt);
    EEPROM.end();

  }


  server.sendHeader("Location", "/",true); 
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);

           //Porta-estaligado-identificador-ligadoManualmente-DesligadoManualmente
  reles[0] = {5, false, 1, false, false};
  reles[1] = {4, false, 2, false, false};
  reles[2] = {18, false, 3, false, false};
  reles[3] = {19, false, 4, false, false};
  reles[4] = {23, false, 5, false, false};
  reles[5] = {15, false, 6, false, false};

  for (int i = 0; i < NUM_RELES; i++) {
    pinMode(reles[i].porta, OUTPUT);
  }
  delay(1000);
  for (int i = 0; i < NUM_RELES; i++) {
    desligarDispositivo(i);
  }

  initWiFi();

	server.on("/", carregarRaiz);
	server.on("/configurar-relogio", carregarTelaParaConfigurarRelogio);
	server.on("/configurar-horario", carregarTelaParaConfigurarHorarioDeLigaEDesliga);
	server.on("/setar-relogio", handleConfigurarRelogio);
	server.on("/adicionar-horario", handleAdicionarHorarioLigaEDesliga);
	server.on("/ligar-manualmente", handleLigarDispositivoManualmente);
	server.on("/desligar-manualmente", handleDesligarDispositivoManualmente);
	server.on("/limpar-horarios", handleLimparHorarios);
	server.on("/relogio", handleConsultarRelogio);    
	server.on("/status", handleConsultarStatus);   
	server.on("/excluir-horario", handleExcluirHorario);   
	server.on("/alterar-estado-horario", handleAlterarEstadoHorario);    


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

int ultimaVerificacaoLigados = 0;

void loop(){

	DateTime now = rtc.now();
	int hora = now.hour();
	int minuto = now.minute();

  int minutoDoDiaAtual = (hora*60) + minuto;
  bool dispositivoDeveEstarLigado = false;
  // 2000 - 
  if(millis() - ultimaVerificacaoLigados > 1000 ){
    ultimaVerificacaoLigados = millis();

    if(quantidadeDeHorariosConfigurados > 0){
      for(int i = 0; i< quantidadeDeHorariosConfigurados; i=i+1){

        int horaLiga = atoi(horariosParaLigarEDesligar.substring(i*12, i*12+2).c_str());
        int minutoLiga = atoi(horariosParaLigarEDesligar.substring(i*12+2, i*12+4).c_str());
        int horaDesliga = atoi(horariosParaLigarEDesligar.substring(i*12+4, i*12+6).c_str());
        int minutoDesliga = atoi(horariosParaLigarEDesligar.substring(i*12+6, i*12+8).c_str());
        int indexRele = atoi(horariosParaLigarEDesligar.substring(i*12+8, i*12+10).c_str());
        int horarioativo = atoi(horariosParaLigarEDesligar.substring(i*12+10, i*12+12).c_str());

        if(horarioativo == 1){
          int minutoDoDiaParaLigarSistema = (horaLiga*60) + minutoLiga;
          int minutoDoDiaParaDesligarSistema = (horaDesliga*60) + minutoDesliga;

          bool deveEstarLigado = verificarSeDeveEstarLigado(minutoDoDiaAtual, minutoDoDiaParaLigarSistema, minutoDoDiaParaDesligarSistema, indexRele);

          if(deveEstarLigado && !reles[indexRele].esta_ligado){
            ligarDispositivo(indexRele);
          } else if(!deveEstarLigado && reles[indexRele].esta_ligado){
            desligarDispositivo(indexRele);
          }
        }


      }
    }

  }
	
	server.handleClient();
}