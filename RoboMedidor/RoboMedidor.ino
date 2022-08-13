#include <Arduino.h>
#include <Ultrasonic.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Motores:
#define motor_direito_horario 5
#define motor_direito_anti 4
#define motor_esquerdo_horario 7
#define motor_esquerdo_anti 6

#define PINO_VELOCIDADE_MOTORES 10

// Gerais
#define DISTANCIA_LIMITE 20 // CM
int velocidade_motores = 255;
int passo = 0;
int sentido = 1;
bool pode_andar = false;
bool medindo = false;
String modo = String("desativado");

//Calibragem
#define PINO_BOTAO 11
byte valor_botao;
byte valor_anterior_botao;

volatile int pulsos_direita = 0; 
volatile int pulsos_esquerda = 0;

// LCD

String primeira_linha_atual_lcd= String("");
String segunda_linha_atual_lcd= String("");

// Sensores de distancia:
#define SENSOR_FRONTAL_ECHO A2
#define SENSOR_FRONTAL_TRIG A3

#define SENSOR_DIREITA_ECHO A0
#define SENSOR_DIREITA_TRIG A1

#define SENSOR_ESQUERDO_ECHO 12
#define SENSOR_ESQUERDO_TRIG 13 

#define SENSOR_TRASEIRO_ECHO 9
#define SENSOR_TRASEIRO_TRIG 8

Ultrasonic ultrassonico_frontal(SENSOR_FRONTAL_TRIG, SENSOR_FRONTAL_ECHO);
Ultrasonic ultrassonico_direito(SENSOR_DIREITA_TRIG, SENSOR_DIREITA_ECHO);
Ultrasonic ultrassonico_esquerdo(SENSOR_ESQUERDO_TRIG, SENSOR_ESQUERDO_ECHO);
Ultrasonic ultrassonico_traseiro(SENSOR_TRASEIRO_TRIG, SENSOR_TRASEIRO_ECHO);

int distancia_frontal = 0;
int distancia_direita = 0;
int distancia_esquerda = 0;
int distancia_traseira = 0;

// LCD
LiquidCrystal_I2C lcd(0x27,20,4);

// Curva e alinhamento:

bool primeira_vez_apos_correcao = false;
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
  pinMode(PINO_VELOCIDADE_MOTORES, OUTPUT);

  pinMode(PINO_BOTAO, INPUT);

  analogWrite(PINO_VELOCIDADE_MOTORES, velocidade_motores);

  lcd.init();
  lcd.backlight();

}


void loop(){

  valor_botao = digitalRead(PINO_BOTAO);

  if(valor_anterior_botao != valor_botao){
    if(valor_botao == 1){

      ligar_motores(0,0);

      if(modo == "medindo"){
        modo = String("calibrando");
      }else if(modo == "calibrando") {
        modo = String("desativado");
      } else if(modo == "desativado"){
        modo = String("medindo");
      }

    }
    valor_anterior_botao = valor_botao;
  }

  if(modo == "calibrando"){
    
    escrever_lcd("Calibrando", "");
    
  } else if(modo == "medindo") {

    escrever_lcd("Medindo", "");
    distancia_frontal = ultrassonico_frontal.Ranging(CM);
    distancia_direita = ultrassonico_direito.Ranging(CM);
    distancia_esquerda = ultrassonico_esquerdo.Ranging(CM);
    distancia_traseira = ultrassonico_traseiro.Ranging(CM);

    switch(passo){

      case 0: { //Procura a parede na frente do roboz

        if(distancia_frontal >= DISTANCIA_LIMITE){
          ligar_motores(-1, -1);
        } else {

          ligar_motores(1, 1);
          delay(75);
          ligar_motores(0, 0);

          mudar_passo(1);
        }
        break;
      }

      case 1: {

        ligar_motores(-1, 1);
          
        delay(800);
        ligar_motores(0,0);

        primeira_vez_apos_correcao = true;

        mudar_passo(2);

        break;
      }

      case 2: {

        if(primeira_vez_apos_correcao){

          primeira_vez_apos_correcao = false;
          distancia_inicial_parede = distancia_direita;

        }

        if(sentido == 1){
          pode_andar = distancia_frontal > DISTANCIA_LIMITE;
        }else {
          pode_andar = distancia_traseira > DISTANCIA_LIMITE;
        }

        if(pode_andar){

          variacao_de_distancia_da_parede = distancia_direita - distancia_inicial_parede;

          if(abs(variacao_de_distancia_da_parede) > 5 && abs(variacao_de_distancia_da_parede) < 30){

            // Freia o robo e espera um segundo
            if(sentido == 1){
              ligar_motores(1, 1);
            } else {
              ligar_motores(-1, -1);
            }
            delay(75);
            ligar_motores(0,0);
            delay(2000);

            if(variacao_de_distancia_da_parede > 0){

              if(sentido == 1){
                ligar_motores(0, -1);
              }else {
                ligar_motores(0, 1);
              }

            }else {

              if(sentido == 1){
                ligar_motores(-1, 1);
              }else {
                ligar_motores(1, -1);
              }

            }

            if(millis() - momento_da_ultima_correcao > 5000){
              delay(200);
            }else if (millis() - momento_da_ultima_correcao > 4000){
              delay(225);
            }else if (millis() - momento_da_ultima_correcao > 3000){
              delay(250);
            }else if (millis() - momento_da_ultima_correcao > 2000){
              delay(275);
            } else {
              delay(300);
            }

            ligar_motores(0, 0);

            delay(1000);
            momento_da_ultima_correcao = millis();
            primeira_vez_apos_correcao = true;
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

          delay(75);
          ligar_motores(0,0);

          delay(2000);

          primeira_vez_apos_correcao = true;

          if(primeira_linha){

            primeira_linha = false;
            inverter_sentido();

            iniciar_medicao();
          } else { 

            if(corrigiu) {

              inverter_sentido();
              corrigiu = false;
              iniciar_medicao();
              
            } else {

              desligar_medicao();

              delay(5000);

              mudar_passo(0);
            }
          }

        }

        break;

      }
      
      default:
        ligar_motores(0,0);
    }

  } else if(modo == "desativado"){
    escrever_lcd("Desativado", "");
  }

}

void escrever_lcd(String linha_1, String linha_2){

  if(!linha_1.equalsIgnoreCase(primeira_linha_atual_lcd)){
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,0);
    lcd.print(linha_1);

    primeira_linha_atual_lcd = linha_1;
  }

  if(!linha_2.equalsIgnoreCase(segunda_linha_atual_lcd)){
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(linha_2);

    segunda_linha_atual_lcd = linha_2;
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

void iniciar_medicao(){

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