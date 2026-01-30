#include "Buttons.h"

Buttons *Buttons::_instance = nullptr;

Buttons &Buttons::instance() {
    if (!_instance) {
        _instance = new Buttons();
    }
    return *_instance;
}

Buttons::Buttons() {
    for (int i = 0; i < 4; i++) {
        lastState[i] = HIGH;  // Pull-up means default HIGH
        lastDebounce[i] = 0;
    }
}

void Buttons::begin() {
    pinMode(BUTTON_1_PIN, INPUT);
    pinMode(BUTTON_2_PIN, INPUT);
    pinMode(BUTTON_3_PIN, INPUT);
    pinMode(BUTTON_4_PIN, INPUT);

    Serial.println("Buttons initialized");
}

void Buttons::update() {
    checkButton(1, BUTTON_1_PIN);
    checkButton(2, BUTTON_2_PIN);
    checkButton(3, BUTTON_3_PIN);
    checkButton(4, BUTTON_4_PIN);
}

void Buttons::checkButton(int buttonNum, pin_t pin) {
    int index = buttonNum - 1;
    bool currentState = digitalRead(pin);

    if (currentState != lastState[index]) {
        if (millis() - lastDebounce[index] > DEBOUNCE_MS) {
            lastDebounce[index] = millis();
            lastState[index] = currentState;

            if (currentState == LOW) {
                Serial.printlnf("Button %d pressed", buttonNum);
            }
        }
    }
}
