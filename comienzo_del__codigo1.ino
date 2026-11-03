#include <Adafruit_NeoPixel.h>

#define PIN_NEOPIXEL 3
#define NUM_LEDS 64 // Matriz 8x8 = 64 LEDs

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUM_LEDS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Pines del Encoder (Motor CC) y Botón
int pinEncoderA = 2;
int pinEncoderB = 6;
int pinBoton    = 5;

int ultimoEstadoA;
int ultimoEstadoBoton = HIGH;

// Variables de juego
int claveSecreta[4];
int intentoJugador[4] = {0, 0, 0, 0};

int filaActiva = 0;        // Filas del 0 al 7 (8 intentos)
int celdaActiva = 0;       // Columnas del 0 al 3 (Colores del juego)
int colorSeleccionado = 0; // Colores del 0 al 5

// Estados: 0 = Inicio, 1 = Jugando, 2 = Validar, 3 = Ganó, 4 = Perdió
int estadoJuego = 0;

// Declaración de funciones
void generarClave();
void leerGiroEncoder();
void leerBoton();
void validarJugada();
void mostrarVictoria();
void mostrarDerrota();
void pintarLED(int led, int color);

void setup() {
  pinMode(pinEncoderA, INPUT);
  pinMode(pinEncoderB, INPUT);
  pinMode(pinBoton, INPUT_PULLUP);
  
  matrix.setBrightness(50);
  matrix.begin();
  matrix.clear();
  matrix.show();

  ultimoEstadoA = digitalRead(pinEncoderA);
  randomSeed(analogRead(A0));
}

void loop() {
  switch (estadoJuego) {
    case 0:
      generarClave();
      break;

    case 1:
      leerGiroEncoder();
      leerBoton();
      break;

    case 2:
      validarJugada();
      break;

    case 3:
      mostrarVictoria();
      break;

    case 4:
      mostrarDerrota();
      break;
  }
}


// 1. Genera la combinación secreta aleatoria
void generarClave() {
  for (int i = 0; i < 4; i++) {
    claveSecreta[i] = random(0, 6);
  }
  
  // Muestra el primer LED en rojo al arrancar
  pintarLED(0, colorSeleccionado);
  matrix.show();
  
  estadoJuego = 1; // Pasa a jugar
}

// 2. Lee el giro del motor/encoder para cambiar el color
void leerGiroEncoder() {
  int estadoA = digitalRead(pinEncoderA);
  
  if (estadoA != ultimoEstadoA) {
    if (digitalRead(pinEncoderB) != estadoA) {
      colorSeleccionado++;
      if (colorSeleccionado > 5) 
        colorSeleccionado = 0;
    } else {
      colorSeleccionado--;
      if (colorSeleccionado < 0) 
        colorSeleccionado = 5;
    }

    intentoJugador[celdaActiva] = colorSeleccionado;

    int posicionLED = (filaActiva * 8) + celdaActiva;
    pintarLED(posicionLED, colorSeleccionado);
    matrix.show();
  }
  ultimoEstadoA = estadoA;
}

// 3. Lee el botón para avanzar de celda 
void leerBoton() {
  int estadoBoton = digitalRead(pinBoton);
  
  if (estadoBoton == LOW && ultimoEstadoBoton == HIGH) {
    delay(200); // Anti-rebote
    
    celdaActiva++;
    colorSeleccionado = 0; // Reiniciar color base para la siguiente celda

    if (celdaActiva > 3) {
      estadoJuego = 2; // Valida si completó la fila
    } else {
      // Enciende la nueva celda activa inmediatamente
      int posicionLED = (filaActiva * 8) + celdaActiva;
      pintarLED(posicionLED, colorSeleccionado);
      matrix.show();
    }
  }
  ultimoEstadoBoton = estadoBoton;
}

// 4. Compara la jugada y coloca las pistas blancas
void validarJugada() {
  int aciertosPosicion = 0;

  for (int i = 0; i < 4; i++) {
    if (intentoJugador[i] == claveSecreta[i]) {
      aciertosPosicion++;
    }
  }

  // Dibujar pistas en las columnas 4 a 7
  for (int i = 0; i < aciertosPosicion; i++) {
    int posicionPista = (filaActiva * 8) + (4 + i);
    matrix.setPixelColor(posicionPista, matrix.Color(255, 255, 255));
  }
  matrix.show();

  if (aciertosPosicion == 4) {
    estadoJuego = 3; // Ganó
  } else if (filaActiva == 7) {
    estadoJuego = 4; // Perdió
  } else {
    filaActiva++;
    celdaActiva = 0;
    colorSeleccionado = 0;
    
    // Prende la primera casilla del nuevo intento
    int posicionLED = (filaActiva * 8) + celdaActiva;
    pintarLED(posicionLED, colorSeleccionado);
    matrix.show();

    estadoJuego = 1;
  }
}

// 5. Animación cuando gana (Toda la matriz verde)
void mostrarVictoria() {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(0, 255, 0));
  }
  matrix.show();
}

// 6. Animación cuando pierde (Toda la matriz roja)
void mostrarDerrota() {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 0, 0));
  }
  matrix.show();
}

// 7. Mapeo de números a colores usando switch
void pintarLED(int led, int color) {
  switch (color) {
    case 0: 
      matrix.setPixelColor(led, matrix.Color(255, 0, 0));   
      break; // Rojo
    case 1: 
      matrix.setPixelColor(led, matrix.Color(0, 255, 0));   
      break; // Verde
    case 2: 
      matrix.setPixelColor(led, matrix.Color(0, 0, 255));   
      break; // Azul
    case 3: 
      matrix.setPixelColor(led, matrix.Color(255, 255, 0)); 
      break; // Amarillo
    case 4: 
      matrix.setPixelColor(led, matrix.Color(0, 255, 255)); 
      break; // Cian
    case 5: 
      matrix.setPixelColor(led, matrix.Color(255, 0, 255)); 
      break; // Magenta
  }
}