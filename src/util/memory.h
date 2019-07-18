//
//  memory.h
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "util.h"

class IMemory{
public:
    virtual uint8 peek(uint16 addr) const = 0; //read, but without the side-effects
    virtual uint8 read(uint16 addr) = 0;
    virtual void write(uint16 addr, uint8 val) = 0;

    //Derived memory accesss
    uint16 peek16(uint16 addr) const {
        return this->peek(addr + 0) | (this->peek(addr + 1) << 8);
    }

    uint16 read16(uint16 addr) {
        return this->read(addr + 0) | (this->read(addr + 1) << 8);
    }

    uint16 peek16_zpg(uint16 addr){
        return this->peek(addr + 0) | (this->peek((addr & 0xFF00) | (addr + 1 & 0x00FF)) << 8);
    }

    uint16 read16_zpg(uint16 addr){
        return this->read(addr + 1) | (this->read((addr & 0xFF00) | (addr + 1 & 0x00FF)) << 8);
    }

    void write16(uint16 addr, uint8 val){
        this->write(addr + 0, val);
        this->write(addr + 1, val);
    }
};

#endif /* memory_h */
