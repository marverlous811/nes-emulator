//
//  mapper_000.cpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "mapper_000.hpp"
#include <assert.h>
#include <nes/loadRom/INes.hpp>

Mapper_000::Mapper_000(const INES& rom_file) : Mapper(rom_file){
    auto& roms = this->rom_file.roms;
    if(this->rom_file.flags.prg_rom_pages == 2){
        this->lo_rom = roms.prg_rom[0];
        this->hi_rom = roms.prg_rom[1];
    }
    else{
        //lo_rom == hi_rom
        this->lo_rom = roms.prg_rom[0];
        this->hi_rom = roms.prg_rom[0];
    }

    //CHR ROM
    // If there was no chr_rom, then intitialize chr_ram
    if(this->rom_file.flags.chr_rom_pages == 0){
        this->chr_ram = new Ram(0x2000);
        this->chr_rom = nullptr;
    }
    else{
        this->chr_ram = nullptr;
        this->chr_rom = roms.chr_rom[0];
    }
}

Mapper_000::~Mapper_000(){
    delete this->chr_ram;
}

uint8 Mapper_000::peek(uint16 addr) const {
    // Wired to the PPU MMU
    if(in_range(addr, 0x0000, 0x1FFF))
        return this->chr_ram != nullptr ? this->chr_ram->peek(addr) : this->chr_rom->peek(addr);

    if(in_range(addr, 0x4020, 0x5FFF)){ return 0x00;} //Nothing in "Cartridge Expansion ROM"
    else if(in_range(addr, 0x6000, 0x7FFF)){ return 0x00; } //Nothing in SRAM
    else if(in_range(addr, 0x8000, 0xBFFF)){ return this->lo_rom->peek(addr - 0x8000); }
    else if(in_range(addr, 0xC000, 0xFFFF)){ return this->hi_rom->peek(addr - 0xC000); }

    assert(false);
    return 0;
}

uint8 Mapper_000::read(uint16 addr){
    return this->peek(addr);
}

void Mapper_000::write(uint16 addr, uint8 val){
   if(this->chr_ram != nullptr)
       this->chr_ram->write(addr, val);
}
