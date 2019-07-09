//
//  cpu_mmu.hpp
//  MyNes
//
//  Created by Ows on 7/9/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef cpu_mmu_hpp
#define cpu_mmu_hpp

#include "../../util/util.h"
#include "../../util/memory.h"
#include "../loadRom/Cartridge.hpp"
#include "./ram.hpp"

// CPU Memory Map (MMU)
// NESdoc.pdf
// https://wiki.nesdev.com/w/index.php/CPU_memory_map
// https://wiki.nesdev.com/w/index.php/2A03
class CPU_MMU final : public Memory{
private:
    // Fixed Referenced (these will never be invalidated)
    Memory& ram;
    Memory& ppu;
    Memory& apu;
    Memory& dma;
    Memory& joy;
    
    // ROM is subject to change
    Memory* rom;
public:
    CPU_MMU(
            Memory& ram,
            Memory& ppu,
            Memory& apu,
            Memory& dma,
            Memory& joy,
            Memory* rom
            );
    
    uint8 read(uint16 addr) override;
    void write(uint16 addr, uint8 val) override;
    
    void addCartridge(Memory* cart);
    void removeCartridge();
};

#endif /* cpu_mmu_hpp */
