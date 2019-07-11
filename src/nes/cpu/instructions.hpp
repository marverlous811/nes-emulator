//
// Created by Ows on 2019-07-11.
//

#ifndef NES_EMULATOR_INSTRUCTIONS_HPP
#define NES_EMULATOR_INSTRUCTIONS_HPP

#pragma once

#include <assert.h>
#include "../../util/util.h"

namespace Instrcutions{
    enum Instr : uint8 {
        INVALID,
        // Instructions are determined by their
        // aaa and cc bits, with the middle bbb
        // bits determining the addressing mode
        // aaa000ccc
        ADC =  0b01100001,
        AND =  0b00100001,
        ASL =  0b00000010,
        BCC =  0b10000000,
        BCS =  0b10100000,
        BEQ =  0b11100000,
        BIT =  0b00100000,
        BMI =  0b00100000,
        BNE =  0b11000000,
        BPL =  0b00000000,
        BRK =  0b00000000,
        BVC =  0b01000000,
        BVS =  0b01100000,
        CLC =  0b00000000,
        CLD =  0b11000000,
        CLI =  0b01000000,
        CLV =  0b10100000,
        CMP =  0b11000001,
        CPX =  0b11100000,
        CPY =  0b11000000,
        DEC =  0b11000010,
        DEX =  0b11000010,
        DEY =  0b10000000,
        EOR =  0b01000001,
        INC =  0b11100010,
        INX =  0b11100000,
        INY =  0b11000000,
        JMP =  0b01000000,
        JSR =  0b00100000,
        LDA =  0b10100001,
        LDX =  0b10100010,
        LDY =  0b10100000,
        LSR =  0b01000010,
        NOP =  0b11100010,
        ORA =  0b00000001,
        PHA =  0b01000000,
        PHP =  0b00000000,
        PLA =  0b01100000,
        PLP =  0b00100000,
        ROL =  0b00100010,
        ROR =  0b01100010,
        RTI =  0b01000000,
        RTS =  0b01100000,
        SBC =  0b11100001,
        SEC =  0b00100000,
        SED =  0b11100000,
        SEI =  0b01100000,
        STA =  0b10000001,
        STX =  0b10000010,
        STY =  0b10000000,
        TAX =  0b10100010,
        TAY =  0b10100000,
        TSX =  0b10100010,
        TXA =  0b10000010,
        TXS =  0b10000010,
        TYA =  0b10000000,
        JMP_I = uint8 (0xFACA11)
    };

    inline Instr  op_to_instr(uint8 op){
        if(op == 0b01101100){
            return Instr::JMP_I;
        }

        return Instr (op & 0b01101100);
    }

    enum class AddrM : uint8 {
        INVALID,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        Accumulator,
        Immediate,
        Implicit,
        IndexedY,
        XIndexed,
        Relative,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Indirect, // used by JMP_I
    };

    inline AddrM op_to_addrm(uint8 op){
        #define MODE(_mode_) AddrM::_mode_

        static const AddrM  cc_bbb_to_addrmode[3][8] = {
            /* cc == 00 */
            {
                /* bbb == 000 */ MODE( Immediate   ),
                /* bbb == 001 */ MODE( ZeroPage    ),
                /* bbb == 010 */ MODE( Implicit    ),
                /* bbb == 011 */ MODE( Absolute    ),
                /* bbb == 100 */ MODE( Relative    ),
                /* bbb == 101 */ MODE( ZeroPageX   ),
                /* bbb == 110 */ MODE( Implicit    ),
                /* bbb == 111 */ MODE( AbsoluteX   )
            },
            /* cc == 01 */
            {
                /* bbb == 000 */ MODE( XIndexed    ),
                /* bbb == 001 */ MODE( ZeroPage    ),
                /* bbb == 010 */ MODE( Immediate   ),
                /* bbb == 011 */ MODE( Absolute    ),
                /* bbb == 100 */ MODE( IndexedY    ),
                /* bbb == 101 */ MODE( ZeroPageX   ),
                /* bbb == 110 */ MODE( AbsoluteY   ),
                /* bbb == 111 */ MODE( AbsoluteX   )
            },
            /* cc == 10 */
            {
                /* bbb == 000 */ MODE( Immediate   ),
                /* bbb == 001 */ MODE( ZeroPage    ),
                /* bbb == 010 */ MODE( Accumulator ),
                /* bbb == 011 */ MODE( Absolute    ),
                /* bbb == 100 */ MODE( INVALID     ),
                /* bbb == 101 */ MODE( ZeroPageX   ),
                /* bbb == 110 */ MODE( Implicit    ),
                /* bbb == 111 */ MODE( AbsoluteX   )
            }
        };

        const uint8  bbb = (op & 0b00011100) >> 2;
        const uint8  cc  = (op & 0b00000011) >> 0;

        assert(cc != 0b11);

        // The 6502 is a mean sonafabitch, and sometimes, for no apparent reason, it
        // implements a instruction that doesn't fit the otherwise clean opcode
        // structure.
        // We handle these "asshole" instructions here:
        //
        // BRK  00  000 000 00  Implicit
        // JSR  20  001 000 00  Absolute
        // RTI  40  010 000 00  Implicit
        // RTS  60  011 000 00  Implicit
        //
        // LDX  be  101 111 10  AbsoluteY // grouped with AbsoluteX
        // STX  96  100 101 10  ZeroPageY // grouped with ZeroPageX
        // LDX  b6  101 101 10  ZeroPageY // grouped with ZeroPageX
        //
        // And of course, this asshole, who is the *only* Indirect instruction _smh_
        //
        // JMP_I  6c  011 011 00  Indirect

        switch (Instr(op & 0b11100011)){
            case Instr::BRK: return MODE(Implicit);
            case Instr::RTI: return MODE(Implicit);
            case Instr::RTS: return MODE(Implicit);
            case Instr::JSR: return MODE(Absolute);

            case Instr::JMP_I: return MODE(Indirect);
            default: return cc_bbb_to_addrmode[cc][bbb];
        }

        #undef MODE
    }
}

#endif //NES_EMULATOR_INSTRUCTIONS_HPP
