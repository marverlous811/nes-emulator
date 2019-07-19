//
//  fakeMemory.h
//  MyNes
//
//  Created by Ows on 7/6/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef fakeMemory_h
#define fakeMemory_h

#include <assert.h>
#include <stdio.h>
#include "./memory.h"

// Void Memory Singleton
// Returns 0x00 on read
// No effect on write
class VoidMemory : public IMemory{
private:
    VoidMemory() = default;
    
public:
    uint8 peek(uint16 addr) const override  { (void) addr; return 0x00; }
    uint8 read(uint16 addr) override { (void)addr; return 0x00; }
    void write(uint16 addr, uint8 val) override { (void) addr; (void) val;}
    
    static VoidMemory* Get() {
        static VoidMemory* the_void  = nullptr;
        if(!the_void) the_void = new VoidMemory();
        return the_void;
    }
};

// Wrapper that transaparently intercepts all transactions that occur through a
// given Memory* and logs them
class MemorySniffer final : public IMemory{
private:
    const char* label;
    IMemory* mem;
    
public:
    MemorySniffer(const char* label, IMemory* mem) : mem(mem), label(label){}
    uint8  read(uint16 addr) override ;
    uint8  peek(uint16 addr) const override ;
    void write(uint16 addr, uint8 val) override ;
};

uint8  MemorySniffer::read(uint16 addr) {
    if(this->mem == nullptr){
        printf("[%s] Underlying IMemory is nullptr\n", this->label);
        return 0x00;
    }

    uint8 val = this->mem->read(addr);
    printf("[%s] R 0x%04X -> 0x%02X\n", this->label, addr, val);
    return val;
}

uint8 MemorySniffer::peek(uint16 addr) const {
    if(this->mem == nullptr){
        printf("[%s] Underlying IMemory is nullptr\n", this->label);
        return 0x00;
    }

    uint8 val = this->mem->peek(addr);
    printf("[%s] R 0x%04X -> 0x%02X\n", this->label, addr, val);
    return val;
}

void MemorySniffer::write(uint16 addr, uint8 val) {
    if(this->mem == nullptr){
        printf("[%s] Underlying IMemory is nullptr\n", this->label);
        return;
    }

    printf("[%s] R 0x%04X -> 0x%02X\n", this->label, addr, val);
    this->mem->write(addr, val);
}


#endif /* fakeMemory_h */
