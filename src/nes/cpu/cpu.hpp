//
//  cpu.hpp
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef cpu_hpp
#define cpu_hpp

#include "../../util/util.h"
#include "../../util/memory.h"
#include "../../util/bitfield.h"

// MOS 6502 (no BCD)
// https://wiki.nesdev.com/w/index.php/CPU
class CPU{
private:
/*----------  Hardware  ----------*/
    Memory& mem;
    
    //register
    struct{
        // special register
        uint8 sp; //stack pointer
        uint16 pc; //program counter
        
        //processor status
        union{
            uint8 raw; //underlying byte
            BitField<7> n; //negative
            BitField<6> v; //overflow
            //BitField<5> _; unuse
            BitField<4> b; //break command
            BitField<3> d; //decimal mode
            BitField<2> i; //interrupt disabled
            BitField<1> z; //zero flag
            BitField<0> c; //carry flag
        } p;
        
        // general register
        uint8 a; //accumulator
        uint8 x; //Index X
        uint8 y; //Index Y
    } reg;
    
/*----------  Emulation Vars  ----------*/
    uint64 cycles; //Cycles elapsed
    bool is_running;
    
/*--------------  Helpers  -------------*/
    uint8 read (uint16 addr);
    uint16 read_16(uint16 addr);
    void write(uint16 addr, uint8 val);
    void write_16(uint16 addr, uint8 val);
    
public:
    CPU(Memory& mem);
    ~CPU();
    
    void power_cycle();
    void reset();
};

#endif /* cpu_hpp */
