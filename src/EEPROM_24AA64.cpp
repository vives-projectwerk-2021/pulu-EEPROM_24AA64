#include "EEPROM_24AA64.h"

EEPROM_24AA64::EEPROM_24AA64(PinName sda, PinName scl, uint16_t address):
    i2c(sda, scl) {
    this->address = address;
}

bool EEPROM_24AA64::read(char* data, uint8_t length, uint16_t start_address) {
    char address[2] = {(char)(start_address>>8), (char)start_address};
    if( i2c.write(this->address, address, 2, true) ) {
        return true;
    }
    return i2c.read(this->address, data, length);
}

bool EEPROM_24AA64::write(char* data, uint8_t length, uint16_t start_address) {
    char buffer[length+2] = {(char)(start_address>>8), (char)start_address};
    memcpy(buffer+2, data, length);
    if( i2c.write(address, buffer, length+2) ) {
        return true;
    }
    wait_us(5000); //wait till write cycle ends (5ms)
    return false;
}