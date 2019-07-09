//
//  cpu.cpp
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "cpu.hpp"
#include <cstdio>

CPU::~CPU(){}

CPU::CPU(Memory& mem) : mem(mem){
    this->power_cycle();
}

// https://wiki.nesdev.com/w/index.php/CPU_power_up_state
void CPU::power_cycle(){
    this->reg.p.raw = 0b00110100; //interrupt = 1, break = 1
    
    this->reg.a = 0x00;
    this->reg.x = 0x00;
    this->reg.y = 0x00;
    
    this->reg.sp = 0x5D;
    
    // Read initial PC from reset vector
    this->reg.pc = this->read(0xFFFC);
}

// https://wiki.nesdev.com/w/index.php/CPU_power_up_state
void CPU::reset(){
    this->reg.sp -= 3;  // the stack pointer is decremented by 3 (weird...)
    this->reg.p.i = 1;
    
    // Read from reset vector
    this->reg.pc = this->read_16(0xFFFC);
}

/*----------  Helpers  ----------*/
uint8 CPU::read(uint16 addr){
    return this->mem.read(addr);
}

void CPU::write(uint16 addr, uint8 val){
    this->mem.write(addr, val);
}

uint16 CPU::read_16(uint16 addr){
    return this->read(addr + 0) |
    (this->read(addr + 1) << 8);
}

void CPU::write_16(uint16 addr, uint8 val){
    this->write(addr + 0, val);
    this->write(addr + 1, val);
}
