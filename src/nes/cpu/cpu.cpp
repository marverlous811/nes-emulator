//
//  cpu.cpp
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "cpu.hpp"
#include "instructions.hpp"
#include <cstdio>
#include <string>

CPU::~CPU(){}

CPU::CPU(IMemory& mem) : mem(mem){
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
//    this->reg.pc = this->read(0xFFFC);
    
    // >> SET TO 0xC000 to do nestest.rom
    this->reg.pc = 0xC000;
    
    this->cycles = 0;
    this->state = CPU::State::Running;
}

// https://wiki.nesdev.com/w/index.php/CPU_power_up_state
void CPU::reset(){
    this->reg.sp -= 3;  // the stack pointer is decremented by 3 (weird...)
    this->reg.p.i = 1;
    
    // Read from reset vector
//    this->reg.pc = this->read_16(0xFFFC);
    
    // >> SET TO 0xC000 to do nestest.rom
    this->reg.pc = 0xC000;
    
    this->cycles = 0;
    this->state = CPU::State::Running;
}

CPU::State CPU::getState() const{
    return this->state;
}

uint8 CPU::step(){
    uint32 old_cycles = this->cycles;
    
    //fetch instruction
    uint8  op = this->read(this->reg.pc);
    Instrcutions::Instr instr = Instrcutions::op_to_instr(op);
    Instrcutions::AddrM addrm = Instrcutions::op_to_addrm(op);

    printf("%04X  %02X ", this->reg.pc, op);
    
    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3u\n",
           this->reg.a,
           this->reg.x,
           this->reg.y,
           this->reg.p.raw & 0b11101111, // match nestest "golden" log
           this->reg.sp,
           old_cycles * 3 % 341
           // CYC measures PPU X coordinates
           // PPU does 1 x coordinate per cycle
           // PPU runs 3x as fast as CPU
           // ergo, multiply cycles by 3 should be fineee
           );
    
    return this->cycles - old_cycles;
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

uint8 CPU::s_pull(){
    return this->read(0x0100 + this->reg.sp++);
}

void CPU::s_push(uint8 val){
    this->write(0x0100 + this->reg.sp--, val);
}

uint16 CPU::s_pull_16(){
    uint8 lo = this->s_pull();
    uint8 hi = this->s_pull();
    return (hi << 8) | lo;
}

void CPU::s_push_16(uint16 val){
    this->s_push(val >> 8); //push hi
    this->s_push(val);      //push lo
}
