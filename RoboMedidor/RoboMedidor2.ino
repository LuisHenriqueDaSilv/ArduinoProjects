
/*
---------------------Bibliotecas---------------------
*/
#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>


/*
---------------------Constantes Gerais---------------------
*/
#define DISTANCIA_MINIMA_DA_PAREDE 15 // CM
#define PULSOS_PARA_CURVA 18


/*
---------------------Variaveis Gerais---------------------
*/

String modo = String("desativado");
String ultimo_modo_loop;

// ---Calibrador---
float distancia_total_percorrida = 0;
float pulsos_totais = 0;
int distancia_inicial_parede = 0;
int contador_calibrador = 0;
float cms_por_pulso = 0;


// ---Medicao---
long momento_do_ultimo_pulso = 0;
volatile int pulsos_encoder = 0;
int passo = 0;
bool medindo = false;
int distancia_direita_inicio = 0;
int indo_para_frente = true;
bool corrigiu = false;
bool virando = false;
long inicio_da_reta = 0;
int pulsos_para_correcao = 0;


/*
---------------------Sensores De Distancia---------------------
*/
#define TRIG_FRONTAL 8
#define ECHO_FRONTAL 9
#define TRIG_TRASEIRO A3
#define ECHO_TRASEIRO A2
#define TRIG_DIREITO A1
#define ECHO_DIREITO A0

int medir_distancia(int echoPin, int trigPin){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;
  return(distance);
}


/*
---------------------Painel LCD---------------------
*/
LiquidCrystal_I2C lcd(0x27,20,4);

String primeira_linha_atual_lcd = String("");
String segunda_linha_atual_lcd = String("");

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


/*
-----------------------------Controle Dos Motores-----------------------------
*/
void ligar_motores(int esquerda, int direita){

  if(esquerda == 0 && direita == 0){
    analogWrite(10, 0);
  } else {
    analogWrite(10, 255);
  }

  switch(esquerda){

    case -1:
	    digitalWrite(6, LOW); // Esquerda horario
	    digitalWrite(7, HIGH); // Esquerda anti
      break;
    case 1: 
	    digitalWrite(6, HIGH); // Esquerda horario
	    digitalWrite(7, LOW); // Esquerda anti
      break;
    default: 
      digitalWrite(6, LOW); //Esquerda horario
	    digitalWrite(7, LOW); // Esquerda anti
  }

  switch(direita){

    case -1: 
	    digitalWrite(4, LOW); // Direita horario
	    digitalWrite(5, HIGH); // Direita anti
      break;
    case  1: 
	    digitalWrite(4, HIGH); // Direita horario
	    digitalWrite(5, LOW); // Direita anti
      break;
    default: 
      digitalWrite(4, LOW); // Direita horario
	    digitalWrite(5, LOW); // Direita anti

  }

}

void frear_motores(int direcao_atual_motores){
  ligar_motores(direcao_atual_motores*-1, direcao_atual_motores*-1);
  delay(100);
  ligar_motores(0,0);

}


/*
-----------------------------Armazenamento Local-----------------------------
*/
void gravar_eeprom(float x){
  EEPROM.write(0,int(x));
  EEPROM.write (0+1,int((x-int(x))*100));
}

float ler_eeprom(){
  return float(EEPROM.read(0))+ float(EEPROM.read(0+1))/100;
}


/*
-----------------------------Sensor Encoder-----------------------------
*/
void ligar_medicao(){
  pulsos_encoder = 0;
  medindo = true;
  attachInterrupt(digitalPinToInterrupt(2), contar_pulsos, RISING);
}

void desligar_medicao(){
  medindo = false;
  detachInterrupt(digitalPinToInterrupt(2));
}

void contar_pulsos(){

  if(millis() - momento_do_ultimo_pulso > 20){
    pulsos_encoder++; 
    momento_do_ultimo_pulso = millis();
  }
}


/*
-----------------------------Botão de Controle-----------------------------
*/
void mudar_modo(){


  if(millis() - momento_do_ultimo_pulso > 5){

    if(modo == "medindo"){
      modo = String("calibrando");
    }else if(modo == "calibrando") {
      modo = String("linhareta");
    } else if(modo == "linhareta"){
      modo = String("desativado");
    } else if(modo == "desativado"){
      modo = String("medindo");
    }

    momento_do_ultimo_pulso = millis();
  }


}


