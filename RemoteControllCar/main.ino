#include <Dabble.h>

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE

#define motor_direito_horario 8
#define motor_direito_anti 9
#define motor_esquerdo_horario 11
#define motor_esquerdo_anti 10

void setup() {
  Serial.begin(19200);
  Dabble.begin(9600);

  pinMode(motor_direito_horario, OUTPUT);
  pinMode(motor_direito_anti, OUTPUT);
  pinMode(motor_esquerdo_horario, OUTPUT);
  pinMode(motor_esquerdo_anti, OUTPUT);
}



void andar(){

  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, HIGH);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, HIGH);
}

void virar_esquerda(){

  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, HIGH);

  digitalWrite(motor_esquerdo_horario, HIGH);
  digitalWrite(motor_esquerdo_anti, LOW);
}

void virar_direita(){
  digitalWrite(motor_direito_horario, HIGH);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, HIGH);
}

void frear(){
  digitalWrite(motor_direito_horario, LOW);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, LOW);
  digitalWrite(motor_esquerdo_anti, LOW);
}

void re(){

  digitalWrite(motor_direito_horario, HIGH);
  digitalWrite(motor_direito_anti, LOW);

  digitalWrite(motor_esquerdo_horario, HIGH);
  digitalWrite(motor_esquerdo_anti, LOW);
}




void loop() {
  Dabble.processInput();
  Serial.println();
  Serial.print("KeyPressed: ");

  
  if (GamePad.isUpPressed()){
    Serial.print("UP");
    andar();
  }else if (GamePad.isDownPressed()){
    Serial.print("DOWN");
    re();
  }else if (GamePad.isLeftPressed()){
    virar_esquerda();
    Serial.print("Left");
  } else if (GamePad.isRightPressed()){
    virar_direita();
    Serial.print("Right");
  } else {
    frear();
  }

//  if (GamePad.isSquarePressed()){
//    Serial.print("Square");
//  }
//
//  if (GamePad.isCirclePressed()){
//    Serial.print("Circle");
//  }
//
//  if (GamePad.isCrossPressed()){
//    Serial.print("Cross");
//  }
//
//  if (GamePad.isTrianglePressed()){
//    Serial.print("Triangle");
//  }
//
//  if (GamePad.isStartPressed()){
//    Serial.print("Start");
//  }
//
//  if (GamePad.isSelectPressed())
//  {
//    Serial.print("Select");
//  }

//  int a = GamePad.getAngle();
//  Serial.print("Angle: ");
//  Serial.print(a);
//  Serial.print('\t');
//  int b = GamePad.getRadius();
//  Serial.print("Radius: ");
//  Serial.print(b);
//  Serial.print('\t');
//  float c = GamePad.getXaxisData();
//  Serial.print("x_axis: ");
//  Serial.print(c);
//  Serial.print('\t');
//  float d = GamePad.getYaxisData();
//  Serial.print("y_axis: ");
//  Serial.println(d);
//  Serial.println();


}
