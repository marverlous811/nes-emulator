//
//  Cartridge.hpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef Cartridge_hpp
#define Cartridge_hpp

#include "../../util/util.h"
#include "INes.hpp"
#include "../mapper/mapper.hpp"

class Cartridge{
private:
    const INES* rom_data;
    Mapper* mapper;
public:
    Cartridge(const uint8* data, uint32 data_length);
    ~Cartridge();
    
    uint8 read(uint16 addr);
    void write(uint16 addr, uint8 val);
    
    bool is_valid();
};

#endif /* Cartridge_hpp */