/*
-----------------------------Controle Arduino-----------------------------
*/

void reiniciar_robo(){

  escrever_lcd("", "");
  desligar_medicao();
  medindo = false;
  passo = 0;
  pulsos_encoder = 0;
  distancia_direita_inicio = 0;
  indo_para_frente = true;
  corrigiu = false;
  virando = false;
  inicio_da_reta = 0;
  pulsos_para_correcao = 0;
  distancia_total_percorrida = 0;
  pulsos_totais = 0;
  distancia_inicial_parede = 0;
  contador_calibrador = 0;

  cms_por_pulso = ler_eeprom();

}

void setup(){

  Serial.begin(9600);

  // ---Pinos dos Sensores Ultrassonicos---
  pinMode(ECHO_DIREITO, INPUT);
  pinMode(ECHO_TRASEIRO, INPUT);
  pinMode(ECHO_FRONTAL, INPUT);

  pinMode(TRIG_DIREITO,OUTPUT);
  pinMode(TRIG_TRASEIRO,OUTPUT);
  pinMode(TRIG_FRONTAL,OUTPUT);


  // ---Pinos de Controle Dos Motores---
  pinMode(4, OUTPUT); // Direita Horario
  pinMode(5, OUTPUT); // Direita Anti-Horario
  pinMode(6, OUTPUT); // Esquerda Horario
  pinMode(7, OUTPUT); // Esquerda Anti-Horario
  pinMode(10, OUTPUT); // Pino Para Controle de Velocidade Dos Motores

  // ---Iniciando LCD---
  lcd.init();
  lcd.backlight();

  // ---Mensagem de inicio---
  escrever_lcd("Iniciando robo", "Aguarde!");
  delay(2000);

  // ---Sensores---
  pinMode(2, INPUT_PULLUP); // Pino do Sensor Encoder
  pinMode(3, INPUT_PULLUP); // Pino do Botão de Controle de Modos
  attachInterrupt(digitalPinToInterrupt(3), mudar_modo, FALLING); // Pino do Botão de Controle de Modos

  reiniciar_robo();
}

