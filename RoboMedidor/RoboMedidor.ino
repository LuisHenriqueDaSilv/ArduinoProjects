#include <Arduino.h>
#include <Ultrasonic.h>

// Motores:
#define motor_direito_horario 5
#define motor_direito_anti 4
#define motor_esquerdo_horario 7
#define motor_esquerdo_anti 6

// Gerais
int passo = 0;
int distancia_limite = 10; // CM
int sentido = 1;
bool pode_andar = false;


// Sensores de distancia:
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


// Curva e alinhamento:

bool comeco_de_linha = false;
bool lado_parede_e_direita = false;
int distancia_inicial_parede = 0;
int variacao_de_distancia_da_parede = 0;
int momento_da_ultima_correcao = 0; 
bool corrigiu = false;
bool primeira_linha = true;




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

    case 0: { //Procura a parede na frente do robo

      if(distancia_frontal >= distancia_limite){
        ligar_motores(-1, -1);
      } else {
        ligar_motores(1, 1);
        delay(50);
        ligar_motores(0, 0);

        mudar_passo(1);
      }
      break;
    }

    case 1: {// Girar o robo para um lado que não tenha parede

      if(distancia_direita > distancia_limite){
        ligar_motores(0, 1);
        
        lado_parede_e_direita = true;
        
      } else {
        ligar_motores(1, 0);

        lado_parede_e_direita = false;
      }
      delay(1800);
      ligar_motores(0,0);

      comeco_de_linha = true;

      mudar_passo(2);

      break;
    }

    case 2: {

      if(comeco_de_linha){
        comeco_de_linha = false;

        if(lado_parede_e_direita){
          distancia_inicial_parede = distancia_direita;
        }else {
          distancia_inicial_parede = distancia_esquerda;
        }
      }


      if(sentido == 1){
        pode_andar = distancia_frontal > distancia_limite;
      }else {
        pode_andar = distancia_traseira > distancia_limite;
      }


      if(pode_andar){

        if(lado_parede_e_direita){
          variacao_de_distancia_da_parede = distancia_direita - distancia_inicial_parede;
        }else {
          variacao_de_distancia_da_parede = distancia_esquerda - distancia_inicial_parede;
        }

        if(abs(variacao_de_distancia_da_parede) > 5 && abs(variacao_de_distancia_da_parede) < 30){


          // Freia o robo e espera um segundo
          if(sentido == 1){
            ligar_motores(1, 1);
          } else {
            ligar_motores(-1, -1);
          }
          delay(50);
          ligar_motores(0,0);
          delay(1000);

          if(variacao_de_distancia_da_parede > 0){

            if(lado_parede_e_direita){
              if(sentido == 1){
                ligar_motores(1, 0);
              }else {
                ligar_motores(-1, 0);
              }
            }else {
              if(sentido == 1){
                ligar_motores(0, 1);
              }else {
                ligar_motores(0, -1);
              }
            }

          }else {

            if(lado_parede_e_direita){
              if(sentido == 1){
                ligar_motores(0, 1);
              }else {
                ligar_motores(0, -1);
              }
            }else {
              if(sentido == 1){
                ligar_motores(1, 0);
              }else {
                ligar_motores(-1, 0);
              }
            }
          }

          if(millis() - momento_da_ultima_correcao > 5000){
            delay(400);
          }else if (millis() - momento_da_ultima_correcao > 300){
            delay(600);
          } else {
            delay(800);
          }

          ligar_motores(0, 0);

          delay(1000);
          momento_da_ultima_correcao = millis();
          comeco_de_linha = true;
          corrigiu = true;

        } else { //anda
          if(sentido == 1){
            ligar_motores(-1, -1);
          } else {
            ligar_motores(1, 1);
          }
        }

      } else {


        if(sentido == 1){
          ligar_motores(1, 1);
        } else {
          ligar_motores(-1, -1);
        }

        delay(50);
        ligar_motores(0,0);


        if(primeira_linha){

          primeira_linha = false;
          inverter_sentido();
        } else { 

          if(corrigiu) {
            delay(2000);
            inverter_sentido();
            corrigiu = false;
          } else {
            mudar_passo(3);
          }
        }

      }


      break;
    }
    
    default:
      ligar_motores(0,0);
  }


}

void mudar_passo(int proximo_passo){
  ligar_motores(0,0);
  delay(2000); 
  passo = proximo_passo;
}

void ligar_motores(int direita, int esquerda){


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

void inverter_sentido(){

  if(sentido == 1){
    sentido = -1;
  } else {
    sentido = 1;
  }
}
