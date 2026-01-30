#include "Buzzer.h"

void Buzzer::init() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    initialized = true;
}

void Buzzer::startBuzzer(int frequency) {
    if (!initialized) return;

    pinMode(BUZZER_PIN, OUTPUT);
    if (frequency > 0) {
        tone(BUZZER_PIN, frequency);
    } else {
        noTone(BUZZER_PIN);
        digitalWrite(BUZZER_PIN, LOW);
    }
}

void Buzzer::stopBuzzer() {
    if (!initialized) return;
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::playNote(int frequency, int duration) {
    if (!initialized) return;
    startBuzzer(frequency);
    delay(duration);
    stopBuzzer();
    delay(20);
}

void Buzzer::playSuccessTone() {
    if (!initialized) return;
    playNote(2200, 70);
    playNote(2700, 70);
    playNote(3000, 70);
}

void Buzzer::playFailureTone() {
    if (!initialized) return;
    playNote(2900, 120);
    playNote(1900, 200);
}

void Buzzer::playSleepTone() {
    if (!initialized) return;
    // Descending tone pattern to indicate going to sleep
    playNote(2500, 100);
    playNote(2000, 100);
    playNote(1500, 100);
    playNote(1000, 200);
}
