//
// Created by Ows on 2019-07-18.
//

#ifndef NES_EMULATOR_PPU_HPP
#define NES_EMULATOR_PPU_HPP

#include "../../util/util.h"
#include "../../util/bitfield.h"
#include "../../util/memory.h"

class PPU {
private:
    uint8  frame [ 240 * 256 * 4]; //Pixel buffer
    IMemory& mem;

    uint32 cycles;

    struct {
        uint16 x;
        uint16 y;
    } scan;
public:
    ~PPU();
    PPU(IMemory& mem);

    void power_cycle();
    void reset();

    void cycle();

    const uint8* getFrame() const;
};


#endif //NES_EMULATOR_PPU_HPP
