#include <Arduino.h>
#include <Ultrasonic.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Motores:
#define motor_direito_horario 5
#define motor_direito_anti 4
#define motor_esquerdo_horario 7
#define motor_esquerdo_anti 6

#define velocidade_motor_direito 10
#define velocidade_motor_esquerdo 11

// Gerais
int passo = 0;
int distancia_limite = 30; // CM
int sentido = 1;
bool pode_andar = false;
int velocidade_motores = 155;
bool medindo = false;

volatile int pulsos_direita = 0; 
volatile int pulsos_esquerda = 0; 

// Sensores de distancia:
#define sensor_frontal_echo A2
#define sensor_frontal_trig A3

#define sensor_direito_echo A0
#define sensor_direito_trig A1

#define sensor_esquerdo_echo 12
#define sensor_esquerdo_trig 13 

#define sensor_traseiro_echo 9
#define sensor_traseiro_trig 8

Ultrasonic ultrassonico_frontal(sensor_frontal_trig, sensor_frontal_echo);
Ultrasonic ultrassonico_direito(sensor_direito_trig, sensor_direito_echo);
Ultrasonic ultrassonico_esquerdo(sensor_esquerdo_trig, sensor_esquerdo_echo);
Ultrasonic ultrassonico_traseiro(sensor_traseiro_trig, sensor_traseiro_echo);

int distancia_frontal = 0;
int distancia_direita = 0;
int distancia_esquerda = 0;
int distancia_traseira = 0;


// LCD
LiquidCrystal_I2C lcd(0x27,20,4);


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
  pinMode(velocidade_motor_direito, OUTPUT);
  pinMode(velocidade_motor_esquerdo, OUTPUT);

  analogWrite(velocidade_motor_direito, velocidade_motores);
  analogWrite(velocidade_motor_esquerdo, velocidade_motores);


  lcd.init();
  lcd.backlight();

}

void loop(){

  distancia_frontal = ultrassonico_frontal.Ranging(CM);
  distancia_direita = ultrassonico_direito.Ranging(CM);
  distancia_esquerda = ultrassonico_esquerdo.Ranging(CM);
  distancia_traseira = ultrassonico_traseiro.Ranging(CM);

  switch(passo){

    case 0: { //Procura a parede na frente do roboz

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
        ligar_motores(-1, 1);
        
        lado_parede_e_direita = true;
        
      } else {
        ligar_motores(1, -1);

        lado_parede_e_direita = false;
      }
      delay(700);
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

      if(!medindo && !corrigiu){
        iniciar_medida();
      }


      if(sentido == 1){
        pode_andar = distancia_frontal > distancia_limite;
      }else {
        pode_andar = distancia_traseira > distancia_limite;
      }

        if(medindo){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Medindo");
        }else {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Não medindo");
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
                ligar_motores(0, -1);
              }else {
                ligar_motores(0, 1);
              }

            }else {
              if(sentido == 1){
                ligar_motores(-1, 0);
              }else {
                ligar_motores(1, 0);
              }
            }

          }else {

            if(lado_parede_e_direita){
              if(sentido == 1){
                ligar_motores(-1, 1);
              }else {
                ligar_motores(1, -1);
              }
            }else {
              if(sentido == 1){
                ligar_motores(1, -1);
              }else {
                ligar_motores(-1, 1);
              }
            }
          }

          if(millis() - momento_da_ultima_correcao > 5000){
            delay(100);
          }else if (millis() - momento_da_ultima_correcao > 300){
            delay(200);
          } else {
            delay(300);
          }

          ligar_motores(0, 0);

          delay(1000);
          momento_da_ultima_correcao = millis();
          comeco_de_linha = true;
          corrigiu = true;
          desligar_medicao();


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

        delay(2000);


        if(primeira_linha){

          primeira_linha = false;
          inverter_sentido();
        } else { 

          if(corrigiu) {
            delay(2000);
            inverter_sentido();
            corrigiu = false;
            pulsos_direita = 0;
            pulsos_esquerda = 0;
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


void iniciar_medida(){

  medindo = true;

  pulsos_direita = 0;
  pulsos_esquerda = 0;

  attachInterrupt(digitalPinToInterrupt(3), interrupcao_direita, FALLING);
  attachInterrupt(digitalPinToInterrupt(2), interrupcao_esquerda, FALLING);
}

void interrupcao_direita(){
  pulsos_direita++;
}

void interrupcao_esquerda(){
  pulsos_esquerda++;
}

void desligar_medicao(){

  medindo = false;
  detachInterrupt(digitalPinToInterrupt(3));
  detachInterrupt(digitalPinToInterrupt(2));
}