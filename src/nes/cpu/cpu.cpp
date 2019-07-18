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

uint16 CPU::get_operand_addr(Instructions::Opcode &opcode) {
    using namespace Instructions::AddrM;

    // We want to return a 8 / 16 bit number back to the CPU that can be used by
    // the Instruction
    uint16  retval;

    // To make debugging NESTEST easier, add temp macros to assign argV and argA
    // on demand
    uint8 argV;     //1 bytes - Value, or zero page addr
    uint16 argA;    // 2 bytes - Addresses

    //6502 always reads 1 argument byte, regardless of addressing mode.
#define read_argV() \
    (argV = this->mem_read(this->reg.pc))

    // We don't want to eagerly read another memory address, since that would
    // cause side-effects. Instead, to keep code clean, we define `GET_argA` as
    // a function-specific macro that does the extra read on a case-by-case basis

    argA = -1; //default to invalid value
#define read_argA() \
    (argA = (this->mem_read_16(this->reg.pc)))

    // Define temporary macro to slim down bulky, repeating switch statement code
#define M(mode, inc_pc, ...) \
    case mode: {\
        retval = (__VA_ARGS__); \
        this->reg.pc += inc_pc; \
        /* ... NESTEST DEBUG ...  */ \
        if (inc_pc == 0) printf("       %s ", opcode.instr_name); \
        if (inc_pc == 1) printf("%02X     %s ", this->mem.peek(this->reg.pc - 1), opcode.instr_name); \
        if (inc_pc == 2) printf("%02X %02X  %s ", this->mem.peek(this->reg.pc - 2), argA >> 8, opcode.instr_name); \
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
        if(opcode.instr == Instructions::Instr::LDA && opcode.addrm == indY){
            if(this->mem.peek(argV) + this->reg.y > 0xFF){
                this->cycles += 1;
            }
        }
        else if((opcode.instr == Instructions::Instr::LDA || opcode.instr == Instructions::Instr::LDX) && opcode.addrm == absY){
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

    // ... NESTEST DEBUG ... //
#define NESPRINTF(...) sprintf(nestest_buf, __VA_ARGS__); break;
    switch(opcode.addrm) {
        case abs_: NESPRINTF("$%04X = %02X", argA, this->mem.peek(argA));
        case absX: NESPRINTF("$%04X,X @ %04X = %02X", argA, uint16(argA + this->reg.x), this->mem.peek(argA + this->reg.x));
        case absY: NESPRINTF("$%04X,Y @ %04X = %02X", argA, uint16(argA + this->reg.y), this->mem.peek(argA + this->reg.y));
        case ind_: NESPRINTF("($%04X) = %04X", argA, retval);
        case indY: {
            uint16 ADDR_2 = this->mem.peek(argV)
                         + (this->mem.peek((argV & 0xFF00) | (argV + 1 & 0x00FF)) << 8);
            NESPRINTF("($%02X),Y = %04X @ %04X = %02X",
                      this->mem.peek(this->reg.pc - 1),
                      ADDR_2,
                      uint16(ADDR_2 + this->reg.y),
                      this->mem.peek(ADDR_2 + this->reg.y)
            );
        }
        case Xind: {
            uint8 ADDR_1 = this->reg.x + this->mem.peek(this->reg.pc - 1);
            uint16 ADDR_2 = this->mem.peek(ADDR_1)
                         + (this->mem.peek((ADDR_1 & 0xFF00) | (ADDR_1 + 1 & 0x00FF)) << 8);
            NESPRINTF("($%02X,X) @ %02X = %04X = %02X",
                      this->mem.peek(this->reg.pc - 1),
                      ADDR_1,
                      ADDR_2,
                      this->mem.peek(ADDR_2)
            );
        }
        case zpg_: NESPRINTF("$%02X = %02X", argV, this->mem.peek(argV));
        case zpgX: NESPRINTF("$%02X,X @ %02X = %02X", argV, uint8(argV + this->reg.x), this->mem.peek(uint8(argV + this->reg.x)));
        case zpgY: NESPRINTF("$%02X,Y @ %02X = %02X", argV, uint8(argV + this->reg.y), this->mem.peek(uint8(argV + this->reg.y)));
        case rel : NESPRINTF("$%04X", this->reg.pc + int8(this->mem.peek(this->reg.pc - 1)));
        case imm : NESPRINTF("#$%02X", this->mem.peek(this->reg.pc - 1));
        case acc : NESPRINTF(" ");
        case impl: NESPRINTF(" ");
        default: break;
    }
#undef NESPRINTF

    return retval;
}

uint8 CPU::step(){
    uint32 old_cycles = this->cycles;
    
    //fetch instruction
    uint8  op = this->mem_read(this->reg.pc);
    //Lookup info about opcode
    Instructions::Opcode opcode = Instructions::Opcodes[op];

    char INITIAL_STATE[64];
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
    printf("%04X  %02X ", this->reg.pc - 1, op);
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
            if (opcode.addrm == abs_) {
                sprintf(nestest_buf, "$%04X", addr);
            }
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
            // ... NESTEST DEBUG ... //
            if (opcode.addrm == abs_) {
                sprintf(nestest_buf, "$%04X", addr);
            }
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
                // ... NESTEST DEBUG ... //
                sprintf(nestest_buf, "A");

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
                // ... NESTEST DEBUG ... //
                sprintf(nestest_buf, "A");

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

    printf("%-28s", nestest_buf);
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