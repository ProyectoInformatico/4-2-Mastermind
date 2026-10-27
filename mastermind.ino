#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>


#define NEO_PIN     3
#define PIN_CLK     7
#define PIN_DT      4
#define PIN_SW      2
#define PIN_BUZZER  8


#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_G5 784
#define NOTE_C6 1047


#define CANT_PIXEL 64

const byte ANCHO_MATRIZ  = 8;      // pixeles por fila
const byte LARGO_CLAVE   = 4;      // celdas de color por intento
const byte MAX_INTENTOS  = 8;      // filas de la matriz
const byte CANT_COLORES  = 6;      // colores disponibles
const byte BRILLO        = 40;     // 0-255 (bajo = menos consumo)
const bool MATRIZ_ZIGZAG = false;  // true si tu matriz invierte el sentido en filas impares
const bool MODO_DEBUG    = false;  // true = muestra la clave por el monitor serie


Adafruit_NeoPixel Pixel(CANT_PIXEL, NEO_PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal LCD(5, 6, 10, 11, 12, 13);   // RS, E, D4, D5, D6, D7


const uint8_t PALETA[CANT_COLORES][3] = {
  {255,   0,   0},   // 0 Rojo
  {  0, 255,   0},   // 1 Verde
  {  0,   0, 255},   // 2 Azul
  {255, 255,   0},   // 3 Amarillo
  {  0, 255, 255},   // 4 Cian
  {255,   0, 255}    // 5 Magenta
};

const char* NOMBRES[CANT_COLORES] = {
  "Rojo", "Verde", "Azul", "Amarillo", "Cian", "Magenta"
};

byte claveSecreta[LARGO_CLAVE];
byte combinacionJugador[LARGO_CLAVE];

byte fila    = 0;      // intento actual (0 a 7)
byte columna = 0;      // celda actual (0 a 3)
byte color   = 0;      // color seleccionado (0 a 5)
int  ultimoCLK;        // ultimo estado del pin CLK del encoder
bool juegoTerminado = false;


const int melody[] = {
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_E4, NOTE_D4,
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_C5, NOTE_G4, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4,
  NOTE_D4, NOTE_E4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4
};

const int noteDurations[] = {
  4, 4, 4, 2, 4, 4, 4, 2,
  4, 4, 4, 2, 4, 4, 1, 4,
  4, 4, 2, 4, 4, 4, 2, 4,
  4, 4, 4, 4, 4, 1
};


void sonidoConfirmar() {            // beep corto al fijar un color
  tone(PIN_BUZZER, NOTE_A4, 80);
  delay(100);
  noTone(PIN_BUZZER);
}

void sonidoError() {                // intento incorrecto
  tone(PIN_BUZZER, 200, 300);
  delay(350);
  tone(PIN_BUZZER, 150, 500);
  delay(550);
  noTone(PIN_BUZZER);
}

void sonidoVictoria() {             // fanfarria ascendente
  tone(PIN_BUZZER, NOTE_C5, 200);
  delay(250);
  tone(PIN_BUZZER, NOTE_E5, 200);
  delay(250);
  tone(PIN_BUZZER, NOTE_G5, 200);
  delay(250);
  tone(PIN_BUZZER, NOTE_C6, 600);
  delay(650);
  noTone(PIN_BUZZER);
}

void tocarMelodia() {               // solo se usa al ganar
  int cantidad = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < cantidad; i++) {
    int duracion = 1000 / noteDurations[i];
    tone(PIN_BUZZER, melody[i], duracion);
    delay(duracion * 1.30);
    noTone(PIN_BUZZER);
  }
}

void mostrarLinea(byte linea, const char* texto) {
  char buf[17];
  snprintf(buf, sizeof(buf), "%-16s", texto);
  LCD.setCursor(0, linea);
  LCD.print(buf);
}

void actualizarLCD() {
  char buf[20];

  snprintf(buf, sizeof(buf), "Intento: %d/%d", fila + 1, MAX_INTENTOS);
  mostrarLinea(0, buf);

  snprintf(buf, sizeof(buf), "Color: %s", NOMBRES[color]);
  mostrarLinea(1, buf);
}


// Convierte (fila, columna) en el numero de LED
uint8_t indicePixel(byte f, byte c) {
  if (MATRIZ_ZIGZAG && (f % 2 == 1)) {
    return (f * ANCHO_MATRIZ) + (ANCHO_MATRIZ - 1 - c);
  }
  return (f * ANCHO_MATRIZ) + c;
}

uint32_t colorPaleta(byte i) {
  return Pixel.Color(PALETA[i][0], PALETA[i][1], PALETA[i][2]);
}

void dibujarColor(byte f, byte c, byte col) {
  Pixel.setPixelColor(indicePixel(f, c), colorPaleta(col));
  Pixel.show();
}

