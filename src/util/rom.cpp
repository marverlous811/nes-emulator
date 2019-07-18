//
// Created by Ows on 2019-07-18.
//

#include "rom.hpp"
#include <assert.h>

ROM::ROM(uint32 rom_size, const uint8 *content) {
    if(rom_size > 0xFFFF + 1){
        rom_size = 0xFFFF + 1;
    }

    this->size = rom_size;
    this->rom = new uint8[rom_size];

    for(uint32 addr = 0; addr < this->size; addr++){
        this->rom[addr] = rom[addr];
    }
}

ROM::~ROM() {
    delete[] this->rom;
}

uint8 ROM::read(uint16 addr) { return this->peek(addr); }
uint8 ROM::read(uint16 addr) const { return this->peek(addr); }

uint8 ROM::peek(uint16 addr) const {
    assert(addr < this->size);
    return this->rom[addr];
}

void ROM::write(uint16 addr, uint8 val) {
    (void) addr;
    (void) val;
}