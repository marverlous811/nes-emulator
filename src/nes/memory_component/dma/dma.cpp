//
// Created by Ows on 2019-07-19.
//

#include "dma.hpp"
#include <assert.h>

DMA::DMA(IMemory &cpu_wram, IMemory &ppu_oam)
    : cpu_wram(cpu_wram), ppu_oam(ppu_oam)
{}

// Reading from DMA is not a valid operation...
uint8 DMA::read(uint16 addr) {
    assert(addr == 0x4014);
    (void) addr;
    return 0x0;
}

uint8 DMA::peek(uint16 addr) const {
    assert(addr == 0x4014);
    (void) addr;
    return 0x0;
}

void DMA::write(uint16 addr, uint8 page) {
    assert(addr == 0x4014);
    for(uint16 addr = 0; addr <= 256; addr++){
        //read value from CPU WRAM
        uint8 cpu_val = this->cpu_wram[(uint16(page) << 8) + addr];
        //dump it into the PPU OAM
        this->ppu_oam[addr] = cpu_val;

    }
}