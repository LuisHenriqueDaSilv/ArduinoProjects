#include <Arduino.h>
#include <IRremote.h>

#define RECV_PIN 2
#define LEFT_MOTOR 9
#define RIGHT_MOTOR 10


IRrecv irrecv(RECV_PIN); // PASSA O PARÂMETRO PARA A FUNÇÃO irrecv

int ultimo_comando_e_acelerar = 0;

void setup()
{
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);

  irrecv.enableIRIn();
  Serial.begin(9600);
}

void acelerar(){
  ultimo_comando_e_acelerar = 1;
  analogWrite(LEFT_MOTOR, 255);
  analogWrite(RIGHT_MOTOR, 255);
}

void freiar(){
  ultimo_comando_e_acelerar = 0;
  analogWrite(LEFT_MOTOR, 0);
  analogWrite(RIGHT_MOTOR, 0);
}

void virarEsquerda(){
  if(ultimo_comando_e_acelerar == 1){
    freiar();
    delay(200);
  }
  analogWrite(LEFT_MOTOR, 0);
  analogWrite(RIGHT_MOTOR, 255);
}

void virarDireita(){
  if(ultimo_comando_e_acelerar == 1){
    freiar();
    delay(200);
  }
  analogWrite(LEFT_MOTOR, 255);
  analogWrite(RIGHT_MOTOR, 0);
}

void loop(){

  if (IrReceiver.decode()){

    switch (IrReceiver.decodedIRData.command){

      case (21):{ // Acelerar
        acelerar();
        break;
      }

      case (64):{ // Direita
        virarDireita();
        break;
      }

      case (68):{ // Esquerda
        virarEsquerda();
        break;
      }

      default: {
        freiar();
      }

    }
    IrReceiver.resume();
  }else {
    freiar();
  }


  delay(200);
}
