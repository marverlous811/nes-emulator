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
    //hold on to the raw data and delete later
    this->raw_data = data;
    this->data_len = data_len;
    
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
    
    // 0       7
    // ---------
    // NNNN FTBM
    
    // N: Lower 4 bits of the Mapper
    // F: has_4screen
    // T: has_trainer
    // B: has_battery
    // M: mirror_type (0 = horizontal, 1 = vertical)
    
    this->flags.has_4screen = nth_bit(data[5], 4);
    this->flags.has_trainer = nth_bit(data[5], 5);
    this->flags.has_battery = nth_bit(data[5], 6);
    this->flags.mirror_type = nth_bit(data[5], 7);
    
    // 0       7
    // ---------
    // NNNN xxPV
    
    // N: Upper 4 bits of the mapper
    // P: is_PC10
    // V: is_VS
    // x: is_NES2 (when xx == 10)
    this->flags.is_nes2 = nth_bit(data[6], 4) && !nth_bit(data[6], 5);
    this->flags.is_PC10 = nth_bit(data[6], 6);
    this->flags.is_VS   = nth_bit(data[6], 7);
    
    this->mapper = data[5] >> 4 & (data[6] & 0x00FF);
    
    // Find base addresses for the various ROM sections in the data
    // iNES is laid out as follows:
    
    // Section             | Multiplier    | Size
    // --------------------|---------------|--------
    // Header              | 1             | 0x10
    // Trainer ROM         | has_trainer   | 0x200
    // Program ROM         | prg_rom_pages | 0x4000
    // Character ROM       | chr_rom_pages | 0x2000
    // PlayChoice INST-ROM | is_PC10       | 0x2000
    // PlayChoice PROM     | is_PC10       | 0x10
    
    this->roms.trn_rom = data + 0x10;
    this->roms.prg_rom = this->roms.trn_rom + 0x200  * this->flags.has_trainer;
    this->roms.chr_rom = this->roms.prg_rom + 0x4000 * this->flags.prg_rom_pages;
    this->roms.pci_rom = this->roms.chr_rom + 0x2000 * this->flags.chr_rom_pages;
    this->roms.pc_prom = this->roms.pci_rom + 0x2000 * this->flags.is_PC10;
}

INES::~INES() {
    delete[] raw_data;
}
