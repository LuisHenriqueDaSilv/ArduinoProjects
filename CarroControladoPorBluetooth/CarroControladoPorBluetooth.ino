#include <Arduino.h>
#include <Dabble.h>

// Bluetooth
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

// Motores
#define motor_direito_horario 9
#define motor_direito_anti 8
#define motor_esquerdo_horario 10
#define motor_esquerdo_anti 11

void setup(){

  Serial.begin(19200);
  Dabble.begin(9600);

  pinMode(motor_direito_horario, OUTPUT);
  pinMode(motor_direito_anti, OUTPUT);
  pinMode(motor_esquerdo_horario, OUTPUT);
  pinMode(motor_esquerdo_anti, OUTPUT);
}

void loop(){
  

  Dabble.processInput();
  Serial.println();
  Serial.print("KeyPressed: ");


  if (GamePad.isUpPressed()){
    andar();
    Serial.print("UP");
  }else if (GamePad.isDownPressed()){
    re();
    Serial.print("DOWN");
  }else if (GamePad.isLeftPressed()){
    virar_esquerda();
    Serial.print("Left");
  } else if (GamePad.isRightPressed()){
    virar_direita();
    Serial.print("Right");
  } else {
    frear();
  }

}


void frear(){
  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, LOW);
}

void andar(){

  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, HIGH);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, HIGH);
}

void re(){

  digitalWrite(motor_direito_horario, HIGH);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, HIGH);
  digitalWrite(motor_esquerdo_anti, LOW);
}

void virar_direita(){
  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, HIGH);
}

void virar_esquerda(){

  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, HIGH);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, LOW);
}
