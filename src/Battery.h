#ifndef BATTERY_H
#define BATTERY_H

#include "Particle.h"

#define MAX17049_ADDR   0x36
#define BATT_ALERT_PIN  A1

class Battery {
public:
    static Battery& instance();

    bool begin();
    float getSoC();       // State of Charge (0-100%)
    float getVoltage();   // Battery voltage
    uint16_t getRawVoltage();  // Raw VCELL register

    void quickStart();   // Reset SOC calculation

private:
    Battery() = default;
    bool _initialized = false;
    uint16_t readReg(uint8_t reg);
    void writeReg(uint8_t reg, uint16_t value);
};

#endif
