#include <LiquidCrystal.h>

#define rs 12
#define en 11
#define d4 5
#define d5 4
#define d6 3
#define d7 2

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int acierto = 0;
int intento = 0;
int intentosMax = 8;
int restante = 8;

void setup() {
  lcd.begin(16, 2);
  lcd.print("Mastermind");
  delay(2000);
  lcd.clear();
}

void loop() {
  intento++;
  acierto = 2;
  restante = intentosMax - intento;

  lcd.setCursor(0, 0);
  lcd.print("Aciertos: ");
  lcd.print(acierto);

  lcd.setCursor(0, 1);
  lcd.print("Usados:");
  lcd.print(intento);
  lcd.print(" Rest:");
  lcd.print(restante);

  delay(3000);
}