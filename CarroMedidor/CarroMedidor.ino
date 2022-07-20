#include <Arduino.h>
#include <Ultrasonic.h>

// Motores
#define motor_direito_horario 9
#define motor_direito_anti 8
#define motor_esquerdo_horario 10
#define motor_esquerdo_anti 11

// Gerais
int passo = 0;
int distancia_limite = 10; // CM

// Sensores de distancia
#define sensor_frontal_echo 7
#define sensor_frontal_trig 6

#define sensor_direito_echo A5
#define sensor_direito_trig A4

#define sensor_esquerdo_echo A1
#define sensor_esquerdo_trig A0 

#define sensor_traseiro_echo A2
#define sensor_traseiro_trig A3

Ultrasonic ultrassonico_frontal(sensor_frontal_trig, sensor_frontal_echo);
Ultrasonic ultrassonico_direito(sensor_direito_trig, sensor_direito_echo);
Ultrasonic ultrassonico_esquerdo(sensor_esquerdo_trig, sensor_esquerdo_echo);
Ultrasonic ultrassonico_traseiro(sensor_traseiro_trig, sensor_traseiro_echo);

float distancia_frontal = 0;
float distancia_direita = 0;
float distancia_esquerda = 0;
float distancia_traseira = 0;


// Curva 
bool testando_se_ta_reto = false;
int inicio_teste_se_ta_reto = 0;
int duracao_teste_se_ta_reto = 5000;
int distancia_ponto_de_inicio = 0;
bool ta_indo_pra_frente = true;
int timer = millis();
float divisor_de_tempo = 2;

void setup(){
  Serial.begin(9600);

  pinMode(motor_direito_horario, OUTPUT);
  pinMode(motor_direito_anti, OUTPUT);
  pinMode(motor_esquerdo_horario, OUTPUT);
  pinMode(motor_esquerdo_anti, OUTPUT);
}

void loop(){

  distancia_frontal = ultrassonico_frontal.Ranging(CM);
  distancia_direita = ultrassonico_direito.Ranging(CM);
  distancia_esquerda = ultrassonico_esquerdo.Ranging(CM);
  distancia_traseira = ultrassonico_traseiro.Ranging(CM);
  
  timer = millis();

  switch(passo){


    case 0:
      if(distancia_frontal >= distancia_limite){
        setar_motores(-1, -1);
      }else {
        setar_motores(1, 1);
        delay(50);
        setar_motores(0, 0);

        passo = 1;
      }
      break;

    case 1:
      intervalo(2);
      break;

    case 2:
      if(distancia_traseira >= distancia_limite){
        setar_motores(1, 1);
      }else {
        setar_motores(-1, -1);
        delay(50);
        setar_motores(0,0);
        passo = 3;
      }
      break;

    case 3: 
      intervalo(4);
      break;

    case 4: 
      setar_motores(-1, 0);
      delay(2000);
      setar_motores(0,0);
      passo = 5;
      break;

    case 5: 
      intervalo(6);
      break;

    case 6:

      if(testando_se_ta_reto){

        if(
          (
            (timer - inicio_teste_se_ta_reto) > duracao_teste_se_ta_reto
          ) || (
            ta_indo_pra_frente && distancia_frontal < distancia_limite
          ) || (
            !ta_indo_pra_frente && distancia_traseira < distancia_limite
          )
        ){

          setar_motores(0, 0);

          int variacao_de_distancia = distancia_esquerda - distancia_ponto_de_inicio;

          if(abs(variacao_de_distancia) > 3){


            if(variacao_de_distancia > 0){
              setar_motores(0, -1);
            } else {
              setar_motores(-1, 0);
            }

            delay(2000/divisor_de_tempo);
            setar_motores(0, 0);

            delay(1000);

            divisor_de_tempo= divisor_de_tempo +2;
            testando_se_ta_reto = false;

          }else {
            passo = 7;
          }

          
        }

      }else {

        inicio_teste_se_ta_reto = timer;
        testando_se_ta_reto = true;
        distancia_ponto_de_inicio = distancia_esquerda;
        ta_indo_pra_frente = !ta_indo_pra_frente;

        if(ta_indo_pra_frente){
          setar_motores(1, 1);
        } else {
          setar_motores(-1, -1);
        }
      }

      break;

    case 7: 
      passo =0;
      break;
    default:
      setar_motores(0,0);
  }
}

void intervalo(int proximo_passo){
  setar_motores(0,0);
  delay(2000); 
  passo = proximo_passo;
}

void setar_motores(int direita, int esquerda){


  switch(direita){

    case -1:
	    digitalWrite(motor_direito_horario, LOW);
	    digitalWrite(motor_direito_anti, HIGH);
      break;
    case 1: 
	    digitalWrite(motor_direito_horario, HIGH);
	    digitalWrite(motor_direito_anti, LOW);
      break;
    default: 
      digitalWrite(motor_direito_horario, LOW);
	    digitalWrite(motor_direito_anti, LOW);
  }

  switch(esquerda){

    case -1: 
	    digitalWrite(motor_esquerdo_horario, LOW);
	    digitalWrite(motor_esquerdo_anti, HIGH);
      break;
    case  1: 
	    digitalWrite(motor_esquerdo_horario, HIGH);
	    digitalWrite(motor_esquerdo_anti, LOW);
      break;
    default: 
      digitalWrite(motor_esquerdo_horario, LOW);
	    digitalWrite(motor_esquerdo_anti, LOW);

  }

}
