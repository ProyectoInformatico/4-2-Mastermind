/* investiga librería Adafruit NeoPixel, 
prueba encender un LED y 
prueba un barrido completo de colores.*/

#include <Adafruit_NeoPixel.h>

#define NeoPin 3
#define Cant_Pixel 64

int velocidad = 500; 
int Matriz[8][8];
Adafruit_NeoPixel Pixel = Adafruit_NeoPixel(Cant_Pixel, NeoPin, NEO_GRB + NEO_KHZ800);

void setup() {
  Pixel.begin();
  Pixel.show();
  Pixel.clear();
}

void loop() 
{
  
  for(int fila=0; fila<8; fila++) 
  {
    for(int columna=0; columna<8; columna++) 
    {
      int pixelID = fila*8 + columna;
      Pixel.setPixelColor(pixelID, Pixel.Color(0, 255, 255)); 
    }
    Pixel.show(); 
    delay(velocidad);
  }
  
  delay(100); 
  
  //Barrido de color
  int Num_Leds=0;
  for(int i=0; i<64; i++)
  {
    //                                       (R, G, B)
    Pixel.clear();
    Pixel.setPixelColor(Num_Leds,Pixel.Color(255,255,255));
    Pixel.show();
    delay(velocidad);
      
    if(Num_Leds < 64 )
    {
      Num_Leds++;
    }
  }

  Pixel.clear();
  Pixel.show();
  delay(200);
}
