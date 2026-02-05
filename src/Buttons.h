#ifndef __BUTTONS_H
#define __BUTTONS_H

#include "Particle.h"

// Button pin definitions (active LOW with pull-up resistors)
#define BUTTON_1_PIN  A5
#define BUTTON_2_PIN  A4
#define BUTTON_3_PIN  A7
#define BUTTON_4_PIN  A6

class Buttons {
public:
    static Buttons &instance();

    void begin();
    void update();  // Call this in loop()

    // Check if a specific button was just pressed (returns true once per press)
    bool wasPressed(int buttonNum);

private:
    Buttons();

    static Buttons *_instance;

    bool lastState[4];  // Previous button states
    unsigned long lastDebounce[4];  // Debounce timers
    bool buttonPressed[4];  // Flag for button press events
    static const unsigned long DEBOUNCE_MS = 50;

    void checkButton(int buttonNum, pin_t pin);
};

#endif /* __BUTTONS_H */
