#pragma once

#include "mbed.h"

#define EEPROM_24AA64_DEBUG(x, ...)   //set as comment to enable debugging
#ifndef EEPROM_24AA64_DEBUG
#define EEPROM_24AA64_DEBUG(x, ...) printf("[EEPROM_24AA64]\t" x "\r\n", ##__VA_ARGS__)
#endif

class EEPROM_24AA64 {
    public:
        EEPROM_24AA64(PinName sda, PinName scl, uint16_t address);
    
        bool read(char* data, uint16_t length, uint16_t start_address = 0); // success = 0
        bool write(char* data, uint16_t length, uint16_t start_address);   // success = 0
        bool clear();
    
    private:
        bool write_block(char* data, uint8_t length, uint16_t write_address);
        void wait_till_ready();
        bool is_valid_address_range(uint16_t address, uint16_t length);
    
    private:
        I2C i2c;
        uint16_t address;

    public:
        static const uint16_t MAX_ADDRESS = 0x1FFF;
        static const uint8_t PAGE_SIZE = 32;
};