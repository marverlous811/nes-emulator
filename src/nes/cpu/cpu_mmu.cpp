//
//  cpu_mmu.cpp
//  MyNes
//
//  Created by Ows on 7/9/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "cpu_mmu.hpp"
#include <assert.h>

CPU_MMU::CPU_MMU(
                 IMemory& ram,
                 IMemory& ppu,
                 IMemory& apu,
                 IMemory& dma,
                 IMemory& joy,
                 Cartridge* rom
                 ): ram(ram), ppu(ppu), apu(apu), dma(dma), joy(joy), rom(rom)
{}

uint8 CPU_MMU::peek(uint16 addr) const {
    if(in_range(addr, 0x0000, 0x07FF)) return ram.peek(addr);
    if(in_range(addr, 0x2000, 0x2007)) return ppu.peek(addr);
    if(in_range(addr, 0x4000, 0x4013)) return apu.peek(addr);
    if(in_range(addr, 0x4014)) return dma.peek(addr);
    if(in_range(addr, 0x4015)) return apu.peek(addr);
    if(in_range(addr, 0x4016)) return joy.peek(addr);
    if(in_range(addr, 0x4017)) return joy.peek(addr);
    if(in_range(addr, 0x4018, 0xFFFF)) return rom ? rom->peek(addr) : 0x0;;

    assert(false);
    return 0;
}

// 0x0000 ... 0x1FFF: 0x0000 - 0x07FF are RAM           (Mirrored 4x)
// 0x2000 ... 0x3FFF: 0x2000 - 0x2007 are PPU Regusters (Mirrored every 8 bytes)
// 0x4000 ... 0x4013: APU registers
// 0x4014           : DMA
// 0x4015           : APU register
// 0x4016           : Joy1 Data (Read) and Joystick Strobe (Write)
// 0x4017           : Joy2 Data (Read) and APU thing (Write)
// 0x4018 ... 0xFFFF: Cartridge ROM (may not be plugged in)
uint8 CPU_MMU::read(uint16 addr){
    if(in_range(addr, 0x0000, 0x07FF)) return ram.read(addr);
    if(in_range(addr, 0x2000, 0x2007)) return ppu.read(addr);
    if(in_range(addr, 0x4000, 0x4013)) return apu.read(addr);
    if(in_range(addr, 0x4014)) return dma.read(addr);
    if(in_range(addr, 0x4015)) return apu.read(addr);
    if(in_range(addr, 0x4016)) return joy.read(addr);
    if(in_range(addr, 0x4017)) return joy.read(addr);
    if(in_range(addr, 0x4018, 0xFFFF)) return rom ? rom->read(addr) : 0x0;;
    
    assert(false);
    return 0;
}

void CPU_MMU::write(uint16 addr, uint8 val){
    if(in_range(addr, 0x0000, 0x07FF)) return ram.write(addr, val);
    if(in_range(addr, 0x2000, 0x2007)) return ppu.write(addr, val);
    if(in_range(addr, 0x4000, 0x4013)) return apu.write(addr, val);
    if(in_range(addr, 0x4014)) return dma.write(addr, val);
    if(in_range(addr, 0x4015)) return apu.write(addr, val);
    if(in_range(addr, 0x4016)) return joy.write(addr, val);
    if(in_range(addr, 0x4017)) return apu.write(addr, val);
    if(in_range(addr, 0x4018, 0xFFFF)) return rom ? rom->write(addr, val) : void();
    
    assert(false);
    return void();
}

void CPU_MMU::addCartridge(Cartridge *cart){
    this->rom = cart;
}

void CPU_MMU::removeCartridge(){
    this->rom = nullptr;
}
