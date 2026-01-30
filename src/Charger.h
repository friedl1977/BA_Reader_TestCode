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

// Charging status from REG03 bits[5:4]
enum class ChargeStatus : uint8_t {
    NOT_CHARGING = 0,
    PRE_CHARGE = 1,
    FAST_CHARGE = 2,
    CHARGE_DONE = 3
};

class Charger {
public:
    static Charger& instance();

    bool begin();
    bool isAvailable() const { return _initialized; }

    // Status readings from REG03
    ChargeStatus getChargeStatus();
    const char* getChargeStatusString();
    bool isCellBalancing();       // CELL_BAL: cell balancing active
    bool isInputPowerGood();      // VIN_STAT: input voltage good
    bool isThermalRegulation();   // THERM_STAT: charge current reduced due to IC temp
    bool isSystemMinVoltage();    // VSYS_STAT: VSYS < VSYS_MIN

    // Fault readings from REG04
    bool hasWatchdogFault();
    bool hasInputOVP();
    bool hasThermalShutdown();
    bool hasBatteryFault();
    bool hasChargeTimerFault();
    bool hasAnyFault();

    // For debugging
    void readAllRegisters();
    uint8_t readReg(uint8_t reg);

private:
    Charger() = default;
    bool _initialized = false;
};

#endif
