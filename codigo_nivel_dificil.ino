#include <Adafruit_NeoPixel.h>

#define PIN_NEOPIXEL 3
#define NUM_LEDS 64

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUM_LEDS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

int pinEncoderA = 2;
int pinEncoderB = 6;
int pinBoton    = 5;

int ultimoEstadoA;
int ultimoEstadoBoton = HIGH;

int claveSecreta[7];
int intentoJugador[7] = {0, 0, 0, 0, 0, 0, 0};

int filaActiva = 0;        
int celdaActiva = 0;       
int colorSeleccionado = 0; 

int estadoJuego = 0;

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

void generarClave() {
  for (int i = 0; i < 7; i++) {
    claveSecreta[i] = random(0, 8);
  }
  
  pintarLED(0, colorSeleccionado);
  matrix.show();
  
  estadoJuego = 1;
}

void leerGiroEncoder() {
  int estadoA = digitalRead(pinEncoderA);
  
  if (estadoA != ultimoEstadoA) {
    if (digitalRead(pinEncoderB) != estadoA) {
      colorSeleccionado++;
      if (colorSeleccionado > 7) 
        colorSeleccionado = 0;
    } else {
      colorSeleccionado--;
      if (colorSeleccionado < 0) 
        colorSeleccionado = 7;
    }

    intentoJugador[celdaActiva] = colorSeleccionado;

    int posicionLED = (filaActiva * 8) + celdaActiva;
    pintarLED(posicionLED, colorSeleccionado);
    matrix.show();
  }
  ultimoEstadoA = estadoA;
}

void leerBoton() {
  int estadoBoton = digitalRead(pinBoton);
  
  if (estadoBoton == LOW && ultimoEstadoBoton == HIGH) {
    delay(200);
    
    celdaActiva++;
    colorSeleccionado = 0;

    if (celdaActiva > 5) {
      estadoJuego = 2;
    } else {
      int posicionLED = (filaActiva * 8) + celdaActiva;
      pintarLED(posicionLED, colorSeleccionado);
      matrix.show();
    }
  }
  ultimoEstadoBoton = estadoBoton;
}

void validarJugada() {
  int aciertosPosicion = 0;
  int aciertosColor = 0;

  bool claveUsada[7] = {false, false, false, false, false, false, false};
  bool intentoUsado[7] = {false, false, false, false, false, false, false};

  for (int i = 0; i < 7; i++) {
    if (intentoJugador[i] == claveSecreta[i]) {
      aciertosPosicion++;
      claveUsada[i] = true;
      intentoUsado[i] = true;
    }
  }

  for (int i = 0; i < 7; i++) {
    if (!intentoUsado[i]) {
      for (int j = 0; j < 7; j++) {
        if (!claveUsada[j] && intentoJugador[i] == claveSecreta[j]) {
          aciertosColor++;
          claveUsada[j] = true;
          break;
        }
      }
    }
  }

  delay(1500);

  for (int i = 0; i < 7; i++) {
    int posicionLED = (filaActiva * 8) + i;
    matrix.setPixelColor(posicionLED, matrix.Color(0, 0, 0));
  }

  int indicePista = 0;
  for (int i = 0; i < aciertosPosicion; i++) {
    int posicionPista = (filaActiva * 8) + (7 - indicePista);
    matrix.setPixelColor(posicionPista, matrix.Color(0, 255, 0));
    indicePista++;
  }
  
  for (int i = 0; i < aciertosColor; i++) {
    int posicionPista = (filaActiva * 8) + (7 - indicePista);
    matrix.setPixelColor(posicionPista, matrix.Color(255, 0, 0));
    indicePista++;
  }

  if (aciertosPosicion >= 2 || aciertosColor >= 2) {
    int posicionExtra = (filaActiva * 8) + 7;
    matrix.setPixelColor(posicionExtra, matrix.Color(0, 255, 0));
  } else {
    int posicionExtra = (filaActiva * 8) + 7;
    matrix.setPixelColor(posicionExtra, matrix.Color(255, 0, 0));
  }

  matrix.show();

  if (aciertosPosicion == 7) {
    estadoJuego = 3;
  } else if (filaActiva == 7) {
    estadoJuego = 4;
  } else {
    delay(1500);
    
    filaActiva++;
    celdaActiva = 0;
    colorSeleccionado = 0;
    
    for (int i = 0; i < 7; i++) {
      intentoJugador[i] = 0;
    }
    
    int posicionLED = (filaActiva * 8) + celdaActiva;
    pintarLED(posicionLED, colorSeleccionado);
    matrix.show();

    estadoJuego = 1;
  }
}

void mostrarVictoria() {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(0, 255, 0));
  }
  matrix.show();
}

void mostrarDerrota() {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 0, 0));
  }
  matrix.show();
}

void pintarLED(int led, int color) {
  switch (color) {
    case 0: 
      matrix.setPixelColor(led, matrix.Color(255, 0, 0));   
      break;
    case 1: 
      matrix.setPixelColor(led, matrix.Color(0, 255, 0));   
      break;
    case 2: 
      matrix.setPixelColor(led, matrix.Color(0, 0, 255));   
      break;
    case 3: 
      matrix.setPixelColor(led, matrix.Color(255, 255, 0)); 
      break;
    case 4: 
      matrix.setPixelColor(led, matrix.Color(0, 255, 255)); 
      break;
    case 5: 
      matrix.setPixelColor(led, matrix.Color(255, 0, 255)); 
      break;
    case 6: 
      matrix.setPixelColor(led, matrix.Color(255, 128, 0)); 
      break;
    case 7: 
      matrix.setPixelColor(led, matrix.Color(255, 255, 255)); 
      break;
  }
}