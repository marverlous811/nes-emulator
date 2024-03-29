//
//  INes.hpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef INes_hpp
#define INes_hpp

#include "../../util/rom.hpp"
#include "../../util/util.h"

// iNES file container
// https://wiki.nesdev.com/w/index.php/INES
struct INES{
private:
    const uint8* raw_data;
    uint32 data_len;
    
public:
    uint8 mapper; //Mapper number
    bool is_valid;
    
    //ROM
    struct{
        ROM** prg_rom; //start of pro_rom bank (16k for each)
        ROM** chr_rom; //start of chr_rom bank (8k for each)

        ROM* trn_rom; //Start of Trainer
        ROM* pci_rom; //start of play choice INST-ROM
        ROM* pc_prom;  //start play choice PROM
    } roms;
    
    struct{
        uint8 prg_rom_pages;  //Num 16k program ROM pages
        uint8 chr_rom_pages;  //Num 8k char ROM pages

        bool has_4screen;     // Nametable mirroring is FourScreen
        bool mirror_type;     // 0 = horizontal mirror, 1 = vertical
        bool has_trainer;     //Has 512 byte trainer at 7000 - 71FFh
        bool has_battery;     //Has battery backed SRAM at 6000 - 7FFF
        
        bool is_PC10;         //is a PC-10 game
        bool is_VS;           //is a VS. game
        
        bool is_nes2;         //is using NES 2.0 extension
    } flags;
    
    ~INES();
    INES(const uint8* data, uint32 data_len);
};

#endif /* INes_hpp */
