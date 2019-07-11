//
//  mapper.hpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef mapper_hpp
#define mapper_hpp

#include "../../util/util.h"
#include "../../util/memory.h"
#include "../loadRom/INes.hpp"

class Mapper : public IMemory{
protected:
    const INES& rom_file;
    const uint8* lo_rom;
    const uint8* hi_rom;
    
public:
    Mapper(const INES& rom_file) : rom_file(rom_file) {};
    virtual ~Mapper() {};

    virtual uint8 peek(uint16 addr) const = 0;
    virtual uint8 read(uint16 addr) = 0;
    virtual void write(uint16 addr, uint8 val) = 0;
    
    //creates correct mapper given an iNes object
    static Mapper* Factory(const INES& rom_file);
};

#endif /* mapper_hpp */
