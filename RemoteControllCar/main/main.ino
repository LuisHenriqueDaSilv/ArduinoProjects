#include <Arduino.h>
#include <Ultrasonic.h>
#include <Dabble.h>

// Motores
#define motor_direito_horario 10
#define motor_direito_anti 11
#define motor_esquerdo_horario 9
#define motor_esquerdo_anti 8

// Bluetooth
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

// Sensores de distancia
#define sensor_frontal_echo A2
#define sensor_frontal_trig A3

#define sensor_direito_echo A1
#define sensor_direito_trig A0

#define sensor_esquerdo_echo A5
#define sensor_esquerdo_trig A4

HC_SR04 ultrassonico_frontal(sensor_frontal_trig, sensor_frontal_echo);
HC_SR04 ultrassonico_direito(sensor_direito_trig, sensor_direito_echo);
HC_SR04 ultrassonico_esquerdo(sensor_esquerdo_trig, sensor_esquerdo_echo);

int distancia_frontal = 0;
int distancia_direita = 0;
int distancia_esquerda = 0;

int ultimo_comando = 0;
/*
 * 0: Parado
 * 1: Andar
 * 2: Virar_esquerda
 * 3: Virar_direita
 * 4: Re
 */

int car_mode = 1;
/*
 * 0: Controlado por bluetooth
 * 1: Autonomo desviando de obstáculos
 * 2: Medindo espaço
 */

char *ultimo_comando_bluetooth[] = {"input"};

void setup(){
	Serial.begin(19200);
	Dabble.begin(9600);

	pinMode(motor_direito_horario, OUTPUT);
	pinMode(motor_direito_anti, OUTPUT);
	pinMode(motor_esquerdo_horario, OUTPUT);
	pinMode(motor_esquerdo_anti, OUTPUT);
}

void loop(){

	Dabble.processInput();

	distancia_frontal = ultrassonico_frontal.distance();
	distancia_direita = ultrassonico_direito.distance();
	distancia_esquerda = ultrassonico_esquerdo.distance();

	if (GamePad.isCrossPressed() && ultimo_comando_bluetooth[0] != "Cross" ) {

		ultimo_comando_bluetooth[0] = "Cross";
		car_mode = 0;
		Serial.println("Modo configurado para 0");
	} else if (GamePad.isSquarePressed() && ultimo_comando_bluetooth[0] != "Square"){

		ultimo_comando_bluetooth[0] = "Square";
		car_mode = 1;
		Serial.println("Modo configurado para 1");
	}

	if (car_mode == 1) {

		if (distancia_frontal > 20) {
			Serial.println("Andando");
			andar();
		} else {

			if (distancia_direita > 20) {
				Serial.println("Virando direita");
				virar_direita();
			}else {

				if (distancia_esquerda > 20) {
					Serial.println("Virando esquerda");
					virar_esquerda();
				} else {
					Serial.println("parando");
					frear();
				}

			}
		}

	} else if (car_mode == 0) {
		if (GamePad.isUpPressed()) {
			andar();
			Serial.println("UP");
		} else if (GamePad.isDownPressed()) {
			re();
			Serial.println("DOWN");
		} else if (GamePad.isLeftPressed()) {
			virar_esquerda();
			Serial.println("Left");
		} else if (GamePad.isRightPressed()) {
			virar_direita();
			Serial.println("Right");
		} else {
			frear();
		}
	}

	//  Serial.print("Sensor direita: " );
	//  Serial.print(distancia_direita);

	//  Serial.print("Sensor esquerda: ");
	//  Serial.print(distancia_esquerda);

	//  Serial.print("Sensor frontal: ");
	//  Serial.print(distancia_frontal);
	//  Serial.println();

	delay(1000);
}

void frear(){

	if (ultimo_comando != 0){

		digitalWrite(motor_esquerdo_anti, LOW);
		digitalWrite(motor_esquerdo_horario, LOW);

		digitalWrite(motor_direito_horario, LOW);
		digitalWrite(motor_direito_anti, LOW);

		delay(1000);
		ultimo_comando = 0;
	}
}

void andar() {

	if (ultimo_comando != 1) {
		frear();
	}

	ultimo_comando = 1;

	digitalWrite(motor_direito_horario, HIGH);
	digitalWrite(motor_direito_anti, LOW);

	digitalWrite(motor_esquerdo_horario, HIGH);
	digitalWrite(motor_esquerdo_anti, LOW);
}

void virar_esquerda() {

	if (ultimo_comando != 2) {
		frear();
	}

	ultimo_comando = 2;

	digitalWrite(motor_direito_horario, HIGH);
	digitalWrite(motor_direito_anti, LOW);

	digitalWrite(motor_esquerdo_horario, LOW);
	digitalWrite(motor_esquerdo_anti, HIGH);
}

void virar_direita() {

	if (ultimo_comando != 3) {
		frear();
	}

	ultimo_comando = 3;

	digitalWrite(motor_direito_horario, LOW);
	digitalWrite(motor_direito_anti, HIGH);

	digitalWrite(motor_esquerdo_horario, HIGH);
	digitalWrite(motor_esquerdo_anti, LOW);
}

void re() {

	if (ultimo_comando != 4) {
		frear();
	}

	ultimo_comando = 4;

	digitalWrite(motor_direito_horario, LOW);
	digitalWrite(motor_direito_anti, HIGH);

	digitalWrite(motor_esquerdo_horario, LOW);
	digitalWrite(motor_esquerdo_anti, HIGH);
}