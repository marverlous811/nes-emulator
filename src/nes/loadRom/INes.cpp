//
//  INes.cpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "INes.hpp"
#include <iostream>

INES::INES(const uint8* data, uint32 data_len) {
    (void)data_len;
    
    if(data == nullptr){
        this->is_valid = false;
        return;
    }
    
    //check that the data is iNes compatible
    this->is_valid = (data[0] == 'N'
                      && data[1] == 'E'
                      && data[2] == 'S');
    
    if(!this->is_valid) return;
    
    // Parse the rest of the header
    this->flags.prg_rom_pages = data[3];
    this->flags.chr_rom_pages = data[4];
    
    //Can't use rom without prg_rom
    if(this->flags.prg_rom_pages == 0){
        this->is_valid = false;
        return;
    }
    
    // 7       0
    // ---------
    // NNNN FTBM
    
    // N: Lower 4 bits of the Mapper
    // F: has_4screen
    // T: has_trainer
    // B: has_battery
    // M: mirror_type (0 = horizontal, 1 = vertical)
    this->flags.has_4screen = nth_bit(data[5], 3);
    this->flags.has_trainer = nth_bit(data[5], 2);
    this->flags.has_battery = nth_bit(data[5], 1);
    this->flags.mirror_type = nth_bit(data[5], 0);

    
    // 7       0
    // ---------
    // NNNN xxPV
    
    // N: Upper 4 bits of the mapper
    // P: is_PC10
    // V: is_VS
    // x: is_NES2 (when xx == 10)
    this->flags.is_nes2 = nth_bit(data[6], 3) && !nth_bit(data[6], 2);
    this->flags.is_PC10 = nth_bit(data[6], 1);
    this->flags.is_VS   = nth_bit(data[6], 0);
    
    this->mapper = data[5] >> 4 & (data[6] & 0x00FF);

    // Find addresses for the various ROM sections in the data, and throw them
    // into some ROM ADTs
    // iNES is laid out as follows:
    
    // Section             | Multiplier    | Size
    // --------------------|---------------|--------
    // Header              | 1             | 0x10
    // Trainer ROM         | has_trainer   | 0x200
    // Program ROM         | prg_rom_pages | 0x4000
    // Character ROM       | chr_rom_pages | 0x2000
    // PlayChoice INST-ROM | is_PC10       | 0x2000
    // PlayChoice PROM     | is_PC10       | 0x10
    
    const uint8* data_p = data + 0x10; //move past header
    if(this->flags.has_trainer){
        this->roms.trn_rom = new ROM(0x200, data_p);
        data_p += 0x200;
    }
    else{
        this->roms.trn_rom = nullptr;
    }

    this->roms.prg_rom = new ROM*[this->flags.prg_rom_pages];
    for (uint i = 0; i < this->flags.prg_rom_pages; i++){
        this->roms.prg_rom[i] = new ROM(0x4000, data_p);
        data_p += 0x4000;
    }

    this->roms.chr_rom = new ROM*[this->flags.chr_rom_pages];
    for(uint i = 0; i< this->flags.chr_rom_pages; i++){
        this->roms.chr_rom[i] = new ROM(0x2000, data_p);
        data_p += 0x2000;
    }

    if(this->flags.is_PC10){
        this->roms.pci_rom = new ROM(0x2000, data_p);
        data_p += 0x2000;
        this->roms.pc_prom = new ROM(0x10, data_p);
    }
    else{
        this->roms.pci_rom = nullptr;
        this->roms.pc_prom = nullptr;
    }
}

INES::~INES() {
    for(uint i = 0; i < this->flags.prg_rom_pages; i++)
        delete this->roms.prg_rom[i];
    delete[] this->roms.prg_rom;

    for (uint i = 0; i < this->flags.chr_rom_pages; i++)
        delete this->roms.chr_rom[i];
    delete[] this->roms.chr_rom;


    delete this->roms.trn_rom;
    delete this->roms.pci_rom;
    delete this->roms.pc_prom;
}
