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

// ... NESTEST DEBUG ... //
static char nestest_buf [64];

uint16 CPU::get_operand_addr(const Instructions::Opcode &opcode) {
    using namespace Instructions;
    using namespace Instructions::AddrM;

    // We want to return a 8 / 16 bit number back to the CPU that can be used by
    // the Instruction
    uint16  retval;

    // To make debugging NESTEST easier, add temp macros to assign argV and argA
    // on demand
    uint8 argV;     //1 bytes - Value, or zero page addr
    uint16 argA;    // 2 bytes - Addresses

    //6502 always reads 1 argument byte, regardless of addressing mode.
#define read_argV() (argV = this->mem_read(this->reg.pc))
#define read_argA() (argA = (this->mem_read_16(this->reg.pc)))

    // Define temporary macro to slim down bulky, repeating switch statement code
#define M(mode, inc_pc, ...) \
    case mode: {\
        retval = (__VA_ARGS__); \
        this->reg.pc += inc_pc; \
        } break;

    switch (opcode.addrm){
        M(abs_, 2, read_argA())
        M(absX, 2, read_argA() + this->reg.x)
        M(absY, 2, read_argA() + this->reg.y)
        M(ind_, 1, this->mem_read_16_zpg(read_argA()))
        M(indY, 1, this->mem_read_16_zpg(read_argA())+ this->reg.y)
        M(Xind, 1, this->mem_read_16_zpg((read_argV() + this->reg.x) & 0xFF))
        M(zpg_, 1, read_argV())
        M(zpgX, 1, (read_argV() + this->reg.x) & 0xFF)
        M(zpgY, 1,(read_argV() + this->reg.y) & 0xFF)
        M(rel , 1, this->reg.pc)
        M(imm , 1, this->reg.pc)
        M(acc , 0, this->reg.a)
        M(impl, 0, uint8 (0xFACA11))
        case INVALID:
            fprintf(stderr, "Invalid Addressing Mode! Double check table!\\n\n");
            fprintf(stderr, "%02X\n", this->mem.peek(0x02));
            exit(-1);
            break;
    }

#undef M
#undef read_argA
#undef read_argV
    //Check to see if we need to add extra cycles due to crossing pages
    if(opcode.check_pg_cross == true){
        if(opcode.instr == Instr::LDA && opcode.addrm == indY){
            if(this->mem.peek(argV) + this->reg.y > 0xFF){
                this->cycles += 1;
            }
        }
        else if((opcode.instr == Instr::LDA || opcode.instr == Instr::LDX) && opcode.addrm == absY){
            if((argA & 0xFF00) != ((argA + this->reg.y) & 0xFF00)){
                this->cycles += 1;
            }
        }
        else{
            // We know a page boundary was crossed when the calculated addr was of the
            // form $xxFF, since trying to read a 16 bit address from memory at that
            // address would cross into the next page (eg: $12FF -> $1300 crosses pages)
            if((argA & 0xFF) == 0xFF){
                this->cycles += 1;
            }
        }
    }

    return retval;
}

