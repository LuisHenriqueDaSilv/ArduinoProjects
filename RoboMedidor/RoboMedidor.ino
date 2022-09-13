
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
#define DISTANCIA_MINIMA_DA_PAREDE 10 // CM
#define PULSOS_PARA_CURVA 17
const int  PULSOS_PARA_CORRECAO = 1;


/*
---------------------Variaveis Gerais---------------------
*/

String modo = String("Em espera");
String ultimo_modo_loop;
long momento_do_ultimo_pulso_troca_de_modo = 0;
long momento_da_ultima_troca_de_modo = 0;

// ---Calibrador---
float distancia_total_percorrida = 0;
float pulsos_totais = 0;
int distancia_inicial_parede = 0;
int contador_calibrador = 0;
int cms_por_pulso = 0;


// ---Medicao---
long momento_do_ultimo_pulso = 0;
volatile int pulsos_encoder = 0;
int passo = 0;
bool medindo = false;
int distancia_direita_inicio = 0;
int indo_para_frente = true;
bool virando = false;

int primeira_reta = 0;
int segunda_reta = 0;
int pulsos_inicio_de_correcao = 0;
bool procurando_canto = true;
int contador_de_correcoes = 0;


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
void ligar_motores(int esquerda, int direita){

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

  if(millis() - momento_do_ultimo_pulso > 5){
    pulsos_encoder++; 
    momento_do_ultimo_pulso = millis();
  }
}


/*
-----------------------------Botão de Controle-----------------------------
*/
void mudar_modo(){

  if(millis() - momento_do_ultimo_pulso_troca_de_modo > 2000){

    if(modo == "Autonomo"){
      modo = String("Linha Reta");
    }else if(modo == "Linha Reta") {
      modo = String("Calibrando");
    } else if(modo == "Calibrando"){
      modo = String("Em espera");
    } else if(modo == "Em espera"){
      modo = String("Autonomo");
    }

    momento_do_ultimo_pulso_troca_de_modo = millis();
  }


}


/*
-----------------------------Controle Arduino-----------------------------
*/

void reiniciar_robo(){

  desligar_medicao();
  ligar_motores(0,0);
  medindo = false;
  passo = 0;
  pulsos_encoder = 0;
  distancia_direita_inicio = 0;
  indo_para_frente = true;
  virando = false;
  contador_de_correcoes = 0;
  distancia_total_percorrida = 0;
  pulsos_totais = 0;
  distancia_inicial_parede = 0;
  contador_calibrador = 0;
  momento_do_ultimo_pulso_troca_de_modo = 0;
  pulsos_inicio_de_correcao = 0;
  procurando_canto = true;

  cms_por_pulso = ler_eeprom();

}

