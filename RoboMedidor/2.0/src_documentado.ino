#include <Arduino.h>
#include <Wire.h> 
#include <Ultrasonic.h> // Biblioteca usada pelos sensores de distancia ultrassonicos.
#include <LiquidCrystal_I2C.h> //  Biblioteca usada pelo painel LCD.


/*
------Portas Usadas pelos sensores e atuadores------
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
#define DISTANCIA_MINIMA_PAREDE 15 // Distancia minima (em CM's) da parede
#define CMS_POR_PULSO 0.48 // Cm's equivalente a um pulso do sensor ENCODER ligado à roda (1 pulsos do sensor encoder -> 0.48CM's)


/*
------Funções para controle dos motores------
*/

void 
controlarMotores(int direcao, bool direcaoInformadaEaDoCarro=false){
    /*
        Parâmetros:
            direcaoInformadaEaDoCarro: Parâmetro responsável por informar se o comando enviado no 
                parametro "direção" refere-se ao sentido de giro dos motores, 
                ou do próprio carro.  
            direcao: caso o parâmetro "direcaoInformadaEaDoCarro" seja true, refere-se ao sentido 
                de giro do carro como um todo.  Caso contrário, refere se ao 
                sentido de giro dos motores, controlando se o carro vai para 
                frente ou para trás.
                -1: Sentido anti-horario;
                1: Sentido horario;
    */


	// Primeiro desliga todos os motores. Garantindo que o controle será 
    // feito de forma correta
	digitalWrite(MOTOR_DIREITO_ANTIHORARIO, 0);
	digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, 0);
	digitalWrite(MOTOR_DIREITO_HORARIO, 0);
	digitalWrite(MOTOR_ESQUERDO_HORARIO, 0);


	if(direcaoInformadaEaDoCarro){


		if(direcao == 1){
			// Gira os motores do lado esquerdo no sentido anti horario e os
			// do lado direito no sentido contrário, fazendo com que o carro gire
			digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, HIGH);
			digitalWrite(MOTOR_DIREITO_HORARIO, HIGH);
		} else if (direcao == -1){
			// Gira os motores do lado direito no sentido anti horario e os
			// do lado esquerdo no sentido contrário, fazendo com que o carro gire
			digitalWrite(MOTOR_ESQUERDO_HORARIO, HIGH);
			digitalWrite(MOTOR_DIREITO_ANTIHORARIO, HIGH);
		}

	} else {

		if(direcao == 1){
			// Faz todos os motores girarem no sentido horario
			digitalWrite(MOTOR_DIREITO_HORARIO, HIGH);
			digitalWrite(MOTOR_ESQUERDO_HORARIO, HIGH);
		} else if(direcao == -1) {
			// Faz todos os motores girarem no anti horario
			digitalWrite(MOTOR_DIREITO_ANTIHORARIO, HIGH);
			digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, HIGH);
		}
		
	}

}

void
frear(int direcaoAtual=1){

  /*
	Função destinada apenas a frear o carrinho da seguinte forma:
		Força o motor a fazer uma força contrária à direção em que ele estava girando
		durante 100 milisegundos, evitando que o carro continue andando devido a inércia,
		após isso, desliga os motores.
  */
  controlarMotores(direcaoAtual*-1);
  delay(100);
  controlarMotores(0);
}


/*
------Configurações do Display LCD------
*/
// Iniciando a biblioteca para o painel LCD de 16 colunas, 2 linhas utilizando 
// o módulo I2C. Localizado no endereço 0x27.
LiquidCrystal_I2C displayLCD(0x27,16,2); // (Endereço I2C do painel, numero de colunas, numero de linhas.)

/*
Variaveis responsáveis por armazenar o texto escritos na primeira e segunda 
linha do painel LCD. Usadas para, ao receber o comando de escrever algo no painel, 
comparar se realmente está havendo uma mudança, para economizar recursos evitando 
operações desnecessárias:  
*/
String primeiraLinhaLCD = "";
String segundaLinhaLCD = "";

