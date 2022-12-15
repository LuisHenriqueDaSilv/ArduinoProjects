#include <Arduino.h>
#include <Ultrasonic.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>


/*
------Portas------
*/
#define MOTOR_DIREITO_HORARIO 7
#define MOTOR_DIREITO_ANTIHORARIO 6
#define MOTOR_ESQUERDO_HORARIO 4
#define MOTOR_ESQUERDO_ANTIHORARIO 5

#define SENSOR_ENCODER 2
#define PINO_BOTAO_DE_CONTROLE 3

#define TRIG_ULTRASSONICO_FRONTAL A0
#define ECHO_ULTRASSONICO_FRONTAL A1

#define TRIG_ULTRASSONICO_DIREITO 11
#define ECHO_ULTRASSONICO_DIREITO 10

#define TRIG_ULTRASSONICO_TRASEIRO A3
#define ECHO_ULTRASSONICO_TRASEIRO A2

/*
Portas LCD:
	SDA: A4 
	SCL: A5
*/


/*
------Constantes globais------
*/
#define DISTANCIA_MINIMA_PAREDE 10
#define CMS_POR_PULSO 0.44


/*
------Função para controle dos motores------
*/

void 
controlarMotores(int direcao, bool girando=false){

	// Desligando todos os motores:
	digitalWrite(MOTOR_DIREITO_ANTIHORARIO, 0);
	digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, 0);
	digitalWrite(MOTOR_DIREITO_HORARIO, 0);
	digitalWrite(MOTOR_ESQUERDO_HORARIO, 0);


	//Controle
	if(girando){

		if(direcao == 1){
			digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, HIGH);
			digitalWrite(MOTOR_DIREITO_HORARIO, HIGH);
		} else if (direcao == -1){
			digitalWrite(MOTOR_ESQUERDO_HORARIO, HIGH);
			digitalWrite(MOTOR_DIREITO_ANTIHORARIO, HIGH);
		}

	} else {

		if(direcao == 1){
			digitalWrite(MOTOR_DIREITO_HORARIO, HIGH);
			digitalWrite(MOTOR_ESQUERDO_HORARIO, HIGH);
		} else if(direcao == -1) {
			digitalWrite(MOTOR_DIREITO_ANTIHORARIO, HIGH);
			digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, HIGH);
		}
		
	}

}

void
frear(int direcaoAtual=1){
  controlarMotores(direcaoAtual*-1);
  delay(100);
  controlarMotores(0);
}


/*
------Display LCD------
*/
LiquidCrystal_I2C displayLCD(0x27,20,4);

String primeiraLinhaLCD = "";
String segundaLinhaLCD = "";

void 
escreverLCD(String primeiraLinha	, String segundaLinha = ""){

	if(!primeiraLinha.equalsIgnoreCase(primeiraLinhaLCD)){
		displayLCD.setCursor(0,0);
		displayLCD.print("                ");
		displayLCD.setCursor(0,0);
		displayLCD.print(primeiraLinha);
		primeiraLinhaLCD = primeiraLinha;
	}

	if(!segundaLinha.equalsIgnoreCase(segundaLinhaLCD)){
		displayLCD.setCursor(0,1);
		displayLCD.print("                ");
		displayLCD.setCursor(0,1);
		displayLCD.print(segundaLinha);
		segundaLinhaLCD = segundaLinha;
	}

}


/*
------Controle de funções------
*/

String funcao = "desativado";
String ultimaFuncaoLoop;
volatile long ultimaTrocaDeFuncao = 0;
long inicioDelayDeTrocaDeModo = 0;

void 
trocarFuncao(){

	if(millis() - ultimaTrocaDeFuncao > 500){
		if(funcao == "desativado"){
			funcao = "medicao livre";
		} else if(funcao == "medicao livre"){
			funcao = "medicao completa";
		} else if (funcao == "medicao completa"){
			funcao = "desativado";
		}
		ultimaTrocaDeFuncao = millis();
		inicioDelayDeTrocaDeModo = millis();
	}


}


