//
//  ram.hpp
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef ram_hpp
#define ram_hpp

#include "../../util/util.h"
#include "../../util/memory.h"

// Dead simple 16 bit RAM ADT
// (max 64K of RAM)
class Ram final : public IMemory{
private:
    uint8* ram;
    uint32 size;
public:
    Ram(uint32 ram_size);
    ~Ram();

    uint8 peek(uint16 addr) const override;
    uint8 read(uint16 addr) override;
    void write(uint16 addr, uint8 val) override;
    void clear();
};

#endif /* ram_hpp */
