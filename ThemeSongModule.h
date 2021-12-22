/**
 * ThemeSongModule.h - Plays the theme song!!
 */
#include "Melody.h"

int tempo = 200;
int buzzer = BUZZER_PIN;

// 2 values per note, pitch & duration, so notecount is divided by 2 at the end
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;

int thisNote = 0;

long long noteChangeTime = 0;

void themeSongLoop() {
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)

  if (thisNote >= notes * 2) {
    thisNote = 0;
    return;
  }

  long long currentTime = millis();

  // if should still play current note - return
  if (currentTime - noteChangeTime  < noteDuration) {
    return;
  }

  noteChangeTime = currentTime;
  
  // need to change the note
  noTone(buzzer);

  // calculates the duration of each note
  divider = melody[thisNote + 1];
  if (divider > 0) {
    // regular note, just proceed
    noteDuration = (wholenote) / divider;
  } else if (divider < 0) {
    // dotted notes are represented with negative durations
    noteDuration = (wholenote) / abs(divider);
    noteDuration *= 1.5; // dotted notes are longer
  }

  // 10% pause, note change time
  tone(buzzer, melody[thisNote], noteDuration);
  
  thisNote += 2;

}
