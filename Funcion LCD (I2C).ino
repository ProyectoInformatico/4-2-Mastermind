#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
// LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  lcd.init();
  
  lcd.backlight();
  delay(500);
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("INICIANDO. . .");

}

void loop() {
  delay(1600);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EN PARTIDA.");
  delay(25000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALERTA INTRUSO");
  delay(999);
}