// Pistas en las columnas 4 a 7:
//   BLANCO  = color correcto en posicion correcta
//   NARANJA = color correcto en posicion incorrecta
void dibujarPistas(byte f, byte exactos, byte parciales) {
  byte col = LARGO_CLAVE;   // la primera pista va en la columna 4

  for (byte i = 0; i < exactos; i++) {
    Pixel.setPixelColor(indicePixel(f, col), Pixel.Color(255, 255, 255));
    col++;
  }
  for (byte i = 0; i < parciales; i++) {
    Pixel.setPixelColor(indicePixel(f, col), Pixel.Color(255, 90, 0));
    col++;
  }
  Pixel.show();
}



bool leerEncoder() {
  bool huboCambio = false;
  int estadoCLK = digitalRead(PIN_CLK);

  if (estadoCLK != ultimoCLK && estadoCLK == LOW) {
    if (digitalRead(PIN_DT) != estadoCLK) {
      color = (color + 1) % CANT_COLORES;
    } else {
      color = (color + CANT_COLORES - 1) % CANT_COLORES;
    }
    huboCambio = true;
  }

  ultimoCLK = estadoCLK;
  return huboCambio;
}


bool botonPresionado() {
  if (digitalRead(PIN_SW) == LOW) {
    delay(30);                              // antirrebote
    if (digitalRead(PIN_SW) == LOW) {
      while (digitalRead(PIN_SW) == LOW) {} // espera a que lo suelte
      delay(30);
      return true;
    }
  }
  return false;
}


void generarClave() {
  for (byte i = 0; i < LARGO_CLAVE; i++) {
    claveSecreta[i] = random(0, CANT_COLORES);
  }

  if (MODO_DEBUG) {
    Serial.print("Clave: ");
    for (byte i = 0; i < LARGO_CLAVE; i++) {
      Serial.print(claveSecreta[i]);
      Serial.print(' ');
    }
    Serial.println();
  }
}

void reiniciarJuego() {
  Pixel.clear();
  generarClave();

  fila = 0;
  columna = 0;
  color = 0;
  juegoTerminado = false;

  dibujarColor(fila, columna, color);
  actualizarLCD();
}

void ganar() {
  delay(800);
  mostrarLinea(0, "GANASTE!");
  char buf[20];
  snprintf(buf, sizeof(buf), "En %d intento(s)", fila + 1);
  mostrarLinea(1, buf);

  sonidoVictoria();
  tocarMelodia();

  mostrarLinea(1, "Presiona boton");
  juegoTerminado = true;
}

void perder() {
  mostrarLinea(0, "PERDISTE");
  mostrarLinea(1, "Mira la clave");

  // Muestra la clave secreta en la primera fila
  Pixel.clear();
  for (byte i = 0; i < LARGO_CLAVE; i++) {
    Pixel.setPixelColor(indicePixel(0, i), colorPaleta(claveSecreta[i]));
  }
  Pixel.show();

  sonidoError();
  juegoTerminado = true;
}


void evaluarIntento() {
  byte exactos = 0;
  byte parciales = 0;
  byte contClave[CANT_COLORES] = {0};
  byte contJugador[CANT_COLORES] = {0};

  for (byte i = 0; i < LARGO_CLAVE; i++) {
    if (combinacionJugador[i] == claveSecreta[i]) {
      exactos++;
    } else {
      contClave[claveSecreta[i]]++;
      contJugador[combinacionJugador[i]]++;
    }
  }


  for (byte c = 0; c < CANT_COLORES; c++) {
    parciales += min(contClave[c], contJugador[c]);
  }

  dibujarPistas(fila, exactos, parciales);

  if (exactos == LARGO_CLAVE) {
    ganar();
    return;
  }

  sonidoError();
  fila++;

  if (fila >= MAX_INTENTOS) {
    perder();
  } else {
    columna = 0;
    color = 0;
    dibujarColor(fila, columna, color);
    actualizarLCD();
  }
}

// Fija el color elegido en la celda actual
void confirmarColor() {
  combinacionJugador[columna] = color;
  sonidoConfirmar();
  columna++;

  if (columna >= LARGO_CLAVE) {
    evaluarIntento();
  } else {
    dibujarColor(fila, columna, color);   // vista previa en la siguiente celda
  }
}

void setup() {
  pinMode(PIN_CLK, INPUT_PULLUP);
  pinMode(PIN_DT, INPUT_PULLUP);
  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);

  if (MODO_DEBUG) Serial.begin(9600);

  Pixel.begin();
  Pixel.setBrightness(BRILLO);
  Pixel.clear();
  Pixel.show();

  LCD.begin(16, 2);
  mostrarLinea(0, "-M-Bienvenido-M-");
  mostrarLinea(1, "Iniciando...");
  delay(1500);

  randomSeed(analogRead(A0));   // A0 debe quedar sin conectar
  ultimoCLK = digitalRead(PIN_CLK);

  reiniciarJuego();
}

void loop() {
  if (juegoTerminado) {
    if (botonPresionado()) reiniciarJuego();
    return;
  }

  if (leerEncoder()) {
    dibujarColor(fila, columna, color);   // vista previa del color
    actualizarLCD();
  }

  if (botonPresionado()) {
    confirmarColor();
  }
}
