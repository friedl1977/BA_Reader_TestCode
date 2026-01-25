#ifndef RFID_H
#define RFID_H

#include "Particle.h"
#include "DFRobot_PN532.h"

// Pin definitions
#define RF_V1       D5
#define RF_V2       D6
#define RF_V3       D7
#define RF_V4       D26
#define PN532_IRQ   A3
#define PN532_RST   A2

// =====================================================
// CHANGE THIS TO TEST DIFFERENT ANTENNAS (1-12)
// Set to 0 for ALL OFF (no antenna)
// =====================================================
#define TEST_ANTENNA  0

class RFID {
public:
    static RFID& instance();

    bool begin();
    bool scan(uint8_t* uid);

private:
    RFID() = default;
    DFRobot_PN532_IIC* _nfc = nullptr;
    bool _initialized = false;
};

#endif
