
/*
---------------------Bibliotecas---------------------
*/
#include <Arduino.h>
#include <Ultrasonic.h>
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

// ---Medicao---
long momento_do_ultimo_pulso = 0;
volatile long pulsos_encoder = 0;
int passo = 0;
bool medindo = false;



/*
---------------------Sensores De Distancia---------------------
*/
Ultrasonic ultrassonico_frontal(8, 9); //Trig-Echo
Ultrasonic ultrassonico_direito(A1, A0); //Trig-Echo
Ultrasonic ultrassonico_traseiro(A3, A2); //Trig-Echo


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
void ligar_motores(int direita, int esquerda){

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

void frear_motores(int direcao){
  ligar_motores(direcao*-1, direcao*-1);
  delay(100);
  ligar_motores(0,0);

}


/*
-----------------------------Armazenamento Local-----------------------------
*/
/*
void gravar_eeprom(float x){
  EEPROM.write(0,int(x));
  EEPROM.write (0+1,int((x-int(x))*100));
}

float ler_eeprom(){
  return float(EEPROM.read(0))+ float(EEPROM.read(0+1))/100;
}
*/


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

  if(millis() - momento_do_ultimo_pulso > 5){
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

  medindo = false;
  passo = 0;
  pulsos_encoder = 0;
}

void setup(){

  // ---Pinos de Controle Dos Motores---
  pinMode(4, OUTPUT); // Direita Horario
  pinMode(5, OUTPUT); // Direita Anti-Horario
  pinMode(6, OUTPUT); // Esquerda Horario
  pinMode(7, OUTPUT); // Esquerda Anti-Horario
  pinMode(10, OUTPUT); // Pino Para Controle de Velocidade Dos Motores

  // ---Sensores---
  pinMode(2, INPUT_PULLUP); // Pino do Sensor Encoder
  attachInterrupt(digitalPinToInterrupt(3), mudar_modo, FALLING); // Pino do Botão de Controle de Modos

  // ---Iniciando LCD---
  lcd.init();
  lcd.backlight();

  // ---Mensagem de inicio---
  escrever_lcd("Iniciando robo", "Aguarde!");
  delay(2000);

  reiniciar_robo();
}

void loop(){
  
  int distancia_frontal = ultrassonico_frontal.Ranging(CM);
  int distancia_direita = ultrassonico_direito.Ranging(CM);
  int distancia_traseira = ultrassonico_traseiro.Ranging(CM);

  switch(passo){

    case 0: {

      if(distancia_frontal > DISTANCIA_MINIMA_DA_PAREDE){ // Andar até encontrar a parede
        ligar_motores(-1,-1);
      } else {
        frear_motores(-1);

        delay(1000);
        passo++;
      }

      break;
    }

    case 1: { // Voltar até ficar com 40cm da parede

      if(distancia_frontal < 40){
        ligar_motores(1, 1);
      } else {
        frear_motores(1);

        delay(1000);
        passo++;
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
        passo++;
      }
      
      break;
    }

    case 3: { //Seguir a parede ao lado

      
    }

    default: {
      ligar_motores(0,0);
    }
  }


}
