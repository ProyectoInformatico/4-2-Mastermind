#include <Wire.h>
#include <LiquidCrystal_I2C.h>
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

const byte DIRECCION_LCD = 0x27;
const byte ANCHO_MATRIZ  = 8;
const byte LARGO_MAX     = 5;
const byte COLORES_MAX   = 6;
const byte BRILLO        = 40;
const bool MATRIZ_ZIGZAG = false;
const bool MODO_DEBUG    = false;

const unsigned long TIEMPO_NIVEL2_MS = 150000UL;

Adafruit_NeoPixel Pixel(CANT_PIXEL, NEO_PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C LCD(DIRECCION_LCD, 16, 2);

const uint8_t PALETA[COLORES_MAX][3] = {
  {255,   0,   0},
  {  0, 255,   0},
  {  0,   0, 255},
  {255, 255,   0},
  {  0, 255, 255},
  {255,   0, 255}
};

const char* NOMBRES[COLORES_MAX] = {
  "Rojo", "Verde", "Azul", "Amarillo", "Cian", "Magenta"
};

byte nivel           = 1;
byte largoClave      = 4;
byte maxIntentos     = 8;
byte cantColores     = 6;
byte filasPorIntento = 1;
unsigned long tiempoLimite = 0;
unsigned long inicioJuego  = 0;
int  segPrevios = -1;

byte claveSecreta[LARGO_MAX];
byte combinacionJugador[LARGO_MAX];

byte fila    = 0;
byte columna = 0;
byte color   = 0;
int  ultimoCLK;
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

void sonidoConfirmar() {
  tone(PIN_BUZZER, NOTE_A4, 80);
  delay(100);
  noTone(PIN_BUZZER);
}

void sonidoError() {
  tone(PIN_BUZZER, 200, 300);
  delay(350);
  tone(PIN_BUZZER, 150, 500);
  delay(550);
  noTone(PIN_BUZZER);
}

void sonidoVictoria() {
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

void tocarMelodia() {
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

int segundosRestantes() {
  unsigned long transcurrido = millis() - inicioJuego;
  if (transcurrido >= tiempoLimite) return 0;
  return (int)((tiempoLimite - transcurrido + 999UL) / 1000UL);
}

void actualizarLCD() {
  char buf[20];

  if (tiempoLimite > 0) {
    snprintf(buf, sizeof(buf), "Int:%d/%d T:%3ds", fila + 1, maxIntentos, segundosRestantes());
  } else {
    snprintf(buf, sizeof(buf), "Intento: %d/%d", fila + 1, maxIntentos);
  }
  mostrarLinea(0, buf);

  snprintf(buf, sizeof(buf), "Color: %s", NOMBRES[color]);
  mostrarLinea(1, buf);
}

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
  Pixel.setPixelColor(indicePixel(f * filasPorIntento, c), colorPaleta(col));
  Pixel.show();
}

void dibujarPistas(byte f, byte exactos, byte parciales) {
  byte filaPix, col;
  if (filasPorIntento == 1) {
    filaPix = f;
    col = largoClave;
  } else {
    filaPix = f * filasPorIntento + 1;
    col = 0;
  }

  for (byte i = 0; i < exactos; i++) {
    Pixel.setPixelColor(indicePixel(filaPix, col), Pixel.Color(255, 255, 255));
    col++;
  }
  for (byte i = 0; i < parciales; i++) {
    Pixel.setPixelColor(indicePixel(filaPix, col), Pixel.Color(255, 90, 0));
    col++;
  }
  Pixel.show();
}

int8_t leerEncoder() {
  int8_t giro = 0;
  int estadoCLK = digitalRead(PIN_CLK);

  if (estadoCLK != ultimoCLK && estadoCLK == LOW) {
    giro = (digitalRead(PIN_DT) != estadoCLK) ? 1 : -1;
  }

  ultimoCLK = estadoCLK;
  return giro;
}

bool botonPresionado() {
  if (digitalRead(PIN_SW) == LOW) {
    delay(30);
    if (digitalRead(PIN_SW) == LOW) {
      while (digitalRead(PIN_SW) == LOW) {}
      delay(30);
      return true;
    }
  }
  return false;
}

void mostrarNivelMenu() {
  mostrarLinea(0, "Elige nivel:");
  if (nivel == 1) mostrarLinea(1, "< 1 Normal     >");
  else            mostrarLinea(1, "< 2 Reloj      >");
}

void menuNivel() {
  mostrarNivelMenu();
  ultimoCLK = digitalRead(PIN_CLK);

  while (true) {
    if (leerEncoder() != 0) {
      nivel = (nivel == 1) ? 2 : 1;
      mostrarNivelMenu();
    }
    if (botonPresionado()) break;
  }

  randomSeed(micros() ^ analogRead(A0));

  if (nivel == 1) {
    largoClave = 4;  maxIntentos = 8;  cantColores = 6;
    filasPorIntento = 1;  tiempoLimite = 0;
  } else {
    largoClave = 5;  maxIntentos = 4;  cantColores = 5;
    filasPorIntento = 2;  tiempoLimite = TIEMPO_NIVEL2_MS;
  }

  ultimoCLK = digitalRead(PIN_CLK);
}

void generarClave() {
  for (byte i = 0; i < largoClave; i++) {
    claveSecreta[i] = random(0, cantColores);
  }

  if (MODO_DEBUG) {
    Serial.print("Clave: ");
    for (byte i = 0; i < largoClave; i++) {
      Serial.print(claveSecreta[i]);
      Serial.print(' ');
    }
    Serial.println();
  }
}

void iniciarPartida() {
  Pixel.clear();
  generarClave();

  fila = 0;
  columna = 0;
  color = 0;
  juegoTerminado = false;
  inicioJuego = millis();
  segPrevios = -1;

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

void perder(bool porTiempo) {
  mostrarLinea(0, porTiempo ? "SE ACABO TIEMPO" : "PERDISTE");
  mostrarLinea(1, "Mira la clave");

  Pixel.clear();
  for (byte i = 0; i < largoClave; i++) {
    Pixel.setPixelColor(indicePixel(0, i), colorPaleta(claveSecreta[i]));
  }
  Pixel.show();

  sonidoError();
  juegoTerminado = true;
}

void evaluarIntento() {
  byte exactos = 0;
  byte parciales = 0;
  byte contClave[COLORES_MAX] = {0};
  byte contJugador[COLORES_MAX] = {0};

  for (byte i = 0; i < largoClave; i++) {
    if (combinacionJugador[i] == claveSecreta[i]) {
      exactos++;
    } else {
      contClave[claveSecreta[i]]++;
      contJugador[combinacionJugador[i]]++;
    }
  }

  for (byte c = 0; c < cantColores; c++) {
    parciales += min(contClave[c], contJugador[c]);
  }

  dibujarPistas(fila, exactos, parciales);

  if (exactos == largoClave) {
    ganar();
    return;
  }

  sonidoError();
  fila++;

  if (fila >= maxIntentos) {
    perder(false);
  } else {
    columna = 0;
    color = 0;
    dibujarColor(fila, columna, color);
    actualizarLCD();
  }
}

void confirmarColor() {
  combinacionJugador[columna] = color;
  sonidoConfirmar();
  columna++;

  if (columna >= largoClave) {
    evaluarIntento();
  } else {
    dibujarColor(fila, columna, color);
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

  LCD.init();
  LCD.backlight();
  mostrarLinea(0, "-M-Bienvenido-M-");
  mostrarLinea(1, "Iniciando...");
  delay(1500);

  menuNivel();
  iniciarPartida();
}

void loop() {
  if (juegoTerminado) {
    if (botonPresionado()) {
      menuNivel();
      iniciarPartida();
    }
    return;
  }

  if (tiempoLimite > 0) {
    if (millis() - inicioJuego >= tiempoLimite) {
      perder(true);
      return;
    }
    int seg = segundosRestantes();
    if (seg != segPrevios) {
      segPrevios = seg;
      actualizarLCD();
    }
  }

  int8_t giro = leerEncoder();
  if (giro != 0) {
    color = (color + cantColores + giro) % cantColores;
    dibujarColor(fila, columna, color);
    actualizarLCD();
  }

  if (botonPresionado()) {
    confirmarColor();
  }
}