void setup(){

  Serial.begin(9600);

  // ---Pinos de Controle Dos Motores---
  pinMode(4, OUTPUT); // Direita Horario
  pinMode(5, OUTPUT); // Direita Anti-Horario
  pinMode(6, OUTPUT); // Esquerda Horario
  pinMode(7, OUTPUT); // Esquerda Anti-Horario

  // ---Iniciando LCD---
  lcd.init();
  lcd.backlight();

  // ---Mensagem de inicio---
  escrever_lcd("Iniciando Robo", "Aguarde!");
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
    escrever_lcd(modo, "Carregando...");
    ligar_motores(0,0);
    reiniciar_robo();
    momento_da_ultima_troca_de_modo = millis();
  }

  if(millis() - momento_da_ultima_troca_de_modo > 2000){

    escrever_lcd(modo, "");

    if(modo == "Calibrando") {

      if(contador_calibrador < 20){

        bool pode_andar = false;

        if(indo_para_frente){
          pode_andar = ultrassonico_frontal.Ranging(CM) <= 40;
        } else {
          pode_andar = ultrassonico_frontal.Ranging(CM) >= DISTANCIA_MINIMA_DA_PAREDE;
        }

        if(pode_andar){
          if(indo_para_frente){
            ligar_motores(1,1);
          } else {
            ligar_motores(-1,-1);
          }
        } else {

          if(indo_para_frente){
            frear_motores(1);
          } else {
            frear_motores(-1);
          }

          desligar_medicao();

          distancia_total_percorrida = distancia_total_percorrida + abs(ultrassonico_frontal.Ranging(CM) - distancia_inicial_parede);
          pulsos_totais = pulsos_totais + pulsos_encoder;
          contador_calibrador++;

          delay(1000);
          indo_para_frente = !indo_para_frente;
          distancia_inicial_parede = ultrassonico_frontal.Ranging(CM);
          ligar_medicao();
        }

      } else {
        ligar_motores(0,0);
        int media_de_cms_por_pulso = distancia_total_percorrida/pulsos_totais;
        if(media_de_cms_por_pulso == 0){
          media_de_cms_por_pulso = 1;
        }
        escrever_lcd("Media de:", String(media_de_cms_por_pulso) + "cms/pulso");
        gravar_eeprom(media_de_cms_por_pulso);

        delay(10000);
        modo = String("Em espera");
        reiniciar_robo();
      }

    } else if(modo == "Autonomo"){

      switch(passo){

        case 0: {

          if(ultrassonico_frontal.Ranging(CM) > DISTANCIA_MINIMA_DA_PAREDE){
            ligar_motores(-1,-1);
          } else {
            frear_motores(-1);

            delay(1000);
            passo++;
          }

          break;
        }

        case 1: {

          if(ultrassonico_frontal.Ranging(CM) < 40){
            ligar_motores(1, 1);
          } else {
            frear_motores(1);
            passo++;
            delay(1000);
          }
          break;
        }

        case 2: {

          if(!medindo){
            ligar_medicao();
          }

          if(pulsos_encoder < PULSOS_PARA_CURVA){
            ligar_motores(-1, 1);
          } else {
            ligar_motores(0,0);
            desligar_medicao();

            passo++;
            delay(1000);
          }
          distancia_direita_inicio = 0;
          break;
        }
    
        case 3: {

          if(distancia_direita_inicio == 0){
            distancia_direita_inicio = ultrassonico_direito.Ranging(CM);
          }

          if(!medindo){
            ligar_medicao();
          }

          if(virando){
            if(pulsos_encoder - pulsos_inicio_de_correcao > PULSOS_PARA_CORRECAO){
              ligar_motores(0,0);
              delay(500);

              virando = false;
              contador_de_correcoes++;
              distancia_direita_inicio = ultrassonico_direito.Ranging(CM);
              delay(1000);
            }
            return;
            break;
          }
          
          bool pode_andar;

          if(indo_para_frente){
            pode_andar = ultrassonico_frontal.Ranging(CM) > DISTANCIA_MINIMA_DA_PAREDE;
          } else {
            pode_andar = ultrassonico_traseiro.Ranging(CM) > DISTANCIA_MINIMA_DA_PAREDE;
          }

          if(pode_andar){
            int variacao_lateral = ultrassonico_direito.Ranging(CM) - distancia_direita_inicio;

            if(abs(variacao_lateral) > 5 && abs(variacao_lateral) < 20){ 

              if(indo_para_frente){
                frear_motores(-1);
              } else {
                frear_motores(1);
              }
              delay(1000);

              pulsos_inicio_de_correcao = pulsos_encoder;
              virando = true;

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


            if(procurando_canto){
              indo_para_frente = !indo_para_frente;
              distancia_direita_inicio = ultrassonico_direito.Ranging(CM);
              procurando_canto = false;

              desligar_medicao();
            } else {

              int tamanho = (pulsos_encoder*cms_por_pulso) + (2* DISTANCIA_MINIMA_DA_PAREDE) + 28 - contador_de_correcoes * PULSOS_PARA_CORRECAO; 

              if(primeira_reta == 0){
                primeira_reta = tamanho;
                escrever_lcd(String(primeira_reta) + "cm", "N/M");
                delay(5000);
                desligar_medicao();

                reiniciar_robo();
                passo = 0;
              } else {
                
                segunda_reta = tamanho;
                escrever_lcd(String(primeira_reta) + "cm", String(segunda_reta) + "cm");
                delay(10000);

                reiniciar_robo();
                modo = String("Em espera");
                primeira_reta = 0;
                segunda_reta = 0;
              }

            }
            contador_de_correcoes = 0;
          }

          break;
          
        }

        default: {
          ligar_motores(0,0);
        }
      }
    } else if(modo == "Linha Reta"){

      switch(passo){

        case 0: {
          if(ultrassonico_frontal.Ranging(CM) > 10){
            ligar_motores(-1,-1);
          } else {
            frear_motores(-1);

            delay(1000);
            passo++;
          }

          break;
        }

        case 1: {
          if(!medindo){
            ligar_medicao();
          }

          if(ultrassonico_traseiro.Ranging(CM) > 10){
            ligar_motores(1,1);
          } else {
            frear_motores(1);
            int tamanho = (pulsos_encoder*cms_por_pulso) + 47; 
            escrever_lcd("Linha reta:", String(tamanho) + String("cm"));
            delay(10000);

            reiniciar_robo();
            modo = String("Em espera");
          }
          break;
        }

      }

    }
  
  }
}
