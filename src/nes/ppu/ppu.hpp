//
// Created by Ows on 2019-07-18.
//

#ifndef NES_EMULATOR_PPU_HPP
#define NES_EMULATOR_PPU_HPP

#include "../../util/util.h"
#include "../../util/bitfield.h"
#include "../../util/memory.h"

class PPU {
public:
    enum class Mirroring{
        Vertical,
        Horizontal,
        FourScreen
    };
private:
    IMemory& mem;
public:
    ~PPU();
    PPU(IMemory& mem);
};


#endif //NES_EMULATOR_PPU_HPP