uint8 CPU::step(){
    uint32 old_cycles = this->cycles;
    
    //fetch instruction
    uint8  op = this->mem_read(this->reg.pc);
    //Lookup info about opcode
    Instructions::Opcode opcode = Instructions::Opcodes[op];

    this->nestest(opcode);
    // Depending on what addrm this instruction uses, this will either be a u8
    // or a u16. Thus, we use a u16 to get the value from the fn, and let
    // individual instructions cast it to u8 when they need to.
    uint16 addr = this->get_operand_addr(opcode);

    /* EXECUTE INSTRUCTION */
    using namespace Instructions::Instr;
    using namespace Instructions::AddrM;

    // Define some utility macros (to cut down on repetitive cpu code)
    // Set Zero and Negative flags
#define set_zn(val) \
    this->reg.p.z = val == 0; \
    this->reg.p.n = nth_bit(val, 7);

// Branch if condition is satisfied
#define branch(cond)                                                 \
    if (!cond) break;                                                  \
    int8 offset = int8(this->mem_read(addr));                              \
    /* Extra cycle on succesful branch */                              \
    this->cycles += 1;                                                 \
    /* Check if extra cycles due to jumping across pages */            \
    if ((this->reg.pc & 0xFF00) != ((this->reg.pc + offset) & 0xFF00)) \
      this->cycles += 2;                                               \
    this->reg.pc += offset;

    switch (opcode.instr){
        case JMP: {
            this->reg.pc = addr;
        } break;
        case LDX: {
            this->reg.x = this->mem_read(addr);
            set_zn(this->reg.x);
        } break;
        case STX: {
            this->mem_write(addr, this->reg.x);
        } break;
        case JSR: {
            this->s_push_16(this->reg.pc - 1);
            this->reg.pc = addr;
        } break;
        case NOP: {
            //me irl
        }   break;
        case SEC: {
            this->reg.p.c = 1;
        } break;
        case BCS: {
            branch(this->reg.p.c);
        } break;
        case CLC: {
            this->reg.p.c = 0;
        } break;
        case BCC: {
            branch(!this->reg.p.c)
        } break;
        case LDA: {
            this->reg.a = this->mem_read(addr);
            set_zn(this->reg.a);
        } break;
        case BEQ: {
            branch(this->reg.p.z);
        } break;
        case BNE: {
            branch(!this->reg.p.z);
        } break;
        case STA: {
            this->mem_write(addr, this->reg.a);
        } break;
        case BIT: {
            uint8 mem = this->mem.read(addr);
            this->reg.p.z = (this->reg.a & mem) == 0;
            this->reg.p.v = nth_bit(mem, 6);
            this->reg.p.n = nth_bit(mem, 7);
        } break;
        case BVS: {
            branch(this->reg.p.v);
        } break;
        case BVC: {
            branch(!this->reg.p.v);
        } break;
        case BPL: {
            branch(!this->reg.p.n);
        } break;
        case RTS: {
            this->reg.pc = this->s_pull_16() + 1;
        } break;
        case AND: {
            this->reg.a &= this->mem_read(addr);
            set_zn(this->reg.a);
        } break;
        case SEI: {
            this->reg.p.i = 1;
        } break;
        case SED: {
            this->reg.p.d = 1;
        } break;
        case PHP: {
            this->s_push(this->reg.p.raw);
        } break;
        case PLA: {
            this->reg.a = this->s_pull();
            set_zn(this->reg.a);
        } break;
        case CMP: {
            uint8 val = this->mem_read(addr);
            this->reg.p.c = this->reg.a >= val;
            set_zn(this->reg.a - val);
        } break;
        case CLD: {
            this->reg.p.d = 0;
        } break;
        case PHA: {
            this->s_push(this->reg.a);
        } break;
        case PLP: {
            this->reg.p.raw = this->s_pull() | 0x20; // NESTEST
        } break;
        case BMI: {
            branch(this->reg.p.n);
        } break;
        case ORA: {
            this->reg.a |= this->mem_read(addr);
            set_zn(this->reg.a);
        } break;
        case CLV: {
            this->reg.p.v = 0;
        } break;
        case EOR: {
            this->reg.a ^= this->mem_read(addr);
            set_zn(this->reg.a);
        } break;
        case ADC: {
            uint8 val = this->mem_read(addr);
            uint16 sum = this->reg.a + val + !!this->reg.p.c;
            this->reg.p.c = sum > 0xFF;
            this->reg.p.z = uint8 (sum) == 0;
            //http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
            this->reg.p.v = ~(this->reg.a ^ val)
                            &  (this->reg.a ^ sum)
                            & 0x80;
            this->reg.p.n = nth_bit(uint8(sum), 7);
            this->reg.a = uint8(sum);
        } break;
        case SBC: {
            uint8 val = this->mem_read(addr);
            uint16 sum = this->reg.a + ~val + !!this->reg.p.c;
            this->reg.p.c = !(sum > 0xFF);
            this->reg.p.z = uint8(sum) == 0;
            // http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
            this->reg.p.v = ~(this->reg.a ^ ~val)
                            &  (this->reg.a ^ sum)
                            & 0x80;
            this->reg.p.n = nth_bit(uint8(sum), 7);
            this->reg.a = uint8(sum);
        } break;
        case LDY: {
            this->reg.y = this->mem_read(addr);
            set_zn(this->reg.y);
        } break;
        case CPY: {
            uint8 val = this->mem_read(addr);
            this->reg.p.c = this->reg.y >= val;
            set_zn(this->reg.y - val);
        } break;
        case CPX: {
            uint8 val = this->mem_read(addr);
            this->reg.p.c = this->reg.x >= val;
            set_zn(this->reg.x - val);
        } break;
        case INY: {
            this->reg.y++;
            set_zn(this->reg.y);
        } break;
        case INX: {
            this->reg.x++;
            set_zn(this->reg.x);
        } break;
        case DEY: {
            this->reg.y--;
            set_zn(this->reg.y);
        } break;
        case DEX: {
            this->reg.x--;
            set_zn(this->reg.x);
        } break;
        case STY: {
            this->mem_write(addr, this->reg.y);
        } break;
        case TAY: {
            this->reg.y = this->reg.a;
            set_zn(this->reg.y);
        } break;
        case TAX: {
            this->reg.x = this->reg.a;
            set_zn(this->reg.x);
        } break;
        case TYA: {
            this->reg.a = this->reg.y;
            set_zn(this->reg.a);
        } break;
        case TXA: {
            this->reg.a = this->reg.x;
            set_zn(this->reg.a);
        } break;
        case TSX: {
            this->reg.x = this->reg.sp;
            set_zn(this->reg.x);
        } break;
        case TXS: {
            this->reg.sp = this->reg.x;
        } break;
        case RTI: {
            this->reg.p.raw = this->s_pull() | 0x20; // NESTEST
            this->reg.pc = this->s_pull_16();
        } break;
        case BRK: {
            this->s_push_16(this->reg.pc);
            this->s_push(this->reg.p.raw);
            this->reg.pc = this->mem_read_16(0xFFFE);
        } break;
        case LSR: {
            if (opcode.addrm == acc) {

                this->reg.p.c = nth_bit(this->reg.a, 0);
                this->reg.a >>= 1;
                set_zn(this->reg.a);
            } else {
                uint8 val = this->mem_read(addr);
                this->reg.p.c = nth_bit(val, 0);
                val >>= 1;
                set_zn(val);
                this->mem_write(addr, val);
            }
        } break;
        case ASL: {
            if (opcode.addrm == acc) {

                this->reg.p.c = nth_bit(this->reg.a, 7);
                this->reg.a <<= 1;
                set_zn(this->reg.a);
            } else {
                uint8 val = this->mem_read(addr);
                this->reg.p.c = nth_bit(val, 7);
                val <<= 1;
                set_zn(val);
                this->mem_write(addr, val);
            }
        } break;
        case ROR: {
            if (opcode.addrm == acc) {
                // ... NESTEST DEBUG ... //
                sprintf(nestest_buf, "A");

                bool old_bit_0 = nth_bit(this->reg.a, 0);
                this->reg.a = (this->reg.a >> 1) | (!!this->reg.p.c << 7);
                this->reg.p.c = old_bit_0;
                set_zn(this->reg.a);
            } else {
                uint8 val = this->mem_read(addr);
                bool old_bit_0 = nth_bit(val, 0);
                val = (val >> 1) | (!!this->reg.p.c << 7);
                this->reg.p.c = old_bit_0;
                set_zn(val);
                this->mem_write(addr, val);
            }
        } break;
        case ROL: {
            if (opcode.addrm == acc) {
                // ... NESTEST DEBUG ... //
                sprintf(nestest_buf, "A");

                bool old_bit_0 = nth_bit(this->reg.a, 7);
                this->reg.a = (this->reg.a << 1) | !!this->reg.p.c;
                this->reg.p.c = old_bit_0;
                set_zn(this->reg.a);
            } else {
                uint8 val = this->mem_read(addr);
                bool old_bit_0 = nth_bit(val, 7);
                val = (val << 1) | !!this->reg.p.c;
                this->reg.p.c = old_bit_0;
                set_zn(val);
                this->mem_write(addr, val);
            }
        } break;
        case INC: {
            uint8 val = this->mem_read(addr);
            val++;
            set_zn(val);
            this->mem_write(addr, val);
        } break;
        case DEC: {
            uint8 val = this->mem_read(addr);
            val--;
            set_zn(val);
            this->mem_write(addr, val);
        };
        default: fprintf(stderr, "Unimplemented Instruction!\n"); exit(-1);
    }

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
    return this->mem_read(0x0100 + ++this->reg.sp);
}

