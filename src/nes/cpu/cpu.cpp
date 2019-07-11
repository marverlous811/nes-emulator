//
//  cpu.cpp
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "cpu.hpp"
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
    
    this->reg.sp = 0xFD;
    
    // Read initial PC from reset vector
//    this->reg.pc = this->mem_read_16(0xFFFC);
    
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
//    this->reg.pc = this->mem_read_16(0xFFFC);
    
    // >> SET TO 0xC000 to do nestest.rom
    this->reg.pc = 0xC000;
    
    this->cycles = 0;
    this->state = CPU::State::Running;
}

CPU::State CPU::getState() const{
    return this->state;
}

uint16 CPU::get_instr_args(Instrcutions::Opcode &opcode) {
    using namespace Instrcutions::AddrM;

    //the instruction
    uint16  retval;

    //2 size of argument
    uint8 argV;     //1 bytes - Value, or zero page addr
    uint16 argA;    // 2 bytes - Addresses

    //6502 always reads 1 argument byte, regardless of addressing mode.
    argV = this->mem_read(this->reg.pc);

    // We don't want to eagerly read another memory address, since that would
    // cause side-effects. Instead, to keep code clean, we define `GET_argA` as
    // a function-specific macro that does the extra read on a case-by-case basis

    argA = -1; //default to invalid value
#define read_argA() \
    (argA = (uint16(argV) + (this->mem_read(this->reg.pc + 1) << 8)))

    // Define temporary macro to slim down bulky, repeating switch statement code
#define M(mode, inc_pc, ...) \
    case mode: retval = (__VA_ARGS__); this->reg.pc += inc_pc; break;

    switch (opcode.addrm){
        M(abs_, 2, read_argA())
        M(absX, 2, read_argA() + this->reg.x)
        M(absY, 2, read_argA() + this->reg.y)
        M(ind_, 1, this->mem_read_16(this->mem_read_16(read_argA())))
        M(indY, 1, this->mem_read_16(this->mem_read_16(argV) + this->reg.y))
        M(Xind, 1, this->mem_read_16(this->mem_read_16((argV + this->reg.x) & 0xFF)) )
        M(zpg_, 1, this->mem_read((argV))                                          )
        M(zpgX, 1, this->mem_read((argV + this->reg.x) & 0xFF)                     )
        M(zpgY, 1, this->mem_read((argV + this->reg.y) & 0xFF)                     )
        M(rel , 1, argV                                                            )
        M(imm , 1, argV                                                            )
        M(acc , 0, this->reg.a                                                     )
        M(impl, 0, uint8 (0xFACA11) /* no args! return fack all :D */                  )
        case INVALID:
            fprintf(stderr, "Invalid Addressing Mode!\n");
            assert(false);
            break;
    }

#undef M
#undef read_argA

    //Check to see if we need to add extra cycles due to crossing pages
    if(opcode.check_pg_cross == true){
        // We know a page boundary was crossed when the calculated addr was of the
        // form $xxFF, since trying to read a 16 bit address from memory at that
        // address would cross into the next page (eg: $12FF -> $1300 crosses pages)
        if((argA & 0xFF) == 0xFF){
            this->cycles += 1;
        }
    }

    return retval;
}

uint8 CPU::step(){
    uint32 old_cycles = this->cycles;
    
    //fetch instruction
    uint8  op = this->mem_read(this->reg.pc);
    //Lookup info about opcode
    Instrcutions::Opcode opcode = Instrcutions::Opcodes[op];

    char INITIAL_STATE[32];
    sprintf(INITIAL_STATE, "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3u\n",
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
    printf("%04X  %02X  %s <%-4s> ", this->reg.pc - 1, op, opcode.instr_name, opcode.addrm_name);
    // Depending on what addrm this instruction uses, this will either be a u8
    // or a u16. Thus, we use a u16 to get the value from the fn, and let
    // individual instructions cast it to u8 when they need to.
    uint16 arg = this->get_instr_args(opcode);
    printf("%04X", arg);

    /* EXECUTE INSTRUCTION */
    using namespace Instrcutions::Instr;

    switch (opcode.instr){
        case JMP: {
            this->reg.pc = arg;
        } break;
        case LDX: {
            this->reg.x = arg;
            this->reg.p.z = arg == 0;
            this->reg.p.n = nth_bit(arg, 7);
        } break;
        case STX: {
            this->mem_write(arg, this->reg.x);
        } break;
        case JSR: {
            this->s_push_16(this->reg.pc - 1);
            this->reg.pc = arg;
        } break;
        case NOP: {
            //me irl
        }   break;
        case SEC: {
            this->reg.p.c = 1;
        } break;
        case BCS: {
            if(this->reg.p.c) this->branch(arg);
        } break;
        case CLC: {
            this->reg.p.c = 0;
        } break;
        case BCC: {
            if(!this->reg.p.c) this->branch(arg);
        } break;
        case LDA: {
            this->reg.a = arg;
            this->reg.p.z = arg == 0;
            this->reg.p.n = nth_bit(arg, 7);
        } break;
        case BEQ: {
            if(this->reg.p.z) this->branch(arg);
        } break;
        case BNE: {
            if(!this->reg.p.z) this->branch(arg);
        } break;
        case STA: {
            this->reg.a = arg;
        } break;
        case BIT: {
            this->reg.p.z = (this->reg.a & arg) == 0;
            this->reg.p.v = nth_bit(arg, 6);
            this->reg.p.n = nth_bit(arg, 7);
        } break;
        case BVS: {
            if(this->reg.p.v) this->branch(arg);
        } break;
        case BVC: {
            if(!this->reg.p.v) this->branch(arg);
        } break;
        case BPL: {
            if(!this->reg.p.n) this->branch(arg);
        } break;
        default: assert(false);
    }

    /* NEED TO IMPLEMENT INTERUPTS BEFORE CONTINUING! */

    // ... NESTEST DEBUG ... //
    printf("%s", INITIAL_STATE);

    this->cycles += opcode.cycles;

    return this->cycles - old_cycles;
}

/*----------  Helpers  ----------*/
uint8 CPU::mem_read(uint16 addr){
    return this->mem.read(addr);
}

void CPU::mem_write(uint16 addr, uint8 val){
    this->mem.write(addr, val);
}

uint16 CPU::mem_read_16(uint16 addr){
    return this->mem_read(addr + 0) |
    (this->mem_read(addr + 1) << 8);
}

void CPU::mem_write_16(uint16 addr, uint8 val){
    this->mem_write(addr + 0, val);
    this->mem_write(addr + 1, val);
}

uint8 CPU::s_pull(){
    return this->mem_read(0x0100 + this->reg.sp++);
}

void CPU::s_push(uint8 val){
    this->mem_write(0x0100 + this->reg.sp--, val);
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

void CPU::branch(uint8 offset) {
    this->cycles += 1;
    if((this->reg.pc & 0xFF00) == (offset & 0xFF00))
        this->cycles += 2;
    this->reg.pc += int8 (offset);
}