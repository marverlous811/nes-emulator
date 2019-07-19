//
// Created by Ows on 2019-07-18.
//

#ifndef NES_EMULATOR_PPU_MMU_HPP
#define NES_EMULATOR_PPU_MMU_HPP

#include "util/memory.h"
#include "util/util.h"
#include "nes/loadRom/Cartridge.hpp"

class PPU_MMU final: public IMemory{
private:
    // Fixed References (these will never be invalidated)
    IMemory& ciram; //PPU internal VRAM
    IMemory& pram; //palette RAM

    //Rom is subject to change
    Cartridge* rom;
    IMemory* vram; //changes báe on mirroring mode

    //Nametable offsets
    uint16 nt_0;
    uint16 nt_1;
    uint16 nt_2;
    uint16 nt_3;

public:
    // No Destructor, since no owned resources
    PPU_MMU(IMemory& ciram, IMemory& pram, Cartridge* rom);

    //<Memory>
    uint8 read(uint16 addr) override ;
    uint8 peek(uint16 addr) const override ;
    void write(uint16 addr, uint8 val) override ;
    //</Memory>

    void addCartridge(Cartridge* cart);
    void removeCartridge();
};


#endif //NES_EMULATOR_PPU_MMU_HPP