void loop(){

  if(modo != ultimo_modo_loop){
    ultimo_modo_loop = modo;
    ligar_motores(0,0);
    reiniciar_robo();
    delay(500);
  }

  if(modo == "desativado"){

    escrever_lcd("Desativado", "");

  } else if(modo == "calibrando") {

    escrever_lcd("Calibrando", "");

    if(contador_calibrador < 30){

      bool pode_andar = false;

      if(indo_para_frente){
        pode_andar = medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO) < 50;
      } else {
        pode_andar = medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO) > DISTANCIA_MINIMA_DA_PAREDE;
      }

      if(indo_para_frente){
        if(pode_andar){
          ligar_motores(-1,-1);
        } else {
          frear_motores(-1);
        }
      } else {
        if(pode_andar){
          ligar_motores(1,1);
        } else {
          frear_motores(1);
        }
      }

      if(!pode_andar){
        desligar_medicao();

        distancia_total_percorrida = distancia_total_percorrida + abs(medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO) - distancia_inicial_parede);
        pulsos_totais = pulsos_totais + pulsos_encoder;
        contador_calibrador++;

        delay(1000);
        indo_para_frente = !indo_para_frente;
        distancia_inicial_parede = medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO);
        ligar_medicao();
      }

    } else {
      ligar_motores(0,0);
      float media_de_cms_por_pulso = distancia_total_percorrida/pulsos_totais;
      escrever_lcd(String(media_de_cms_por_pulso), "");
      gravar_eeprom(media_de_cms_por_pulso);

      delay(10000);
      modo = String("desativado");
    }

  } else if(modo == "medindo"){

    escrever_lcd("Medindo", "");

    switch(passo){

      case 0: {

        if(medir_distancia(ECHO_FRONTAL, TRIG_FRONTAL) > DISTANCIA_MINIMA_DA_PAREDE){ // Andar até encontrar a parede
          ligar_motores(-1,-1);
        } else {
          frear_motores(-1);

          delay(1000);
          passo++;
        }

        break;
      }

      case 1: { // Voltar até ficar com 40cm da parede

        if(medir_distancia(ECHO_FRONTAL, TRIG_FRONTAL) < 40){
          ligar_motores(1, 1);
        } else {
          frear_motores(1);
          passo++;
          delay(1000);
        }
        break;
      }

      case 2: { // Fazer curva

        if(!medindo){
          ligar_medicao();
        }

        if(pulsos_encoder < PULSOS_PARA_CURVA){
          ligar_motores(-1, 1);
        } else {
          ligar_motores(0,0);
          desligar_medicao();

          distancia_direita_inicio = medir_distancia(ECHO_DIREITO, TRIG_DIREITO);
          passo++;
          delay(1000);
        }

        break;
      }

      case 3: { //Seguir a parede ao lado

        if(inicio_da_reta == 0){ 
          inicio_da_reta = millis();
        }

        if(virando){

          if(pulsos_encoder >= pulsos_para_correcao){
            virando = false;
            distancia_direita_inicio = medir_distancia(ECHO_DIREITO, TRIG_DIREITO);
            desligar_medicao();
            ligar_motores(0,0);
            inicio_da_reta = millis();
            delay(1000);
          }
          return;
        } else {

          bool pode_andar;

          if(indo_para_frente){
            pode_andar = medir_distancia(ECHO_FRONTAL, TRIG_FRONTAL) >= DISTANCIA_MINIMA_DA_PAREDE;
          } else {
            pode_andar = medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO) >= DISTANCIA_MINIMA_DA_PAREDE;
          }

          if(pode_andar){
            int variacao_lateral = medir_distancia(ECHO_DIREITO, TRIG_DIREITO) - distancia_direita_inicio;

            if(abs(variacao_lateral) > 7 && abs(variacao_lateral) < 20){


              if(indo_para_frente){
                frear_motores(-1);
              } else {
                frear_motores(1);
              }
              delay(1000);

              virando = true;
              corrigiu = true;
              ligar_medicao();

              long momento_atual = millis();

              if(momento_atual - inicio_da_reta < 1000){
                pulsos_para_correcao = 6;
              } else if(momento_atual - inicio_da_reta < 2000){
                pulsos_para_correcao = 5;
              } else if(momento_atual - inicio_da_reta < 3000){
                pulsos_para_correcao = 4;
              } else if(momento_atual - inicio_da_reta < 4000){
                pulsos_para_correcao = 3;
              } else {
                pulsos_para_correcao = 2;
              }
              
              if((indo_para_frente && variacao_lateral > 0) || (!indo_para_frente && variacao_lateral < 0)) {
                ligar_motores(1, -1);
              } else {
                ligar_motores(-1, 1);
              }

            } else {

              if(indo_para_frente){
                ligar_motores(-1,-1);
              } else {
                ligar_motores(1,1);
              }

            }
          } else {

              if(indo_para_frente){
                frear_motores(-1);
              } else {
                frear_motores(1);
              }

              delay(1000);

              if(!corrigiu){
                passo++;
              } else {
                indo_para_frente = !indo_para_frente;
                distancia_direita_inicio = medir_distancia(ECHO_DIREITO, TRIG_DIREITO);
                corrigiu = false;
              }


          }

        }

        break;
        
      }

      default: {
        ligar_motores(0,0);
      }
    }
  } else if(modo == "linhareta"){

    escrever_lcd("Linha reta", "");

    switch(passo){

      case 0: {
        if(medir_distancia(ECHO_FRONTAL, TRIG_FRONTAL) > DISTANCIA_MINIMA_DA_PAREDE){ // Andar até encontrar a parede
          ligar_motores(-1,-1);
        } else {
          frear_motores(-1);

          distancia_inicial_parede = medir_distancia(ECHO_FRONTAL, TRIG_FRONTAL);

          delay(1000);
          passo++;
        }

        break;
      }

      case 1: {
        if(!medindo){
          ligar_medicao();
        }

        if(medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO) > DISTANCIA_MINIMA_DA_PAREDE){
          ligar_motores(1,1);
        } else {
          frear_motores(1);
          int tamanho = (pulsos_encoder*cms_por_pulso) + distancia_inicial_parede + medir_distancia(ECHO_TRASEIRO, TRIG_TRASEIRO) + 27; 
          escrever_lcd(String(pulsos_encoder), String(tamanho) + String("CM"));
          delay(10000);

          reiniciar_robo();
          modo = String("desativado");
        }
        break;
      }

    }

  }


}