void CPU::s_push(uint8 val){
    this->mem_write(0x0100 + this->reg.sp--, val);
}

uint16 CPU::s_pull_16(){
    uint16 lo = this->s_pull();
    uint16 hi = this->s_pull();
    return (hi << 8) | lo;
}

void CPU::s_push_16(uint16 val){
    this->s_push(val >> 8); //push hi
    this->s_push(val);      //push lo
}

uint16 CPU::mem_read_16_zpg(uint16 addr) {
    return this->mem_read(addr + 0) |
           (this->mem_read((addr & 0xFF00) | (addr + 1 & 0x00FF)) << 8);
}

/*
 * NESTEST
 */
void CPU::nestest(const Instructions::Opcode &opcode) const {
    using namespace Instructions;
    // Print PC and raw opcode byte
    printf("%04X  %02X ", this->reg.pc - 1, opcode.raw);

    //create buffer for instruction operands
    char instr_buf[64];

    //Decode addressing mode
    uint16 addr = 0;

    //open a new scope to use AddrM namespace
    using namespace Instructions::AddrM;

#define mem_peek16(x) \
    (this->mem.peek(x + 0) | \
    (this->mem.peek(x + 1) << 8))

#define mem_peek16_zpg(x)  \
    (this->mem.peek(x + 0) | \
    (this->mem.peek((x & 0xFF00) | (x + 1 & 0x00FF)) << 8))

    //Evaluate a few useful value
    uint8 argV1 = this->mem.peek(this->reg.pc + 0);
    uint8 argV2 = this->mem.peek(this->reg.pc + 1);
    uint16 argA = mem_peek16(this->reg.pc);

    //print operand bytes
    const char* iname = opcode.instr_name;
    switch(opcode.addrm){
        case abs_:
        case absX:
        case absY:
        case ind_:
            printf("%02X %02X", argV1, argV2);
            break;
        case indY:
        case Xind:
        case zpg_:
        case zpgX:
        case zpgY:
        case rel :
        case imm :
            printf("%02X   "  , argV1);
            break;
        default:
            printf("     ");
            break;
    }
    // Followed by Instruction Name
    printf("  %s ", iname);

    // Decode addressing mode
    // Define temporary macro to slim down bulky, repeating switch statement code
#define M(mode, inc_pc, ...) \
    case mode: { retval = __VA_ARGS__; this->reg.pc += inc_pc; } break;

    switch(opcode.addrm) {
        case abs_: addr = argA;                                         break;
        case absX: addr = argA + this->reg.x;                           break;
        case absY: addr = argA + this->reg.y;                           break;
        case ind_: addr = mem_peek16_zpg(argA);                         break;
        case indY: addr = mem_peek16_zpg(argV1) + this->reg.y;          break;
        case Xind: addr = mem_peek16_zpg((argV1 + this->reg.x) & 0xFF); break;
        case zpg_: addr = argV1;                                        break;
        case zpgX: addr = (argV1 + this->reg.x) & 0xFF;                 break;
        case zpgY: addr = (argV1 + this->reg.y) & 0xFF;                 break;
        case rel : addr = this->reg.pc;                                 break;
        case imm : addr = this->reg.pc;                                 break;
        case acc : addr = this->reg.a;                                  break;
        case impl: addr = uint8(0xFACA11);                              break;
        default: break;
    }

    // Print specific instrucion operands for each addressing mode
    switch (opcode.addrm){
        case abs_:
            sprintf(instr_buf, "$%04X = %02X",
                    argA,
                    this->mem.peek(argA)
            );
            break;
        case absX:
            sprintf(instr_buf, "$%04X,X @ %04X = %02X",
                    argA,
                    uint16(argA + this->reg.x),
                    this->mem.peek(argA + this->reg.x)
            );
            break;
        case absY:
            sprintf(instr_buf, "$%04X,Y @ %04X = %02X",
                    argA,
                    uint16(argA + this->reg.y),
                    this->mem.peek(argA + this->reg.y)
            );
            break;
        case indY:
            sprintf(instr_buf, "($%02X),Y = %04X @ %04X = %02X",
                    argV1,
                    mem_peek16_zpg(argV1),
                    uint16(mem_peek16_zpg(argV1) + this->reg.y),
                    this->mem.peek(mem_peek16_zpg(argV1) + this->reg.y)
            );
            break;
        case Xind:
            sprintf(instr_buf, "($%02X,X) @ %02X = %04X = %02X",
                    argV1,
                    uint8(this->reg.x + argV1),
                    mem_peek16_zpg(uint8(this->reg.x + argV1)),
                    this->mem.peek(mem_peek16_zpg(uint8(this->reg.x + argV1)))
            );
            break;
        case ind_:
            sprintf(instr_buf, "($%04X) = %04X",
                    argA,
                    mem_peek16_zpg(argA)
            );
            break;
        case zpg_:
            sprintf(instr_buf, "$%02X = %02X",
                    argV1,
                    this->mem.peek(argV1)
            );
            break;
        case zpgX:
            sprintf(instr_buf, "$%02X,Y @ %02X = %02X",
                    argV1,
                    uint8(argV1 + this->reg.y),
                    this->mem.peek(uint8(argV1 + this->reg.y))
            );
            break;
        case rel:
            sprintf(instr_buf, "$%04X", this->reg.pc + 1 + int8(argV1));
            break;
        case imm:
            sprintf(instr_buf, "#$%02X", argV1);
            break;
        case acc:
            sprintf(instr_buf, "");
            break;
        case impl:
            sprintf(instr_buf, "");
            break;
        default: sprintf(instr_buf, ""); break;
    }

    // handle a few edge cases
    switch (opcode.instr) {
        case Instr::JMP:
        case Instr::JSR:
            if (opcode.addrm == abs_)
                sprintf(instr_buf, "$%04X", addr);
            break;
        case Instr::LSR:
        case Instr::ASL:
        case Instr::ROR:
        case Instr::ROL:
            if (opcode.addrm == acc)
                sprintf(instr_buf, "A");
            break;
        default: break;
    }

    // Print instruction operands
    printf("%-28s", instr_buf);

    // Print processor state
    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%3u\n",
           this->reg.a,
           this->reg.x,
           this->reg.y,
           this->reg.p.raw & 0b11101111, // match nestest "golden" log
           this->reg.sp,
           this->cycles * 3 % 341 // CYC measures PPU X coordinates
            // PPU does 1 x coordinate per cycle
            // PPU runs 3x as fast as CPU
            // ergo, multiply cycles by 3 should be fineee
    );
}