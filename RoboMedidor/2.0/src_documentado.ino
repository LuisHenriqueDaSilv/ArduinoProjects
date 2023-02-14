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
#define CMS_POR_PULSO 0.48 // Cm's equivalente a um pulso do sensor ENCODER ligado à roda (1 pulsos do sensor encoder -> 0.48CM's).


/*
------Funções para controle dos motores------
*/

void desligarMotores(){
	digitalWrite(MOTOR_DIREITO_ANTIHORARIO, LOW);
	digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, LOW);
	digitalWrite(MOTOR_DIREITO_HORARIO, LOW);
	digitalWrite(MOTOR_ESQUERDO_HORARIO, LOW);
}

void andar(String direcao){

	/*
		Função destinada a ligar os motores no sentido anti horario ou horario, fazendo com que o carro ande para frente ou para trás, respectivamente
	*/

	desligarMotores();

	if (direcao == "frente"){
		// Liga os motores em sentido anti-horario, fazendo com que o carro ande para frente
		digitalWrite(MOTOR_DIREITO_ANTIHORARIO, HIGH);
		digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, HIGH);
	}
	else if (direcao == "ré"){
		// Liga os motores em sentido horario, fazendo com que o carro ande para trás
		digitalWrite(MOTOR_DIREITO_HORARIO, HIGH);
		digitalWrite(MOTOR_ESQUERDO_HORARIO, HIGH);
	}
}

void virarCarro(String lado){

	/*
		Função destinada a ligar os pares de motores laterais de forma separada, fazendo a frente do carro girar para os lados. 
	*/

	desligarMotores();

	if (lado == "esquerda"){
		// Liga o motor esquerdo em sentido horario e direito em anti-horario, fazendo com que a frente do carro gire para a esquerda
		digitalWrite(MOTOR_ESQUERDO_HORARIO, HIGH);
		digitalWrite(MOTOR_DIREITO_ANTIHORARIO, HIGH);
	}
	else if (lado == "direita"){
		// Liga o motor direito em sentido horario e esquerdo em anti-horario, fazendo com que a frente do carro gire para a direita
		digitalWrite(MOTOR_ESQUERDO_ANTIHORARIO, HIGH);
		digitalWrite(MOTOR_DIREITO_HORARIO, HIGH);
	}
}

