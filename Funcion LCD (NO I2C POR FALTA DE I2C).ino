#include <LiquidCrystal.h>

              //  rs  E db4 db5 db6 db7
              //   5  6  10  11  12  13
// LiquidCrystal (rs, e, d4, d5, d6, d7)
LiquidCrystal LCD(5, 6, 10, 11, 12, 13);

void setup() {
  LCD.begin( 16 , 2 );
  
  LCD.setCursor(0, 0);
  LCD.print("INICIANDO. . .");

}

void loop() {
  delay(1600);
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("EN PARTIDA.");
  delay(25000);
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("ALERTA INTRUSO");
  delay(999);
}