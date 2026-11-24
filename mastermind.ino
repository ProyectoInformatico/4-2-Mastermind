#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>
#include <avr/pgmspace.h>

#define NEO_PIN     3

// CONFIGURACION PARA PINES DEL ENCODER ROTATIVO
#define PIN_CLK     7
#define PIN_DT      4
#define PIN_SW      2

// Buzzer
#define PIN_BUZZER  8

// CONFIGURACION DE PINES DEL LCD NORMAL (RS, E, D4, D5, D6, D7)
LiquidCrystal LCD(12, 11, 10, 9, 6, 5);

// NOTAS PARA EL BUZZER
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

// CONFIGURACION DE LA MATRIZ 8*8
#define CANT_PIXEL 64
Adafruit_NeoPixel Pixel(CANT_PIXEL, NEO_PIN, NEO_GRB + NEO_KHZ800);

const byte ANCHO_MATRIZ  = 8;      // pixeles por fila
const byte LARGO_MAX     = 5;      // maximo de celdas de color (nivel 2 = 5)
const byte COLORES_MAX   = 6;      // maximo de colores disponibles
const bool MATRIZ_ZIGZAG = false;  // true si tu matriz invierte el sentido en filas impares
const bool MODO_DEBUG    = false;  // true = muestra la clave por el monitor serie
const unsigned long TIEMPO_NIVEL2_MS = 150000UL; // 2 min 30 seg

const uint8_t PALETA[COLORES_MAX] PROGMEM = {
  {255,   0,   0},   // 0 Rojo
  {  0, 255,   0},   // 1 Verde
  {  0,   0, 255},   // 2 Azul
  {255, 255,   0},   // 3 Amarillo
  {  0, 255, 255},   // 4 Cian
  {255,   0, 255}    // 5 Magenta
};

const char* const NOMBRES[COLORES_MAX] = {
  "Rojo", "Verde", "Azul", "Amarillo", "Cian", "Magenta"
};

// VARIABLES PARA EL MODO DE JUEGO CONTRA RELOJ
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

byte fila_Activa    = 0;      // intento actual
byte columna_Activa = 0;      // celda actual
byte colorSelect    = 0;      // color seleccionado
int  ultimoCLK;               // ultimo estado del pin CLK del encoder
bool juegoTerminado = false;

const int melody[] PROGMEM = {
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_E4, NOTE_D4,
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_C5, NOTE_G4, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4,
  NOTE_D4, NOTE_E4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4
};

const uint8_t noteDurations[] PROGMEM = {
  4, 4, 4, 2, 4, 4, 4, 2,
  4, 4, 4, 2, 4, 4, 1, 4,
  4, 4, 2, 4, 4, 4, 2, 4,
  4, 4, 4, 4, 4, 1
};
const byte CANT_NOTAS = sizeof(melody) / sizeof(melody);

// Control de audio sin bloquear el reloj
enum TipoAudio { AUDIO_NINGUNO, AUDIO_ERROR, AUDIO_VICTORIA, AUDIO_MELODIA };
TipoAudio audioActual = AUDIO_NINGUNO;
byte pasoAudio = 0;
unsigned long tiempoAudio = 0;
unsigned long esperaAudio = 0;

void iniciarAudio(TipoAudio tipo) {
  noTone(PIN_BUZZER);
  audioActual = tipo;
  pasoAudio = 0;
  tiempoAudio = millis();
  esperaAudio = 0;
}

void actualizarAudio() {
  if (audioActual == AUDIO_NINGUNO) return;
  if (millis() - tiempoAudio < esperaAudio) return;

  tiempoAudio = millis();

  if (audioActual == AUDIO_ERROR) {
    if (pasoAudio == 0) {
      tone(PIN_BUZZER, 200, 300);
      esperaAudio = 350;
      pasoAudio++;
    } else if (pasoAudio == 1) {
      tone(PIN_BUZZER, 150, 500);
      esperaAudio = 550;
      pasoAudio++;
    } else {
      noTone(PIN_BUZZER);
      audioActual = AUDIO_NINGUNO;
    }
  } 
  else if (audioActual == AUDIO_VICTORIA) {
    const int notasVic[] = { NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6 };
    const int durVic[]   = { 200, 200, 200, 600 };
    if (pasoAudio < 4) {
      tone(PIN_BUZZER, notasVic[pasoAudio], durVic[pasoAudio]);
      esperaAudio = durVic[pasoAudio] + 50;
      pasoAudio++;
    } else {
      iniciarAudio(AUDIO_MELODIA);
    }
  } 
  else if (audioActual == AUDIO_MELODIA) {
    if (pasoAudio < CANT_NOTAS) {
      int nota = pgm_read_word(&melody[pasoAudio]);
      uint8_t div = pgm_read_byte(&noteDurations[pasoAudio]);
      int duracion = 1000 / div;
      tone(PIN_BUZZER, nota, duracion);
      esperaAudio = (unsigned long)(duracion * 1.30);
      pasoAudio++;
    } else {
      noTone(PIN_BUZZER);
      audioActual = AUDIO_NINGUNO;
    }
  }
}

void sonidoConfirmar() {
  if (audioActual == AUDIO_NINGUNO) {
    tone(PIN_BUZZER, NOTE_A4, 70);
  }
}

void mostrarLinea(byte linea, const char* texto) {
  char buf;
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
  char buf;
  if (tiempoLimite > 0) {
    snprintf(buf, sizeof(buf), "Int:%d/%d T:%3ds", fila_Activa + 1, maxIntentos, segundosRestantes());
  } else {
    snprintf(buf, sizeof(buf), "Intento: %d/%d", fila_Activa + 1, maxIntentos);
  }
  mostrarLinea(0, buf);

  snprintf(buf, sizeof(buf), "Color: %s", NOMBRES[colorSelect]);
  mostrarLinea(1, buf);
}

