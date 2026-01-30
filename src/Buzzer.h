#ifndef BUZZER_H
#define BUZZER_H

#include "Particle.h"

#define BUZZER_PIN  A0

class Buzzer {
public:
    static Buzzer& instance() {
        static Buzzer _instance;
        return _instance;
    }

    void init();
    void startBuzzer(int frequency);
    void stopBuzzer();
    void playNote(int frequency, int duration);
    void playSuccessTone();
    void playFailureTone();
    void playSleepTone();

private:
    Buzzer() = default;
    ~Buzzer() = default;
    Buzzer(const Buzzer&) = delete;
    Buzzer& operator=(const Buzzer&) = delete;

    bool initialized = false;
};

#endif
