#include <Adafruit_NeoPixel.h>

#define NeoPin 3
#define Cant_Pixel 64

Adafruit_NeoPixel Pixel = Adafruit_NeoPixel(Cant_Pixel, NeoPin, NEO_GRB + NEO_KHZ800);

void setup() {
  Pixel.begin();
  Pixel.setBrightness(50);
  Pixel.show();
}

void loop() {
  AnimacionVictoria();
  delay(1500); 

  AnimacionDerrota();
  delay(1500);

  AnimacionDeX();
  delay(1500);
  
  AnimacionCorazon();
  delay(1500);
  
  
}

// 1. Carita Feliz (Verde)
void AnimacionVictoria() {
  int Matriz[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
  };

  for (int fila = 0; fila < 8; fila++) {
    for (int columna = 0; columna < 8; columna++) {
      int nroPixel = fila * 8 + columna;   
      if (Matriz[fila][columna] == 1) {
        Pixel.setPixelColor(nroPixel, Pixel.Color(0, 255, 0)); // Verde
      } else {
        Pixel.setPixelColor(nroPixel, Pixel.Color(0, 0, 0));   
      }
    }
  }
  Pixel.show();
}

// 2. Carita Triste (Roja)
void AnimacionDerrota() {
  int Matriz[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
  };

  for (int fila = 0; fila < 8; fila++) {
    for (int columna = 0; columna < 8; columna++) {
      int nroPixel = fila * 8 + columna;   
      if (Matriz[fila][columna] == 1) {
        Pixel.setPixelColor(nroPixel, Pixel.Color(255, 0, 0)); 
      } else {
        Pixel.setPixelColor(nroPixel, Pixel.Color(0, 0, 0));   
      }
    }
  }
  Pixel.show();
}

// 3. Icono de "X" (Roja)(Perdío el juego)
void AnimacionDeX() {
  int Matriz[8][8] = {
    {1, 1, 0, 0, 0, 0, 1, 1},
    {0, 1, 0, 0, 0, 0, 1, 1},
    {0, 0, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {1, 1, 0, 0, 0, 0, 1, 1}
  };

  for (int fila = 0; fila < 8; fila++) {
    for (int columna = 0; columna < 8; columna++) {
      int nroPixel = fila * 8 + columna;   
      if (Matriz[fila][columna] == 1) {
        Pixel.setPixelColor(nroPixel, Pixel.Color(255, 0, 0)); // Rojo
      } else {
        Pixel.setPixelColor(nroPixel, Pixel.Color(0, 0, 0));   
      }
    }
  }
  Pixel.show();
}


  
//Gracias Por jugar
 void AnimacionCorazon() {
  int Matriz[8][8] = {
    {0, 1, 0, 0, 0, 0, 1, 0},
    {1, 1, 1, 0, 0, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0}
  };

  for (int fila = 0; fila < 8; fila++) {
    for (int columna = 0; columna < 8; columna++) {
      int nroPixel = fila * 8 + columna;
      if (Matriz[fila][columna] == 1) {
        Pixel.setPixelColor(nroPixel, Pixel.Color(255, 0, 100)); // Rosa
      } else {
        Pixel.setPixelColor(nroPixel, Pixel.Color(0, 0, 0));
      }
    }
  }
  Pixel.show();
}