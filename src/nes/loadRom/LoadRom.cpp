//
//  LoadRom.cpp
//  MyNes
//
//  Created by Ows on 6/27/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "LoadRom.hpp"
#include <iostream>

INES::INES(std::istream& data_stream) {
    //Read in data from data stream of unknow size
    uint8* data = nullptr;
    const uint chunk_size = 0x4000;
    for(uint chunk = 0; data_stream; chunk++){
        //allocate new data
        uint8* new_data = new uint8[chunk_size * (chunk + 1)];
        data_len = chunk_size * (chunk + 1);
        
        //copy old data into the new data
        for(uint i = 0; i < chunk_size * chunk; i++){
            new_data[i] = data[i];
        }
        
        delete data;
        data = new_data;
        
        // Read another chunk into the data
        data_stream.read((char*)data + chunk_size * chunk, chunk_size);
    }
    
    //hold on to the raw data and delete later
    this->raw_data = data;
    
    //check that the data is iNes compatible
    this->is_valid = (data[0] == 'N'
                      && data[1] == 'E'
                      && data[2] == 'S');
    
    if(!this->is_valid) return;
    
    // Parse the rest of the header
    this->flags.prg_rom_pages = data[3];
    this->flags.chr_rom_pages = data[4];
    
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
    // Header              | 1             | 0xF
    // Trainer ROM         | has_trainer   | 0x200
    // Program ROM         | prg_rom_pages | 0x4000
    // Character ROM       | chr_rom_pages | 0x2000
    // PlayChoice INST-ROM | is_PC10       | 0x2000
    // PlayChoice PROM     | is_PC10       | 0xF
    
    this->roms.trn_rom = data + 0xF;
    this->roms.prg_rom = this->roms.trn_rom + 0x200  * this->flags.has_trainer;
    this->roms.chr_rom = this->roms.prg_rom + 0x4000 * this->flags.prg_rom_pages;
    this->roms.pci_rom = this->roms.chr_rom + 0x2000 * this->flags.chr_rom_pages;
    this->roms.pc_prom = this->roms.pci_rom + 0x2000 * this->flags.is_PC10;
}

INES::~INES() {
    delete raw_data;
}

INES::INES(const INES& other) {
    *this = other;
}

INES& INES::operator= (const INES& other) {
    if(this == &other) return *this;
    
    // Allocate space to copy data into
    this->raw_data = new uint8[other.data_len];
    this->data_len = other.data_len;
    
    // Copy old data into the new data
    for(uint i = 0; i < this->data_len; i++)
        this->raw_data[i] = other.raw_data[i];
    
    // Move over rest of data
    this->mapper   = other.mapper;
    this->flags    = other.flags;
    this->roms     = other.roms;
    this->is_valid = other.is_valid;

    return *this;
}
