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
#include "../../util/memory.h"
#include "INes.hpp"
#include "../mapper/mapper.hpp"

class Cartridge final : public IMemory{
private:
    const INES* rom_data;
    Mapper* mapper;
public:
    Cartridge(const uint8* data, uint32 data_length);
    ~Cartridge();

    uint8 peek(uint16 addr) const override;
    uint8 read(uint16 addr) override;
    void write(uint16 addr, uint8 val) override;
    
    bool is_valid() const;
};

#endif /* Cartridge_hpp */