void 
escreverLCD(String primeiraLinha, String segundaLinha = ""){

	// Função usada para escrever textos no painel LCD.
	/*
		Pârametros: 
			primeiraLinha: Texto que sera escrito na primeira linha do painel LCD;
			segundaLinha: Texto que sera escrito na segunda linha do painel LCD;

		Antes de escrever algo no LCD, primero é testado se o novo texto recebido 
		é diferente do já escrito. Caso positivo, a linha referente ao novo texto 
		é limpa (Escrevendo "                "), e recebe o novo texto
	*/ 


	if(!primeiraLinha.equalsIgnoreCase(primeiraLinhaLCD)){
		displayLCD.setCursor(0,0); //Seta o cursor do painel na coluna 0, linha 0 (Inicio do painel)
		displayLCD.print("                "); //Escreve uma string vazia no painel, limpando uma linha
		displayLCD.setCursor(0,0); //Seta o cursor do painel na coluna 0, linha 0 (Inicio do painel)
		displayLCD.print(primeiraLinha); //Escreve o texto informando na segunda linha
		primeiraLinhaLCD = primeiraLinha;
	}

	if(!segundaLinha.equalsIgnoreCase(segundaLinhaLCD)){
		displayLCD.setCursor(0,1); //Seta o cursor do painel na coluna 0, linha 1 (Inicio da segunda linha)
		displayLCD.print("                "); //Escreve uma string vazia no painel, limpando uma linha
		displayLCD.setCursor(0,1); //Seta o cursor do painel na coluna 0, linha 1 (Inicio da segunda linha)
		displayLCD.print(segundaLinha); //Escreve o texto informando na segunda linha
		segundaLinhaLCD = segundaLinha;
	}

}


/*
------Controle de funções------
*/

/*
O carro tem 3 funções:
	Desativado: O carro não faz nada, apenas fica aguardando a troca de função.
	Medicao completa: Mede duas dimensões de uma sala usando como referencia duas das paredes.
	Medicao livre:	Mede a distancia entre dois obstáculos.
Para detectar quando o botão é pressionado, é usado a função de interrupção do Arduino. 
Assim que o arduino detecta uma interrupção na porta configurada, a execução da função principal (Loop) 
é pausada e a função configurada é chamada
*/
String funcao = "desativado"; // Variavel responsavel por armazenar a função atual do carro
String ultimaFuncaoLoop; /* Variavel responsavel por armazenar a 
ultima função que foi registrada na função loop.
Dentro da função loop existe uma verificação para testar se a função atual é 
igual a ultima que foi detectada, caso seja diferente, as variaveis globais
do carro são reiniciadas e a nova função é escrita no painel LCD.
*/
volatile long ultimaTrocaDeFuncao = 0; /* Usado para garantir um tempo minimo 
entre as trocas de função. Evitando mal contatos ao pressionar o botão */
long inicioDelayDeTrocaDeFuncao = 0; /* Usado para garantir um delay entre uma 
troca de função e o inicio do funcionamento do carro com tal função */

void 
trocarFuncao(){


	/* 
		O intervalo de tempo entre a ultima troca de função deve ser maior que 500ms
		para evitar interferencias no botão. 

		Tempo desde o inicio da execução do programa - tempo da ultima troca de função =  Intervalo de tempo entre a ultima troca de função e agora
	*/
	if(millis() - ultimaTrocaDeFuncao > 500){  

		/*
			Sequencia de troca de modo:
				desativado -> medicao livre
				medicao livre -> medicao completa
				medicao completa -> retorna ao desativado
		*/
		if(funcao == "desativado"){
			funcao = "medicao livre";
		} else if(funcao == "medicao livre"){
			funcao = "medicao completa";
		} else if (funcao == "medicao completa"){
			funcao = "desativado";
		}

		/* 
			Atribui o tempo atual de execução do programa às variáveis usadas 
			para delay ou tempo minimo dentro do controle de funções do carro:
		*/
		ultimaTrocaDeFuncao = millis();
		inicioDelayDeTrocaDeFuncao = millis();
	}


}


/*
------Sensores de Roda Encoder------
*/

/*
	Funcionamento de um sensor encoder: 
		O sensor é baseado em um disco com algumas ranhuras e duas torres, 
		uma com um led e outra com um sensor de luz. Quando o disco gira entre as
		duas torres, as ranhuras impedem ou permiten passagem de luz para o sensor. 
		Quando a luz é interrompida, o sensor envia um pulso para o arduino, que, usando 
		o sistema de interrupções, contabiliza o pulso. 
*/

volatile long pulsos; // Quantidade de pulsos enviados pelo sensor encoder
volatile bool contandoPulsosEncoder = false; /* Variavel responsável por armazenar 
se o status de contagem de pulsos do sensor encoder. false: desligado. true: ligado 
*/

void
contarPulsos(){

	/*
		Função chamada pelo arduino quando é detectado um pulso na porta configurada
		para o sensor encoder. 
	*/

	pulsos++; // Adiciona mais um pulso na contagem de pulsos.
}

void
ligarMedicaoDeRoda(){
	/*
		Função para ativar a contagem de pulsos do sensor encoder

		attachInterrupt é a função do arduino responsável por configurar as 
		portas com função de interrupção.


	*/
	attachInterrupt(digitalPinToInterrupt(SENSOR_ENCODER), contarPulsos, CHANGE);  // (porta do arduino onde esta ligado o sensor encoder, função chamada ao receber um pulso, tipo de interrupção que deve ser recebida)
	contandoPulsosEncoder = true;
}