void frear(String direcaoAtual){

	/*
	  Função destinada a frear o carrinho da seguinte forma:
		Força o motor a fazer uma força contrária à direção em que ele estava girando
		durante 100 milisegundos, evitando que o carro continue andando devido a inércia,
		após isso, desliga os motores.
	*/

	if (direcaoAtual == "frente"){
		andar("ré");
	}
	else{
		andar("frente");
	}
	delay(100);
	desligarMotores();
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

Para alterar entre os modos, é necessário que o botão posicionado na parte frontal 
do carro seja pressionado uma unica vez para alterar entre o modo atual e o próximo. 
A sequencia em que as funções são alteradas é a seguinte:
	desativado -> medicao livre
	medicao livre -> medicao completa
	medicao completa -> retorna ao desativado

Para detectar quando o botão é pressionado, é usado a função de interrupção do Arduino. 
Assim que o arduino detecta uma interrupção na porta configurada, a execução da função principal (Loop) 
é pausada e a função configurada no setup (atráves  da atachInterrupt) é chamada
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
ligarContagemDePulsosSensorEncoder(){
	/*
		Função para ativar a contagem de pulsos do sensor encoder

		attachInterrupt é a função do arduino responsável por configurar as 
		portas com função de interrupção.
	*/
	attachInterrupt(digitalPinToInterrupt(SENSOR_ENCODER), contarPulsos, CHANGE);  // (porta do arduino onde esta ligado o sensor encoder, função chamada ao receber um pulso, tipo de interrupção que deve ser recebida)
	contandoPulsosEncoder = true;
}

void
desligarContagemDePulsosSensorEncoder(){

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
		desliga as funções opcionais do carro. Para fazer uma nova medição
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

	desligarMotores();
	desligarContagemDePulsosSensorEncoder();
}


/*
------Setup e Loop------
*/

void 
setup() {

	Serial.begin(9600);

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

	
	// Escrever mensagem inicial do robo dando um "tutorial" de como selecionar os modos: 
	escreverLCD("Aperte o botao", "frontal para..."); 
	delay(2000);
	escreverLCD("variar entre", "as funcoes de...");
	delay(2000);
	escreverLCD("medicao.");
	delay(2000);

}

void 
loop(){

	
	if(ultimaFuncaoLoop != funcao){ 
		/* 
			Se a função atual for diferente da função na ultima chamada do loop, 
			as variaveis são reiniciadas e a nova função é escrita no painel.
		*/
		reiniciarEstado();
		ultimaFuncaoLoop = funcao;
		escreverLCD(funcao);

		if(funcao == "desativado"){
		}
	}

	if(millis() - inicioDelayDeTrocaDeFuncao < 3000){return;}; /* Delay de 3 segundos 
		entre a ultima troca de função e o inicio da medição para que o usuário tenha 
		tempo de ler a função selecionada. 
	*/ 


	//Estrutura de controle das funções:
	if(funcao == "medicao livre"){

		/*
			No modo "medicao livre" o carro deve andar para frente até encontrar 
			o primeiro obstáculo. Em seguida, começara a andar no sentido contrário
			contando os pulsos do sensor encoder, medindo assim, a distancia entre
			os dois obstáculos.
		*/

		switch (etapa){
			case 1: { // Primeira etapa: Anda até encontrar um obstáculo
				escreverLCD(String("Buscando"), String("barreira")); // Escreve no painel LCD a etapa atual

				/* 
					O carro só pode andar se a distancia atual do sensor frontal 
					for maior que a distancia minima configurada pela constante 
					"DISTANCIA_MINIMA_PAREDE". Caso contrário, a etapa de encontrar 
					o primeiro obstáculo está completa e o carro pode passar para 
					a próxima etapa  
				*/
				bool podeAndar = ultrassonicoFrontal.Ranging(CM) > DISTANCIA_MINIMA_PAREDE; 
				if(podeAndar){
					// Liga os motores.
					andar("frente");
				} else {
					// Freia e passa para a próxima etapa.
					frear("frente");
					delay(500);
					etapa++;
				}

				break;
			}
			case 2: { // Etapa 2: Mede a distancia entre o primeiro obstáculo encontrado e o primeiro obstáculo do sentido contrário.
				escreverLCD(String("medindo reta"));// Escreve no painel LCD a etapa atual

				if(!contandoPulsosEncoder){ // Ativa a contagem de pulsos do sensor encoder caso ainda não esteja ativa.
					ligarContagemDePulsosSensorEncoder(); 
				}

				/* 
					O carro só pode andar se a distancia atual do sensor traseiro 
					for maior que a distancia minima configurada pela constante 
					"DISTANCIA_MINIMA_PAREDE". Caso contrário, a etapa de medição 
					foi completa. O resultado deve ser mostrado no painel LCD durante
					5 segundos, o carro e suas variáveis são reiniciados ao estado 
					inicial de aguardar um novo comando. 
				*/

				bool podeAndar = ultrassonicoTraseiro.Ranging(CM) > DISTANCIA_MINIMA_PAREDE;
				if(podeAndar){
					// Liga os motores
					andar("ré");
				} else {
					desligarContagemDePulsosSensorEncoder(); 
					frear("ré");

					/*
						Calcula a distancia em CM's usando a quantidade de pulsos 
						do sensor Encoder:
							tamanho em cms = pulsos * CMS_POR_PULSO + 2* DISTANCIA_MINIMA_PAREDE + 26
								Pulsos: Quantidade de pulsos do sensor encoder.
								2*DISTANCIA_MINIMA_PAREDE: Distancia do primeiro obstáculo + distancia do segundo obstáculo.
								26: Distancia entre uma ponta e outra do carro. 
					*/
					int resultadoDeMedicao = pulsos * CMS_POR_PULSO + 2* DISTANCIA_MINIMA_PAREDE + 26;
					escreverLCD("Resultado:", String(resultadoDeMedicao) + " CMs"); // Escreve no painel LCD o resultado da medição .
					delay(5000); 
					etapa++; /* Passa para a próxima etapa. Por não existir o 
								caso 3 neste estrutura de switch/case, acaba 
								caindo no default. Onde o carro é 
								reiniciado.
							*/
				}

				break;
			}
			default: {

				desligarMotores();
				reiniciarEstado(); // Reinicia o valor de todas as variaveis para os valores padrões
				funcao = "desativado"; // Desativa o carro.
				break;
			}
		}
		
	} else if(funcao == "medicao completa"){

		/*
			No modo "medicao completa" o carro deve andar para frente até encontrar 
			o primeiro obstáculo, girar ao redor de um ponto central, buscando 
			ficar em paralelo à parede e voltar a procurar um obstáculo. Após isso,
			o carro deve estar em paralelo a uma parede em seu lado direito, e 
			de frente para outra. E enfim, começara a medir a primeira dimensão 
			da sala de forma semelhante ao modo "medição livre", porem, 
			repetindo a mesma linha diversas vezes para ter uma maior precisão. 
			Após fazer a mesma linha algumas vezes, o resultado é armazenado na variável,
			"primeiraMedicao" e o carro volta para a primeiro etapa de procurar 
			um obstaculo e girar, porem, dessa vez ficando em paralelo à parede 
			relativa à segunda dimensão da sala. 
		*/


		switch (etapa){
			case 1: { // Etapa 1: Anda até encontrar um obstáculo.

				/* 
					O carro só pode andar se a distancia atual do sensor frontal 
					for maior que a distancia minima configurada pela constante 
					"DISTANCIA_MINIMA_PAREDE". Caso contrário, a etapa de encontrar 
					o primeiro obstáculo está completa e o carro pode passar para 
					a próxima etapa  
				*/
				escreverLCD(String("buscando"), String("barreira")); // Escreve a etapa no painel LCD

				bool podeAndar = ultrassonicoFrontal.Ranging(CM) > DISTANCIA_MINIMA_PAREDE;
				if(podeAndar){
					//Liga os motores
					andar("frente");
				} else {
					// Freia e passa para a próxima etapa.
					frear("frente"); 
					delay(500);
					etapa++;
				}

				break;
			}

			case 2: { // Etapa 2: criar uma distancia maior que a distancia minima da parede para que possa realizar o giro de forma segura.
				escreverLCD(String("afastando"), String("para girar"));// Escreve a etapa no painel LCD

				if(ultrassonicoFrontal.Ranging(CM) < 20){ // Anda para trás até criar uma distancia de 20cm
					andar("ré");
				}else {
					// Freia e passa para a próxima etapa.
					frear("ré");
					delay(500);
					etapa++;
				}

				break;
			}

			case 3: { // Etapa 3: Faz um giro ao redor do ponto central até ficar em paralelo à parede.
				escreverLCD(String("girando")); // Escreve a etapa no painel LCD

				/*
					Para fazer este giro de forma mais preciso é usada a contagem
					de pulsos do sensor Encoder. Por testes, cheguei a conclusão de que a melhor quantidade de pulsos para 
					o giro de 90° são 40 pulsos.
				*/
				if(!contandoPulsosEncoder){
					ligarContagemDePulsosSensorEncoder();
				}
				
				if(pulsos < 40){
					// Gira o carro:
					virarCarro("esquerda");
				} else {
					// Freia o giro e passa para a próxima etapa.
					virarCarro("esquerda");
					delay(100);
					desligarMotores();

					etapa++;
				}
				
				break;
			}

			case 4: {// Etapa 4: Vai de um obstáculo ate outro, medindo e fazendo correções para que o trajeto seja feito a forma mais reta o possivel.

				if(distanciaInicialDaParedeDireita == 0){ /* Se a distancia inicial da parede 
				direita for igual a 0, provavelmente é a primeira vez que o loop
				é chamado neste etapa ou uma correção acaba de ser feita. Em 
				ambos os caos, o valor deve configurado para a distancia atual */
					distanciaInicialDaParedeDireita = ultrassonicoDireito.Ranging(CM);
				}

				if(corrigindoPercurso){ /* Se foi detectada a necessidade de fazer
				uma correção de trajeto, a variavel "corrigindoPercurso" foi setada 
				como verdadeira e inicia o giro. Este giro deve se manter até 
				alcançar a quantidade configurada de pulsos. 
				*/
					escreverLCD(String("realizando"), String("correcao")); //Escreve o estado atual no painel LCD

					if(pulsos - pulsosIniciaisDaCorrecao > 1){ /* Se a quantidade contada
					de pulsos após a necessidade de correção for maior que 1, 
					a correção foi realizada e pode voltar ao trajeto normal.
					*/ 
						desligarMotores();
						corrigindoPercurso = false;
						contadorDeCorrecoes++; // Contagem necessária para que os pulsos contabilizados durante as correções sejam desconsiderados no resultado final.
						delay(500);

						distanciaInicialDaParedeDireita = 0;// Valor reiniciado para que na próxima chamada do loop seja dado o novo valor de distancia.

						momentoDaUltimaCorrecao = millis(); // Usado para dar um intervalo entre as correções e evitar interferencias dos sensores.

						return;
					} else return;
				} else if(contadorDeRepeticoes == 0){ /* Na primeira vez fazendo 
					aquela linha, o carro provavelmente iniciou em algum ponto 
					aleatório da parede, sendo necessário ir para o inicio 
					antes de iniciar a verdadeira medição 
					*/
					escreverLCD(String("buscando inicio"), String("para medicao")); //Escreve o status no painel LCD
				} else {
					escreverLCD(
						"medicao "+(String)contadorDeRepeticoes+"/5"
					); // Escreve o numero da medição atual para que o usuário possa acompanhar o processo. 
				}


				/* 
					O carro só pode andar se a distancia atual do sensor referente 
					a direção do carro  for maior que a distancia minima configurada 
					pela constante "DISTANCIA_MINIMA_PAREDE". Caso contrário, 
					a linha esta completa e o carro deve ou refazer a linha, ou 
					caso já tenha feito todas as repetições, passar para a próxima etapa.
				*/

				bool podeAndar = indoParaFrente? 
					(ultrassonicoFrontal.Ranging(CM) > DISTANCIA_MINIMA_PAREDE) // Caso esteja indo para frente, o sensor referente a direção é o sensor frontal
						: 
					(ultrassonicoTraseiro.Ranging(CM) > DISTANCIA_MINIMA_PAREDE); // Caso esteja indo para trás, o sensor referente a direção é o sensor traseiro

				if(podeAndar){

					int variacaoLateral = ultrassonicoDireito.Ranging(CM) - distanciaInicialDaParedeDireita; 
					/* 
						Variação lateral é o quanto a distancia
						entre o carro e a parede do lado direito mudou, caso 
						esta mudança for superior a 3cm's, é necessário uma 
						correção de trajeto
					*/

					if(abs(variacaoLateral) > 3 && abs(variacaoLateral) < 50 && millis() - momentoDaUltimaCorrecao > 1500){ 
						/* 
							Se a variacao de distancia lateral for maior que 3cms,
							menor que 50cms (Para evitar interferencias do sensor
							de distancia), e já tiver se passado mais que 1segundo
							e meio desde a ultima correção, uma nova é iniciada.
						*/
						if(indoParaFrente){
							frear("frente");
						} else {
							frear("ré");
						}
						corrigindoPercurso = true; // Atualiza o status de medição para realizando uma correção
						delay(500);
						pulsosIniciaisDaCorrecao = pulsos;// Configura a quantidade de pulsos ao iniciar a correção

						if((indoParaFrente && variacaoLateral > 0) || (!indoParaFrente && variacaoLateral < 0)) {

							/* 
								Se o carro estiver indo para frente e a variação 
								for para fora em relação à parede, ou o carro estiver 
								indo para trás e a variação for para dentro em 
								relação a parede , significa que o carro deve girar em sentido horario
							*/
							virarCarro("direita");
						} else {
							/* 
								Se o carro estiver indo para frente e a variação 
								for para dentro em relação à parede, ou o carro estiver 
								indo para trás e a variação for para fora em 
								relação a parede , significa que o carro deve girar em sentido antihorario
							*/
							virarCarro("esquerda");
						}

					} else {
						if(indoParaFrente){// Caso não seja necessário nenhuma correção, o carro pode andar normalmente em seu trajeto atual.
							andar("frente");
						} else {
							andar("ré"); 
					} 
					}

				} else {

					/*
						Caso o carro tenha encontrado um obstáculo, significa 
						que ele deve voltar a percorrer o mesmo trajeto em 
						sentido contrário ou mostrar o resultado para o usuario 
						pelo painel LCD.
					*/
					
					if(indoParaFrente){
						frear("frente");
					} else {
						frear("ré"); 
					}

					if(contadorDeRepeticoes == 0){ 
					/* 
						Caso seja a primeira repetição  da medição, a contagem de
						pulsos deve ser ignorada por provavelmente o carro não 
						ter iniciado a medição no inicio.
					*/
						pulsos = 0;
						ligarContagemDePulsosSensorEncoder();

						indoParaFrente = !indoParaFrente; // Inverte o sentido do carro
						contadorDeRepeticoes++; // Soma mais um na contagem de repetições de medição
					} else if(contadorDeRepeticoes == 5) {

						/*
							Caso já tenham sido feitas 5 medições na mesma linha 
							já podemos fazer a média de pulsos e calcular a 
							distancia entre os dois obstáculos. 
						*/

						pulsosTotaisDuranteMedicao = pulsosTotaisDuranteMedicao+pulsos;

						if(primeiraMedicao == 0){
							/* 
								Caso ainda seja a medição da primeira dimensão da 
								sala, o robo deve armazenar o resultado na variável 
								primeiraMedicao e reiniciar este processo para que 
								seja medido a segunda dimensão
							*/

							/*
								resultado = (pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes * CMS_POR_PULSO +(2*DISTANCIA_MINIMA_PAREDE) +26;
									(pulsosTotaisDuranteMedicao- contadorDeCorrecoes): Quantidade de pulsos desconsiderando os usados para fazer correções.
									(pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes: Média de pulsos por cada repetição da medição feita.
									2*DISTANCIA_MINIMA_PAREDE: Distancia do primeiro obstáculo + distancia do segundo obstáculo.
									26: Tamanho do carro.
							*/
							primeiraMedicao = (pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes * CMS_POR_PULSO +(2*DISTANCIA_MINIMA_PAREDE) +26;
							escreverLCD("Aguarde", String(primeiraMedicao) +" cms"); //Escreve o resultado a primeira medição no painel LCD para que o usuario possa acompanhar.
							delay(2000); // Garante que a mensagem fique no painel LCD durante dois segundos
							reiniciarEstado(); // Reinicia as variaveis para que o carro volte para a primeira etapa, medindo a segunda dimensão da sala.

						} else {
							/* 
								Ao realizar a medição da segunda dimensão da sala, 
								o resultado deve ser calculado, exibido no painel LCD 
								e o carro deve ser reiniciado.
							*/

							/*
								resultado = (pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes * CMS_POR_PULSO +(2*DISTANCIA_MINIMA_PAREDE) +26;
									(pulsosTotaisDuranteMedicao- contadorDeCorrecoes): Quantidade de pulsos desconsiderando os usados para fazer correções.
									(pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes: Média de pulsos por cada repetição da medição feita.
									2*DISTANCIA_MINIMA_PAREDE: Distancia do primeiro obstáculo + distancia do segundo obstáculo.
									26: Tamanho do carro.
							*/
							int resultado = (pulsosTotaisDuranteMedicao- contadorDeCorrecoes)/contadorDeRepeticoes * CMS_POR_PULSO +(2*DISTANCIA_MINIMA_PAREDE) +26;
							escreverLCD(String(primeiraMedicao) +" cms", String(resultado)+" cms"); //Escreve o resultado final no painel LCD
							delay(5000); // Garante que o resultado fique escrito no painel durante 5 segundos
							reiniciarEstado(); // Reinicia todas as variáveis do carro
							primeiraMedicao = 0;
							funcao = "desativado"; // Volta o status do carro para o inicial, onde  apenas espera por outro comando
						}


					} else {
						/*
							Caso todas as medições configuradas ainda não tenham 
							sido feitas, a contagem de pulsos atuais são armazenadas 
							e o sentido do carro é invertido. Iniciando assim 
							outra medição 
						*/
						pulsosTotaisDuranteMedicao = pulsosTotaisDuranteMedicao+pulsos; // Armazena a quantidade de pulsos contados neste medida
						escreverLCD(String(pulsosTotaisDuranteMedicao)); 
						delay(1000);

						contadorDeRepeticoes++;
						indoParaFrente = !indoParaFrente; // Inverter a direção do carro
						pulsos = 0; // Reinicia a contagem de pulsos do sensor Encoder
						ligarContagemDePulsosSensorEncoder(); // Garante que a contagem de pulsos do sensor Encoder esteja ligado
					}

				}
				
				break;
			}
			
			default: {
				desligarMotores();
				reiniciarEstado();
				funcao = "desativado";

				break;
			}
		}
	}
}