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

const int buzzerPin = 8;

int melody[] = {
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_E4, NOTE_D4,
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_C5, NOTE_G4, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4,
  NOTE_D4, NOTE_E4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4
};

int noteDurations[] = {
  4, 4, 4, 2, 4, 4, 4, 2,
  4, 4, 4, 2, 4, 4, 1, 4,
  4, 4, 2, 4, 4, 4, 2, 4,
  4, 4, 4, 4, 4, 1
};

void sonidoError() {
  tone(buzzerPin, 200, 300);
  delay(350);

  tone(buzzerPin, 150, 500);
  delay(550);

  noTone(buzzerPin);
}

void sonidoVictoria() {
  tone(buzzerPin, NOTE_C5, 200);
  delay(250);

  tone(buzzerPin, NOTE_E5, 200);
  delay(250);

  tone(buzzerPin, NOTE_G4, 200);
  delay(250);

  tone(buzzerPin, NOTE_C5, 600);
  delay(650);

  noTone(buzzerPin);
}

void tocarMelodia() {
  int cantidad = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < cantidad; i++) {
    int duracion = 1000 / noteDurations[i];

    tone(buzzerPin, melody[i], duracion);
    delay(duracion * 1.30);

    noTone(buzzerPin);
  }
}

void setup() {
  tocarMelodia();

  delay(1000);
  sonidoError();

  delay(1000);
  sonidoVictoria();
}

void loop() {

}