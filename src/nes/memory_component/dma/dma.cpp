//
// Created by Ows on 2019-07-19.
//

#include "dma.hpp"
#include <assert.h>

DMA::DMA(IMemory &cpu_wram, IMemory &ppu_oam)
    : cpu_wram(cpu_wram), ppu_oam(ppu_oam)
{}

void DMA::start(uint8 page) {
    assert(this->in_dma == false);

    this->in_dma = true;
    this->page = page;
    this->step = 0;
}

void DMA::transfer() {
    uint16 cpu_addr = this->step + uint16 (this->page << 8);
    uint16 oam_addr = this->step;
    this->ppu_oam[oam_addr] = this->cpu_wram[cpu_addr];
    this->step++;

    if (this->step > 0xFF)
        this->in_dma = false;
}

bool DMA::isActive() const {
    return this->in_dma;
}