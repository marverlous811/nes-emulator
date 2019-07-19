//
// Created by Ows on 2019-07-18.
//

#include "ppu.hpp"
#include <assert.h>

PPU::~PPU() {}
PPU::PPU(IMemory &mem, IMemory& oam, IMemory& dma)
    : mem(mem), dma(dma), oam(oam)
{
    this->cycles = 0;

    this->scan.x = 0;
    this->scan.y = 0;
}

void PPU::power_cycle() {

}

void PPU::reset() {}

#include <cmath>

void PPU::cycle() {
    this->cycles += 1;

    const uint32 offset = (256 * 4 * this->scan.y) + this->scan.x * 4;
    this->frame[offset + 0] = (sin(this->cycles / 1000.0) + 1) * 125;
    this->frame[offset + 1] = (this->scan.y / float(240)) * 255;
    this->frame[offset + 2] = (this->scan.x / float(256)) * 255;

    // always increment x
    this->scan.x += 1;
    this->scan.x %= 256;

    // increment y when x = 0
    this->scan.y += (this->scan.x == 0 ? 1 : 0);
    this->scan.y %= 240;
}

const uint8* PPU::getFrame() const { return this->frame; }

uint8 PPU::read(uint16 addr) {
    assert((addr >= 0x2000 && addr <= 0x2007) || addr == 0x4014);

    switch (addr){
        case 0x2002: break; //PPUSTATUS
        case 0x2004: break; //OAMDATA
        case 0x2007: break; //PPUDATA
        default:
            break;
    }

    return 0x0;
}

uint8 PPU::peek(uint16 addr) const {
    assert((addr >= 0x2000 && addr <= 0x2007) || addr == 0x4014);

    switch (addr){
        case 0x2002: break; //PPUSTATUS
        case 0x2004: break; //OAMDATA
        case 0x2007: break; //PPUDATA
        default:
            break;
    }

    return 0x0;
}

void PPU::write(uint16 addr, uint8 val) {
    assert((addr >= 0x2000 && addr <= 0x2007) || addr == 0x4014);

    switch (addr){
        case 0x2000: break; //PPUCTRL
        case 0x2001: break; //PPUMASK
        case 0x2003: break; //OAMAADDR
        case 0x2004: break; //OAMDATA
        case 0x2005: break; //PPUSCROLL
        case 0x2006: break; //PPUADDR
        case 0x2007: break; //PPUDATA
        case 0x4014: //OAMDMA
        {
            this->dma.write(addr, val);
            // DMA takes 513 / 514 CPU cycles (+1 cycle if starting on an odd CPU cycle)
            // The CPU doesn't do anthhing at that time, but the PPU does!
            uint dma_cycles = 513 + ((this->cycles / 3) % 2);
            for (uint i = 0; i < dma_cycles; i++)
                this->cycle();
        }   break;
        default: break;
    }
}