/*
------Sensores de Roda Encoder------
*/

volatile long pulsos;
volatile long ultimoPulso = 0;
volatile bool medindo = false;

void
contarPulsos(){
	ultimoPulso = millis();
	pulsos++;
}

void
ligarMedicaoDeRoda(){
	attachInterrupt(digitalPinToInterrupt(SENSOR_ENCODER), contarPulsos, CHANGE);
	medindo = true;
}

void
desligarMedicaoDeRoda(){
	detachInterrupt(digitalPinToInterrupt(SENSOR_ENCODER));
	medindo = false;
}


/*
------Sensores de Distancia ultrassonicos------
*/
Ultrasonic ultrassonicoFrontal(TRIG_ULTRASSONICO_FRONTAL, ECHO_ULTRASSONICO_FRONTAL); //Trig-Echo
Ultrasonic ultrassonicoDireito(TRIG_ULTRASSONICO_DIREITO, ECHO_ULTRASSONICO_DIREITO); //Trig-Echo
Ultrasonic ultrassonicoTraseiro(TRIG_ULTRASSONICO_TRASEIRO, ECHO_ULTRASSONICO_TRASEIRO); //Trig-Echo


/*
------Variaveis globais------
*/
int etapa = 1;

void
reiniciarEstado(){
	etapa = 1;
	pulsos = 0;

	ligarMedicaoDeRoda();
	controlarMotores(0);
	desligarMedicaoDeRoda();
}

/*
------Setup e Loop------
*/
void 
setup() {

	Serial.begin(9600);

	//Configurando portas
	pinMode(MOTOR_DIREITO_ANTIHORARIO, OUTPUT);
	pinMode(MOTOR_DIREITO_HORARIO, OUTPUT);
	pinMode(MOTOR_ESQUERDO_ANTIHORARIO, OUTPUT);
	pinMode(MOTOR_ESQUERDO_HORARIO, OUTPUT);
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);

	/// Ativando controle
	displayLCD.init();
	displayLCD.backlight();

	// Botão de controle
	attachInterrupt(digitalPinToInterrupt(PINO_BOTAO_DE_CONTROLE), trocarFuncao, FALLING);
}

void 
loop(){

	//Garantia de que a função atual só sera escrita uma unica vez
	if(ultimaFuncaoLoop != funcao){
		reiniciarEstado();
		ultimaFuncaoLoop = funcao;
		escreverLCD(funcao);
	}

	if(millis() - inicioDelayDeTrocaDeModo < 3000){return;};

	if(funcao == "medicao livre"){

		switch (etapa){
			case 1: { // Anda até encontrar uma barreira
				escreverLCD(String("Buscando"), String("barreira"));

				bool poderAndar = ultrassonicoFrontal.Ranging(CM) > DISTANCIA_MINIMA_PAREDE;
				if(poderAndar){
					controlarMotores(-1);
				} else {
					frear(-1);
					delay(500);
					etapa++;
				}

				break;
			}
			case 2: { //Mede a distancia entre a primeira barreira até a próxima
				escreverLCD(String("Medindo reta"));

				if(!medindo){
					ligarMedicaoDeRoda();
				}

				bool poderAndar = ultrassonicoTraseiro.Ranging(CM) > DISTANCIA_MINIMA_PAREDE;
				if(poderAndar){
					controlarMotores(1);
				} else {
					desligarMedicaoDeRoda();
					frear(1);

					int resultadoDeMedicao = pulsos * CMS_POR_PULSO + 2* DISTANCIA_MINIMA_PAREDE + 26;
					escreverLCD("Resultado:", String(resultadoDeMedicao) + " CMs");
					delay(5000);
					etapa++;
				}

				break;
			}
			default:
				controlarMotores(0);
				reiniciarEstado();
				funcao = "desativado";
				break;
		}
	}
}