//
//  LoadRom.hpp
//  MyNes
//
//  Created by Ows on 6/27/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef LoadRom_hpp
#define LoadRom_hpp

#include <istream>
#include "../../util/util.h"

struct INES{
private:
    uint8* raw_data;
    uint32 data_len;
    
public:
    uint8 mapper; //Mapper number
    bool is_valid;
    
    //ROM
    struct{
        uint8* prg_rom; //start of pro_rom bank (16k for each)
        uint8* chr_rom; //start of chr_rom bank (8k for each)
        
        uint8* trn_rom; //Start of Trainer
        uint8* pci_rom; //start of play choice INST-ROM
        uint8* pc_prom;  //start play choice PROM
    } roms;
    
    struct{
        uint8 prg_rom_pages;  //Num 16k program ROM pages
        uint8 chr_rom_pages;  //Num 8k char ROM pages
        
        bool mirror_type;     // 0 = horizontal mirror, 1 = vertical
        bool has_4screen;     //Uses 4 screen mode
        bool has_trainer;     //Has 512 byte trainer at 7000 - 71FFh
        bool has_battery;     //Has battery backed SRAM at 6000 - 7FFF
        
        bool is_PC10;         //is a PC-10 game
        bool is_VS;           //is a VS. game
        
        bool is_nes2;         //is using NES 2.0 extension
    } flags;
    
    ~INES();
    INES(std::istream& file);
    INES(const INES& other);
    INES& operator= (const INES& other);
};

#endif /* LoadRom_hpp */
