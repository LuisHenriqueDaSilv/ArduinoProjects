#include <Arduino.h>
#include <Ultrasonic.h>

// Motores
#define motor_direito_horario 5
#define motor_direito_anti 4
#define motor_esquerdo_horario 7
#define motor_esquerdo_anti 6

// Gerais
int passo = 0;
int distancia_limite = 15; // CM
bool primeira_linha = true;

// Sensores de distancia
#define sensor_frontal_echo A2
#define sensor_frontal_trig A3

#define sensor_direito_echo A0
#define sensor_direito_trig A1

#define sensor_esquerdo_echo A5
#define sensor_esquerdo_trig A4 

#define sensor_traseiro_echo 3
#define sensor_traseiro_trig 2

Ultrasonic ultrassonico_frontal(sensor_frontal_trig, sensor_frontal_echo);
Ultrasonic ultrassonico_direito(sensor_direito_trig, sensor_direito_echo);
Ultrasonic ultrassonico_esquerdo(sensor_esquerdo_trig, sensor_esquerdo_echo);
Ultrasonic ultrassonico_traseiro(sensor_traseiro_trig, sensor_traseiro_echo);

int distancia_frontal = 0;
int distancia_direita = 0;
int distancia_esquerda = 0;
int distancia_traseira = 0;

bool lado_parede_e_direita = false;
int distancia_inicial_parede = 0;
int divisor_de_curva = 2;
int variacao_distancia_lateral = 0;

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

  switch(passo){

    case 0: //Procura a parede na frente do robo

      if(distancia_frontal >= distancia_limite){
        setar_motores(-1, -1);
      }else {
        setar_motores(1, 1);
        delay(50);
        setar_motores(0, 0);

        mudar_passo(1);
      }
      break;

    case 1: // Girar o robo para um lado que não tenha parede

      if(distancia_esquerda <= distancia_limite){
        setar_motores(0, 1);
        
        lado_parede_e_direita = true;
        distancia_inicial_parede = distancia_direita;
        
      } else {
        setar_motores(1, 0);

        lado_parede_e_direita = false;
        distancia_inicial_parede = distancia_esquerda;
      }

      delay(1800);
      setar_motores(0,0);

      primeira_linha = true;
      mudar_passo(2);

      break;

    case 2: //Andar em uma direção corrigindo o angulo em relação a parede até encontra-la

      bool pode_andar = false;

      if(primeira_linha){
        pode_andar = distancia_frontal > distancia_limite;
      } else {
        pode_andar = distancia_traseira > distancia_limite;
      }

      if(pode_andar){

        if(lado_parede_e_direita){
          variacao_distancia_lateral = distancia_direita - distancia_inicial_parede;
        }else {
          variacao_distancia_lateral = distancia_esquerda - distancia_inicial_parede;
        }

        if(abs(variacao_distancia_lateral) > 5){

          if(variacao_distancia_lateral > 0){

            if(lado_parede_e_direita){
              setar_motores(1, 0);
            }else {
              setar_motores(0, 1);
            }

          }else {

            if(lado_parede_e_direita){
              setar_motores(0, 1);
            }else {
              setar_motores(1, 0);
            }
          }

          delay(1800/divisor_de_curva);
          setar_motores(0, 0);
          divisor_de_curva = divisor_de_curva * 2;
          
          delay(1000);
        }else {
          if(primeira_linha){
            setar_motores(-1, -1);
          } else {
            setar_motores(1, 1);
          }
        }

      }else {
        if(primeira_linha){
          setar_motores(1, 1);
        }else {
          setar_motores(-1, -1);
        }
        delay(50);
        setar_motores(0, 0);
        primeira_linha = false;

      }

      break;

    default:
      setar_motores(0,0);
  }


}

void mudar_passo(int proximo_passo){
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
