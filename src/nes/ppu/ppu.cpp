//
// Created by Ows on 2019-07-18.
//

#include "ppu.hpp"

PPU::~PPU() {}
PPU::PPU(IMemory &mem) : mem(mem) {
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