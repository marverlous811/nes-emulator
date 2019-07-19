//
// Created by Ows on 2019-07-19.
//

#ifndef NES_EMULATOR_DMA_HPP
#define NES_EMULATOR_DMA_HPP

#include "../../../util/memory.h"
#include "../../../util/util.h"

// CPU WRAM to PPU OAM Direct Memory Access (DMA) Unit
// http://wiki.nesdev.com/w/index.php/PPU_programmer_reference#OAMDMA
class DMA final{
private:
    IMemory& cpu_wram;
    IMemory& ppu_oam;

    bool in_dma;

    uint8 page;  // What CPU page to read from
    uint16 step; // How many transfers have occured (from 0x00 to 0xFF)

public:
    ~DMA() = default;
    DMA(IMemory& cpu_wram, IMemory& ppu_oam);

    void start(uint8 page);
    void transfer();

    bool isActive() const;
};


#endif //NES_EMULATOR_DMA_HPP
