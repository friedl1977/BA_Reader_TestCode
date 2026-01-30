#ifndef BATTERY_H
#define BATTERY_H

#include "Particle.h"

#define MAX17049_ADDR   0x36
#define BATT_ALERT_PIN  A1

class Battery {
public:
    static Battery& instance();

    bool begin();
    float getSoC();           // State of Charge from ModelGauge (0-100%)
    float getVoltage();       // Battery voltage (V)
    uint16_t getRawVoltage(); // Raw VCELL register
    float getChangeRate();    // Charge/discharge rate (%/hr)

    void quickStart();        // Reset SOC calculation (use sparingly!)
    void setRCOMP(uint8_t rcomp);  // Set compensation value
    uint8_t getRCOMP();       // Get current RCOMP value
    void sleep();             // Enter low-power sleep mode
    void wake();              // Exit sleep mode

    // For debugging
    void printRegisters();

private:
    Battery() = default;
    bool _initialized = false;
    uint16_t readReg(uint8_t reg);
    void writeReg(uint8_t reg, uint16_t value);
};

#endif
