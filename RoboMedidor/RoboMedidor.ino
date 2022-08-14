#include <Arduino.h>
#include <Ultrasonic.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Gerais
#define DISTANCIA_LIMITE 20 // CM
byte passo = 0;
bool indo_para_frente = true;
String modo = String("desativado");
byte valor_anterior_botao_troca_de_modo;

volatile int pulsos_direita = 0; 
volatile int pulsos_esquerda = 0;

// LCD
LiquidCrystal_I2C lcd(0x27,20,4);

String primeira_linha_atual_lcd = String("");
String segunda_linha_atual_lcd = String("");

// Sensores de distancia:
Ultrasonic ultrassonico_frontal(A3, A2); //Trig-Echo
Ultrasonic ultrassonico_direito(A1, A0); //Trig-Echo
Ultrasonic ultrassonico_esquerdo(13, 12); //Trig-Echo
Ultrasonic ultrassonico_traseiro(8, 9); //Trig-Echo

// Curva e alinhamento:
bool primeira_vez_apos_correcao = false;
int distancia_inicial_parede = 0;
int momento_da_ultima_correcao = 0; 
bool corrigiu = false;
bool primeira_linha = true;

void setup(){

  pinMode(5, OUTPUT); // Direito horario
  pinMode(4, OUTPUT); // Direito anti
  pinMode(7, OUTPUT); // Esquerdo Horario
  pinMode(6, OUTPUT); // Esquero anti
  pinMode(10, OUTPUT); // Pino de velocidade dos motores
  pinMode(11, INPUT); // Pino do botão para troca de modo

  analogWrite(10, 255); // Pino de velocidade dos motores

  // LCD
  lcd.init();
  lcd.backlight();
}

void loop(){

  byte valor_atual_botao_troca_de_modo = digitalRead(11); // Pino do botão para troca de modo
  if(valor_anterior_botao_troca_de_modo != valor_atual_botao_troca_de_modo){
    if(valor_atual_botao_troca_de_modo == 1){

      ligar_motores(0,0);

      if(modo == "medindo"){
        modo = String("calibrando");
      }else if(modo == "calibrando") {
        modo = String("desativado");
      } else if(modo == "desativado"){
        modo = String("medindo");
      }

    }
    valor_anterior_botao_troca_de_modo = valor_atual_botao_troca_de_modo;
  }

  if(modo == "calibrando"){
    
    escrever_lcd("Calibrando", "");
    
  } else if(modo == "medindo") {

    escrever_lcd("Medindo", "");

    int distancia_frontal = ultrassonico_frontal.Ranging(CM);
    int distancia_direita = ultrassonico_direito.Ranging(CM);
    int distancia_esquerda = ultrassonico_esquerdo.Ranging(CM);
    int distancia_traseira = ultrassonico_traseiro.Ranging(CM);

    switch(passo){

      case 0: {

        if(distancia_frontal >= DISTANCIA_LIMITE){
          ligar_motores(-1, -1);
        } else {

          ligar_motores(1, 1);
          delay(75);
          ligar_motores(0, 0);

          passo = 1;
          delay(2000);
        }
        break;
      }

      case 1: {

        ligar_motores(-1, 1);
          
        delay(800);
        ligar_motores(0,0);

        primeira_vez_apos_correcao = true;

        passo = 2;
        delay(2000);

        break;
      }

      case 2: {

        if(primeira_vez_apos_correcao){

          primeira_vez_apos_correcao = false;
          distancia_inicial_parede = distancia_direita;

        }

        bool pode_andar = false;

        if(indo_para_frente){
          pode_andar = distancia_frontal > DISTANCIA_LIMITE;
        }else {
          pode_andar = distancia_traseira > DISTANCIA_LIMITE;
        }

        if(pode_andar){

          int variacao_de_distancia_da_parede = distancia_direita - distancia_inicial_parede;

          if(abs(variacao_de_distancia_da_parede) > 5 && abs(variacao_de_distancia_da_parede) < 30){

            // Freia o robo e espera um segundo
            if(indo_para_frente){
              ligar_motores(1, 1);
            } else {
              ligar_motores(-1, -1);
            }
            delay(75);
            ligar_motores(0,0);
            delay(2000);

            if(variacao_de_distancia_da_parede > 0){

              if(indo_para_frente){
                ligar_motores(0, -1);
              }else {
                ligar_motores(0, 1);
              }

            }else {

              if(indo_para_frente){
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
            if(indo_para_frente){
              ligar_motores(-1, -1);
            } else {
              ligar_motores(1, 1);
            }
          }

        } else {


          if(indo_para_frente){
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
            indo_para_frente = !indo_para_frente;

          } else { 

            if(corrigiu) {

              indo_para_frente = !indo_para_frente;
              corrigiu = false;
              
            } else {


              mudar_passo(0);
              delay(2000);
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


void reiniciar_robo(){

  desligar_medicao();
  pulsos_esquerda = 0;
  pulsos_direita = 0;

  primeira_vez_apos_correcao = false;
  distancia_inicial_parede  = 0;
  momento_da_ultima_correcao = 0;
  corrigiu = false;
  primeira_linha = true;
  passo = 0;
  indo_para_frente = true;
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

void mudar_passo(byte proximo_passo){
  ligar_motores(0,0);
  delay(2000); 
  passo = proximo_passo;
}

void ligar_motores(int direita, int esquerda){

  switch(direita){

    case -1:
	    digitalWrite(5, LOW); // Direito horario
	    digitalWrite(4, HIGH); // Direito anti
      break;
    case 1: 
	    digitalWrite(5, HIGH); // Direito horario
	    digitalWrite(4, LOW); // Direito anti
      break;
    default: 
      digitalWrite(5, LOW); //Direito horario
	    digitalWrite(4, LOW); // Direito anti
  }

  switch(esquerda){

    case -1: 
	    digitalWrite(7, LOW); // Esquerdo horario
	    digitalWrite(6, HIGH); // Esquerdo anti
      break;
    case  1: 
	    digitalWrite(7, HIGH); // Esquerdo horario
	    digitalWrite(6, LOW); // Esquerdo anti
      break;
    default: 
      digitalWrite(7, LOW); // Esquerdo horario
	    digitalWrite(6, LOW); // Esquerdo anti

  }

}


void iniciar_medicao(){


  pulsos_direita = 0;
  pulsos_esquerda = 0;

  attachInterrupt(digitalPinToInterrupt(3), interrupcao_direita, FALLING);
  attachInterrupt(digitalPinToInterrupt(2), interrupcao_esquerda, FALLING);
}

void interrupcao_direita(){
  pulsos_direita++;
};

void interrupcao_esquerda(){
  pulsos_esquerda++;
};

void desligar_medicao(){

  detachInterrupt(digitalPinToInterrupt(3));
  detachInterrupt(digitalPinToInterrupt(2));
}