void
desligarMedicaoDeRoda(){

	/*
		Função para desativar a contagem de pulsos do sensor encoder
	*/

	detachInterrupt(digitalPinToInterrupt(SENSOR_ENCODER));  // Desliga a função de interrupção na porta do sensor encoder
	contandoPulsosEncoder = false;
}


/*
------Sensores de Distancia ultrassonicos------
*/
Ultrasonic ultrassonicoFrontal(TRIG_ULTRASSONICO_FRONTAL, ECHO_ULTRASSONICO_FRONTAL); //(Trig-Echo)-Sensor de distancia frontal
Ultrasonic ultrassonicoDireito(TRIG_ULTRASSONICO_DIREITO, ECHO_ULTRASSONICO_DIREITO); //(Trig-Echo)-Sensor de distancia direito
Ultrasonic ultrassonicoTraseiro(TRIG_ULTRASSONICO_TRASEIRO, ECHO_ULTRASSONICO_TRASEIRO); //(Trig-Echo)-Sensor de distancia traseiro


/*
------Variaveis de medicao------
*/
int distanciaInicialDaParedeDireita = 0; /* Distancia da parede lateral ao iniciar 
	a medição ou ao fazer uma correção. 
	Usada para calcular a variação de distancia da parede, e com isso, fazer 
	correções para alinhar o carro.
*/
int contadorDeRepeticoes; /* Quantidade de vezes que o carro mediu a mesma 
	parede. Usado para calcular a média entre as medições.
*/
int contadorDeCorrecoes;  // Quantidade de correçoes de alinhamento feitos pelo carro ao decorrer da medição.
int pulsosTotaisDuranteMedicao; // Pulsos recebidos do sensor Encoder durante a medição.
bool indoParaFrente = true; /* Define a direção em que o carrinho está medindo.
	Caso verdadeiro, esta indo para frente, caso contrário, para trás. */
bool corrigindoPercurso = false; // Define se está rodando para fazer uma correção de alinhamento em relação à parede.
int pulsosIniciaisDaCorrecao = 0; /* Quantidade de pulsos recebidos pelo sensor 
encoder até o momento de inicio da correção. Usado para calcular a quantidade de
pulsos necessários para cada correção. */
long momentoDaUltimaCorrecao = 0; /* Tempo em milisegundos entre o inicio da execução 
do programa e a ultima correção. Usado para configurar um intervalo minimo entre 
cada correção */


int primeiraMedicao; /* Armazena o resultado da medição da primeira dimensão da
função "Medicao completa" enquanto a segunda está sendo medida.
*/


/*
------Variaveis globais------
*/
int etapa = 1; // Armazena a 

void
reiniciarEstado(){

	/* 
		Reinicia o estado de todas as variáveis para os valores padrões e 
		desliga as funções opcionais do carro. 
	*/

	// Variaveis globais
	etapa = 1;
	pulsos = 0;

	// Variaveis de medicao
	distanciaInicialDaParedeDireita = 0;
	contadorDeRepeticoes = 0;
	pulsosTotaisDuranteMedicao = 0;
	indoParaFrente = 1;
	pulsosIniciaisDaCorrecao = 0;
	corrigindoPercurso = false;
	contadorDeCorrecoes = 0;
	momentoDaUltimaCorrecao = 0;

	controlarMotores(0);
	desligarMedicaoDeRoda();
}


/*
------Setup e Loop------
*/
void 
setup() {

	// Configurando o modo das portas usadas do Arduino
	pinMode(MOTOR_DIREITO_ANTIHORARIO, OUTPUT); // Porta para controle do motor como saida.
	pinMode(MOTOR_DIREITO_HORARIO, OUTPUT); // Porta para controle do motor como saida.
	pinMode(MOTOR_ESQUERDO_ANTIHORARIO, OUTPUT); // Porta para controle do motor como saida.
	pinMode(MOTOR_ESQUERDO_HORARIO, OUTPUT); // Porta para controle do motor como saida.
	pinMode(2, INPUT_PULLUP); // Porta 2 (Sensor encoder) configurado como porta de  interrupção
	pinMode(3, INPUT_PULLUP); // Porta 3 (Botão de troca de função) onfigurado como porta de  interrupção

	displayLCD.init(); 	// Ativa o display LCD:
	displayLCD.backlight(); // Acende a luz de fundo do painel lcd

	// Configurando o modo de interrupção do botão de troca de função
	attachInterrupt(digitalPinToInterrupt(PINO_BOTAO_DE_CONTROLE), trocarFuncao, FALLING); // (porta do arduino onde esta ligado o botão, função chamada ao receber um pulso, tipo de interrupção que deve ser recebida)
}

