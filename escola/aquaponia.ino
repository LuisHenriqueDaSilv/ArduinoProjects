#include <Arduino.h>
#include <LiquidCrystal_I2C.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>  


#define    A0
#define PORTA_SENSOR_DE_TEMPERATURA 7

int valorNivelDeAgua = 0;
int leituraTemperatura = 0;

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
  
  SensorDeTemperatura.begin();
  
  displayLCD.init();
  displayLCD.backlight();
  displayLCD.clear();
}

void loop() {
  delay(10);
  SensorDeTemperatura.requestTemperatures();
  
  valorNivelDeAgua = analogRead(PORTA_SENSOR_DE_AGUA)/10; 
  leituraTemperatura=SensorDeTemperatura.getTempCByIndex(0);
  
  Serial.println(valorNivelDeAgua);
  Serial.println(leituraTemperatura);
  // escreverLCD(String("Nivel agua:"), String("Temp da agua:") );
  escreverLCD(String("Nivel agua:") + String(valorNivelDeAgua), String("Temp da agua:") + String(leituraTemperatura) + String("c") );



  delay(200);
}

