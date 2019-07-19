//
// Created by Ows on 2019-07-19.
//

#ifndef NES_EMULATOR_DMA_HPP
#define NES_EMULATOR_DMA_HPP

#include "../../../util/memory.h"
#include "../../../util/util.h"

// CPU WRAM to PPU OAM Direct Memory Access (DMA) Unit
// http://wiki.nesdev.com/w/index.php/PPU_programmer_reference#OAMDMA
class DMA final : public IMemory {
private:
    IMemory& cpu_wram;
    IMemory& ppu_oam;

public:
    ~DMA() = default;
    DMA(IMemory& cpu_wram, IMemory& ppu_oam);

    //<Memory>
    uint8 read(uint16 addr) override ;
    uint8 peek(uint16 addr) const override ;
    void write(uint16 addr, uint8 page) override ;
    //</Memory>
};


#endif //NES_EMULATOR_DMA_HPP