void 
loop(){

	
	if(ultimaFuncaoLoop != funcao){ 
		/* 
			Se a função atual for diferente da função da ultima chamada do loop, 
			as variaveis são reiniciadas e a nova função é escrita no painel.
		*/
		reiniciarEstado();
		ultimaFuncaoLoop = funcao;
		escreverLCD(funcao);
	}

	if(millis() - inicioDelayDeTrocaDeFuncao < 3000){return;};

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
				escreverLCD(String("medindo reta"));

				if(!contandoPulsosEncoder){
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
			default: {

				controlarMotores(0);
				reiniciarEstado();
				funcao = "desativado";

				break;
			}
		}
} else if(funcao == "medicao completa"){


		switch (etapa){
			case 1: { // Anda até encontrar uma barreira;
				escreverLCD(String("buscando"), String("barreira"));

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

			case 2: { // Cria uma distancia segura para realizar o giro;
				escreverLCD(String("afastando"), String("para girar"));

				if(ultrassonicoFrontal.Ranging(CM) < 20){
					controlarMotores(1);
				}else {
					frear(1);
					delay(500);
					etapa++;
				}

				break;
			}

			case 3: { // Faz um giro de 90° para ficar em paralelo com a parede
				escreverLCD(String("girando"));

				if(!contandoPulsosEncoder){
					ligarMedicaoDeRoda();
				}
				
				if(pulsos < 40){
					controlarMotores(-1, true);
				} else {
					controlarMotores(1, true);
					delay(100);
					controlarMotores(0);

					etapa++;
				}
				
				break;
			}

			case 4: {

				if(distanciaInicialDaParedeDireita == 0){
					distanciaInicialDaParedeDireita = ultrassonicoDireito.Ranging(CM);
				}

				if(corrigindoPercurso){
					escreverLCD(String("realizando"), String("correcao"));

					if(pulsos - pulsosIniciaisDaCorrecao > 1){
						controlarMotores(0);
						corrigindoPercurso = false;
						contadorDeCorrecoes++;
						delay(500);

						distanciaInicialDaParedeDireita = 0;

						momentoDaUltimaCorrecao = millis();

						return;
					} else return;
				} else if(contadorDeRepeticoes == 0){
					escreverLCD(String("buscando inicio"), String("para medicao"));
				} else {
					escreverLCD(
						"medicao "+(String)contadorDeRepeticoes+"/5"
					);
				}

				bool podeAndar = indoParaFrente? 
					(ultrassonicoFrontal.Ranging(CM) > DISTANCIA_MINIMA_PAREDE) 
						: 
					(ultrassonicoTraseiro.Ranging(CM) > DISTANCIA_MINIMA_PAREDE);

				if(podeAndar){

					int variacaoLateral = ultrassonicoDireito.Ranging(CM) - distanciaInicialDaParedeDireita;

					if(abs(variacaoLateral) > 3 && abs(variacaoLateral) < 50 && millis() - momentoDaUltimaCorrecao > 1500){

						frear(indoParaFrente? -1:1);
						corrigindoPercurso = true;
						delay(500);
						pulsosIniciaisDaCorrecao = pulsos;

						if((indoParaFrente && variacaoLateral > 0) || (!indoParaFrente && variacaoLateral < 0)) {
							controlarMotores(1, true);
						} else {
							controlarMotores(-1, true);
						}

					} else {
						controlarMotores(indoParaFrente? -1:1);
					}


				} else {

					frear(indoParaFrente? -1:1);

					if(contadorDeRepeticoes == 0){
						pulsos = 0;
						ligarMedicaoDeRoda();

						indoParaFrente = !indoParaFrente;
						contadorDeRepeticoes++;
					} else if(contadorDeRepeticoes == 5) {

						pulsosTotaisDuranteMedicao = pulsosTotaisDuranteMedicao+pulsos;

						if(primeiraMedicao == 0){
							primeiraMedicao = (pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes * CMS_POR_PULSO +(2*DISTANCIA_MINIMA_PAREDE) +26;
							escreverLCD("Aguarde", String(primeiraMedicao) +" cms");
							delay(2000);
							reiniciarEstado();

						} else {
							int resultado = (pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes * CMS_POR_PULSO +(2*DISTANCIA_MINIMA_PAREDE) +26;
							escreverLCD(String(primeiraMedicao) +" cms", String(resultado)+" cms");
							delay(5000);
							reiniciarEstado();
							funcao = "desativado";
							primeiraMedicao = 0;
						}


					} else {
						pulsosTotaisDuranteMedicao = pulsosTotaisDuranteMedicao+pulsos;
						escreverLCD(String(pulsosTotaisDuranteMedicao));
						delay(1000);

						contadorDeRepeticoes++;
						indoParaFrente = !indoParaFrente;
						pulsos = 0;
						ligarMedicaoDeRoda();
					}

				}
				
				break;
			}
			
			default: {
				controlarMotores(0);
				reiniciarEstado();
				funcao = "desativado";

				break;
			}
		}
	}
}