#include <Arduino.h>
#include <LiquidCrystal_I2C.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>  


#define PORTA_SENSOR_DE_AGUA A0
#define PORTA_SENSOR_DE_TEMPERATURA 7
#define PORTA_RELE 6 
#define PORTA_LED_VERMELHO 5
#define PORTA_LED_VERDE 4 
#define PORTA_LED_AZUL 3

int leituraTemperatura = 0;
volatile int ultimo_momento_pressionado = 0;
bool bombaEstaLigada = false;

LiquidCrystal_I2C displayLCD(0x27,20,4);
OneWire oneWire(PORTA_SENSOR_DE_TEMPERATURA);
DallasTemperature SensorDeTemperatura(&oneWire);

String primeiraLinhaLCD = "";
String segundaLinhaLCD = "";

void 
escreverLCD(String primeiraLinha, String segundaLinha = ""){

	if(!primeiraLinha.equalsIgnoreCase(primeiraLinhaLCD)){
		displayLCD.setCursor(0,0);
		displayLCD.print("                ");
		displayLCD.setCursor(0,0);
		displayLCD.print(primeiraLinha);
		primeiraLinhaLCD = primeiraLinha;
	}

	if(!segundaLinha.equalsIgnoreCase(segundaLinhaLCD)){
		displayLCD.setCursor(0,1);
		displayLCD.print("                ");
		displayLCD.setCursor(0,1);
		displayLCD.print(segundaLinha);
		segundaLinhaLCD = segundaLinha;
	}

}

void setup() {
  Serial.begin(9600);

  pinMode(PORTA_RELE, OUTPUT);
  pinMode(PORTA_LED_AZUL, OUTPUT);
  pinMode(PORTA_LED_VERDE, OUTPUT);
  pinMode(PORTA_LED_VERMELHO, OUTPUT);

  SensorDeTemperatura.begin();
  
  displayLCD.init();
  displayLCD.backlight();
  displayLCD.clear();

  digitalWrite(PORTA_RELE, HIGH)


}

void loop() {
  delay(10);
  SensorDeTemperatura.requestTemperatures();
  
  leituraTemperatura=SensorDeTemperatura.getTempCByIndex(0);

  digitalWrite(PORTA_LED_AZUL, LOW);
  digitalWrite(PORTA_LED_VERDE, LOW);
  digitalWrite(PORTA_LED_VERMELHO, LOW);
  
  if(leituraTemperatura > 30){
    escreverLCD(String("Temp da agua:") + String(leituraTemperatura) + String("c"), String("Quente"));
    digitalWrite(PORTA_LED_VERMELHO, HIGH);
  } else if (leituraTemperatura < 25){
    escreverLCD(String("Temp da agua:") + String(leituraTemperatura) + String("c"), String("Frio"));
    digitalWrite(PORTA_LED_AZUL, HIGH);
  } else {
    escreverLCD(String("Temp da agua:") + String(leituraTemperatura) + String("c"), String("Ideal"));
    digitalWrite(PORTA_LED_VERDE, HIGH);
  }

  delay(1000);
}

