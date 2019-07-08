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
// Returns 0 on read
// No effect on write
class VoidMemory : public Memory{
private:
    VoidMemory() = default;
    
public:
    uint8 read(uint16 addr) override { return 0; }
    void write(uint16 addr, uint8 val) override {}
    
    static VoidMemory* Get() {
        static VoidMemory the_void;
        return &the_void;
    }
};

// Wrapper that transaparently intercepts all transactions that occur through a
// given Memory* and logs them
class MemorySniffer final : public Memory{
private:
    const char* label;
    Memory* mem;
    
public:
    MemorySniffer(const char* label, Memory* mem) : mem(mem), label(label){}
    uint8 read(uint16 addr) override{
        if(this->mem == nullptr){
            printf("[%s] Memory is not define\n", this->label);
            return 0x00;
        }
        
        // only read once, to prevent side effects
        uint8 val = this->mem->read(addr);
        printf("[%s] R 0x%04X -> 0x%X\n", this->label, addr, val);
        return val;
    }
    
    void write(uint16 addr, uint8 val) override {
        if (this->mem == nullptr) {
            printf(
                   "[%s] Underlying Memory is nullptr!\n",
                   this->label
                   );
            return;
        }
        
        printf("[%s] W 0x%04X <- 0x%X\n", this->label, addr, val);
        this->mem->write(addr, val);
    };
};


#endif /* fakeMemory_h */
