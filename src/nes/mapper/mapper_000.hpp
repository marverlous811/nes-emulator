//
//  mapper_000.hpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef mapper_000_hpp
#define mapper_000_hpp

#include "mapper.hpp"
#include "../../util/util.h"
#include "../cpu/ram.hpp"

class Mapper_000 final : public Mapper{
private:
    // PGR ROM
    const ROM* lo_rom; //0x8000 ... 0xBFFF
    const ROM* hi_rom; //0xC000 ... 0xFFFF

    //CHR ROM/RAM
    const ROM* chr_rom;
    Ram* chr_ram;
public:
    Mapper_000(const INES& rom_file);
    ~Mapper_000();

    uint8 peek(uint16 addr) const override;
    uint8 read(uint16 addr) override;
    void write(uint16 addr, uint8 val) override;
};

#endif /* mapper_000_hpp */
