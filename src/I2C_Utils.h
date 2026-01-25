#ifndef I2C_UTILS_H
#define I2C_UTILS_H

#include "Particle.h"

namespace I2C_Utils {
    void scanBus();
    bool devicePresent(uint8_t address);
}

#endif
