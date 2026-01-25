#ifndef CHARGER_H
#define CHARGER_H

#include "Particle.h"

#define MP2672A_ADDR    0x4B

// MP2672A Registers
#define REG00_VBATT     0x00  // Battery voltage, charge config, SYS voltage
#define REG01_BALANCE   0x01  // Cell balance, charge current
#define REG02_TIMER     0x02  // Timer settings
#define REG03_STATUS    0x03  // Status register (read-only)
#define REG04_FAULT     0x04  // Fault register (read-only)

class Charger {
public:
    static Charger& instance();

    bool begin();
    void readAllRegisters();
    uint8_t readReg(uint8_t reg);

private:
    Charger() = default;
    bool _initialized = false;
};

#endif
