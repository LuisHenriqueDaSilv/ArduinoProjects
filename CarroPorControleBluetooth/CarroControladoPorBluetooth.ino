#include <Dabble.h>

// Motores
#define motor_direito_horario 4
#define motor_direito_anti 5
#define motor_esquerdo_horario 6
#define motor_esquerdo_anti 7

void setup(){

  Dabble.begin(9600);

  pinMode(motor_direito_horario, OUTPUT);
  pinMode(motor_direito_anti, OUTPUT);
  pinMode(motor_esquerdo_horario, OUTPUT);
  pinMode(motor_esquerdo_anti, OUTPUT);
}

void loop(){

  Dabble.processInput();

  if (GamePad.isUpPressed()){

    ligar_motores(1, 1);

  }else if (GamePad.isDownPressed()){

    ligar_motores(-1, -1);

  }else if (GamePad.isLeftPressed()){

    ligar_motores(1, -1);

  } else if (GamePad.isRightPressed()){

    ligar_motores(-1, 1);

  } else {

    desligar_motores();

  }
  
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
  }

}

void desligar_motores(){
  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, LOW);
}