uint8_t indicePixel(byte f, byte c) {
  if (MATRIZ_ZIGZAG && (f % 2 == 1)) {
    return (f * ANCHO_MATRIZ) + (ANCHO_MATRIZ - 1 - c);
  }
  return (f * ANCHO_MATRIZ) + c;
}

uint32_t colorPaleta(byte i) {
  uint8_t r = pgm_read_byte(&PALETA[i]);
  uint8_t g = pgm_read_byte(&PALETA[i]);
  uint8_t b = pgm_read_byte(&PALETA[i]);
  return Pixel.Color(r, g, b);
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
  static unsigned long ultimoTiempoGiro = 0;
  int8_t giro = 0;
  int estadoCLK = digitalRead(PIN_CLK);

  if (estadoCLK != ultimoCLK && estadoCLK == LOW) {
    if (millis() - ultimoTiempoGiro > 5) {
      giro = (digitalRead(PIN_DT) != estadoCLK) ? 1 : -1;
      ultimoTiempoGiro = millis();
    }
  }
  ultimoCLK = estadoCLK;
  return giro;
}

bool botonPresionado() {
  static bool estadoAnterior = HIGH;
  static unsigned long ultimoCambio = 0;
  bool estadoActual = digitalRead(PIN_SW);
  bool evento = false;

  if (estadoActual != estadoAnterior && (millis() - ultimoCambio > 35)) {
    ultimoCambio = millis();
    if (estadoActual == LOW) {
      evento = true;
    }
    estadoAnterior = estadoActual;
  }
  return evento;
}

void mostrarNivelMenu() {
  mostrarLinea(0, "Elige nivel:");
  if (nivel == 1) mostrarLinea(1, "< 1 Normal >");
  else mostrarLinea(1, "< 2 Reloj >");
}

void menuNivel() {
  iniciarAudio(AUDIO_NINGUNO);
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
    largoClave      = 4;
    maxIntentos     = 8;
    cantColores     = 6;
    filasPorIntento = 1;
    tiempoLimite    = 0;
  } else {
    largoClave      = 5;
    maxIntentos     = 4;
    cantColores     = 5;
    filasPorIntento = 2;
    tiempoLimite    = TIEMPO_NIVEL2_MS;
  }
  ultimoCLK = digitalRead(PIN_CLK);
}

void generarClave() {
  for (byte i = 0; i < largoClave; i++) {
    claveSecreta[i] = random(0, cantColores);
  }

  if (MODO_DEBUG) {
    Serial.print(F("Clave: "));
    for (byte i = 0; i < largoClave; i++) {
      Serial.print(claveSecreta[i]);
      Serial.print(' ');
    }
    Serial.println();
  }
}

void reiniciarJuego() {
  Pixel.clear();
  generarClave();

  fila_Activa = 0;
  columna_Activa = 0;
  colorSelect = 0;
  juegoTerminado = false;
  inicioJuego = millis();
  segPrevios = -1;

  dibujarColor(fila_Activa, columna_Activa, colorSelect);
  actualizarLCD();
}

void ganar() {
  char buf;
  snprintf(buf, sizeof(buf), "GANASTE! (%d int)", fila_Activa + 1);
  mostrarLinea(0, buf);
  mostrarLinea(1, "Presiona boton");

  iniciarAudio(AUDIO_VICTORIA);
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

  iniciarAudio(AUDIO_ERROR);
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

  dibujarPistas(fila_Activa, exactos, parciales);

  if (exactos == largoClave) {
    ganar();
    return;
  }

  iniciarAudio(AUDIO_ERROR);
  fila_Activa++;

  if (fila_Activa >= maxIntentos) {
    perder(false);
  } else {
    columna_Activa = 0;
    colorSelect = 0;
    dibujarColor(fila_Activa, columna_Activa, colorSelect);
    actualizarLCD();
  }
}

void confirmarColor() {
  combinacionJugador[columna_Activa] = colorSelect;
  sonidoConfirmar();
  columna_Activa++;

  if (columna_Activa >= largoClave) {
    evaluarIntento();
  } else {
    dibujarColor(fila_Activa, columna_Activa, colorSelect);
  }
}

void setup() {
  pinMode(PIN_CLK, INPUT_PULLUP);
  pinMode(PIN_DT, INPUT_PULLUP);

  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  if (MODO_DEBUG) Serial.begin(9600);
  Pixel.begin();
  Pixel.setBrightness(50);
  Pixel.clear();
  Pixel.show();
  // Inicializacion de LCD normal de 16x2
  LCD.begin(16, 2);
  mostrarLinea(0, "-M-Bienvenido-M-");
  mostrarLinea(1, "Iniciando...");
  delay(1500);
  menuNivel();
  reiniciarJuego();
}
void loop() {
  actualizarAudio();
  if (juegoTerminado) {
  if (botonPresionado()) {
  menuNivel();
  reiniciarJuego();
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
      actualizarLCD(); // <-- ADENTRO del if (solo actualiza 1 vez por segundo)
    }
  }
}
actualizarLCD();
  
  }
  int8_t giro = leerEncoder();
  if (giro != 0) {
  colorSelect = (colorSelect + cantColores + giro) % cantColores;
  dibujarColor(fila_Activa, columna_Activa, colorSelect);
  actualizarLCD();
  }
  if (botonPresionado()) {
  confirmarColor();
  }
}
