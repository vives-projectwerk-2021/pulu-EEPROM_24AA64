#include "EEPROM_24AA64.h"
#include <array>

EEPROM_24AA64::EEPROM_24AA64(PinName sda, PinName scl, uint16_t address):
    i2c(sda, scl) {
    this->address = address;
}

bool EEPROM_24AA64::read(char* data, uint16_t length, uint16_t start_address) {
    if(!is_valid_address_range(start_address, length)) {
        EEPROM_24AA64_DEBUG("read: FAILURE");
        return true;
    }
    char read_address[2] = {(char)(start_address>>8), (char)start_address};
    bool ack;
    EEPROM_24AA64_DEBUG("reading from address: %d (0x%X 0x%X)-- length: %d", start_address, read_address[0], read_address[1], length);
    ack = i2c.write(address, read_address, 2, true);
    if(ack) {
        EEPROM_24AA64_DEBUG("read: FAILURE while writing address");
        return true;
    }
    ack = i2c.read(address, data, length);
    if(ack) {
        EEPROM_24AA64_DEBUG("read: FAILURE while reading");
        return true;
    }
    wait_till_ready();
    EEPROM_24AA64_DEBUG("read: DONE");
    return false;
}

bool EEPROM_24AA64::write(char* data, uint16_t length, uint16_t start_address) {
    if(!is_valid_address_range(start_address, length)) {
        EEPROM_24AA64_DEBUG("write: FAILURE");
        return true;
    }
    bool ack;
    char* pointer = data;
    uint16_t write_address = start_address;
    uint8_t first_block_size = PAGE_SIZE-(start_address%PAGE_SIZE);
    if(first_block_size>length) { first_block_size = length;}
    uint8_t blocks = (length-first_block_size)/PAGE_SIZE;
    uint8_t last_block_size = (length-first_block_size) - (blocks*PAGE_SIZE);
    EEPROM_24AA64_DEBUG("writing to address: %d -- total length: %d -- first block: %d bytes - blocks: %d x %d bytes - last block: %d bytes", start_address, length, first_block_size, blocks, PAGE_SIZE, last_block_size);
    // first block
    if(first_block_size) {
        ack = write_block(pointer, first_block_size, write_address);
        pointer += first_block_size;
        write_address += first_block_size;
        if(ack) {
            EEPROM_24AA64_DEBUG("write: FAILURE (first block)");
            return true;
        }
        EEPROM_24AA64_DEBUG("write: SUCCESS (first block)");
    }
    // middle blocks
    if(blocks) {
        for(uint8_t block = 0; block<blocks; block++) {
            ack = write_block(pointer, PAGE_SIZE, write_address);
            pointer += PAGE_SIZE;
            write_address += PAGE_SIZE;
            if(ack) {
                EEPROM_24AA64_DEBUG("write: FAILURE (block %d)", block);
                return true;
            }
            EEPROM_24AA64_DEBUG("write: SUCCESS (block %d)", block);
        }
    }
    // last block
    if(last_block_size) {
        ack = write_block(pointer, last_block_size, write_address);
        if(ack) {
            EEPROM_24AA64_DEBUG("write: FAILURE (last block)");
            return true;
        }
        EEPROM_24AA64_DEBUG("write: SUCCESS (last block)");
    }
    EEPROM_24AA64_DEBUG("write: DONE");
    return false;
}

bool EEPROM_24AA64::clear() {
    std::array<char, PAGE_SIZE> data;
    data.fill(0xFF);
    bool ack;
    uint16_t max_pages = MAX_ADDRESS/PAGE_SIZE;
    for(uint16_t page = 0; page<max_pages; page++) {
        ack = write_block(data.data(), data.size(), page*PAGE_SIZE);
        if(ack) {
            EEPROM_24AA64_DEBUG("clear: FAILURE (page %d/%d)", page+1, max_pages);
            return true;
        }
        EEPROM_24AA64_DEBUG("clear: SUCCESS (page %d/%d)", page+1, max_pages);
    }
    EEPROM_24AA64_DEBUG("clear: DONE");
    return false;
}

bool EEPROM_24AA64::write_block(char* data, uint8_t length, uint16_t write_address) {
    bool ack;
    char* buffer = (char*)malloc(length+2);
    buffer[0] = (char)(write_address>>8);
    buffer[1] = (char)write_address;
    for(uint8_t i = 0; i<length; i++) {
        buffer[i+2] = data[i];
    }
    ack = i2c.write(address, buffer, length+2);
    if(ack) {
        free(buffer);
        return true;
    }
    wait_till_ready();
    free(buffer);
    return false;
}

void EEPROM_24AA64::wait_till_ready() {
    bool ack;
    char cmd[2] = {0,0};
    EEPROM_24AA64_DEBUG("waiting till ready");
    do {
        ack = i2c.write(address, cmd, 2);
        wait_us(500);
    } while(ack != 0);
    EEPROM_24AA64_DEBUG("ready");
}

bool EEPROM_24AA64::is_valid_address_range(uint16_t address, uint16_t length) {
    if(address>MAX_ADDRESS) {
        EEPROM_24AA64_DEBUG("ADDRESS FAILURE: start_address (%d) > %d", address, MAX_ADDRESS);
        return false;
    }
    uint16_t end_address = address + length;
    if(end_address>MAX_ADDRESS) {
        EEPROM_24AA64_DEBUG("ADDRESS FAILURE: end_address (%d) > %d", end_address, MAX_ADDRESS);
        return false;
    }
    if(end_address<address) {
        EEPROM_24AA64_DEBUG("ADDRESS FAILURE: end_address (%d) < start_address (%d)", end_address, address);
        return false;
    }
    return true;
}