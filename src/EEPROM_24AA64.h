#pragma once

#include "mbed.h"

class EEPROM_24AA64 {
    public:
        EEPROM_24AA64(PinName sda, PinName scl, uint16_t address);
    
        bool read(char* data, uint8_t length, uint16_t start_address = 0); // success = 0
        bool write(char* data, uint8_t length, uint16_t start_address);   // success = 0
    
    private:
        I2C i2c;
        uint16_t address;
};