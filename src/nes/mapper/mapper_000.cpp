//
//  mapper_000.cpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "mapper_000.hpp"
#include <assert.h>

Mapper_000::Mapper_000(const INES& rom_file) : Mapper(rom_file){
    if(this->rom_file.flags.prg_rom_pages == 2){
        this->lo_rom = this->rom_file.roms.prg_rom + 0x0000;
        this->hi_rom = this->rom_file.roms.prg_rom + 0x2000;
    }
    else{
        //lo_rom == hi_rom
        this->lo_rom = this->rom_file.roms.prg_rom + 0x0000;
        this->hi_rom = this->rom_file.roms.prg_rom + 0x0000;
    }
}

Mapper_000::~Mapper_000() {}

uint8 Mapper_000::peek(uint16 addr) const {
    assert(addr >= 0x4020); //remove once mapping PPU

    if(in_range(addr, 0x4020, 0x5FFF)){} //Nothing in "Cartridge Expansion ROM"
    else if(in_range(addr, 0x6000, 0x7FFF)){} //Nothing in SRAM
    else if(in_range(addr, 0x8000, 0xBFFF)){ return this->lo_rom[addr - 0x8000]; }
    else if(in_range(addr, 0xC000, 0xFFFF)){ return this->hi_rom[addr - 0xC000]; }

    return 0x00;
}

uint8 Mapper_000::read(uint16 addr){
    return this->peek(addr);
}

void Mapper_000::write(uint16 addr, uint8 val){
    assert(addr >= 0x4020); //remove once mapping PPU
    
    // 000 is a totally static mapper.
    // No registers, no siwtching, no nothing.
}
