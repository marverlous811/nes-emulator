//
//  nes.hpp
//  MyNes
//
//  Created by Ows on 7/6/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef nes_hpp
#define nes_hpp

#include "./loadRom/Cartridge.hpp"
#include "./cpu/cpu.hpp"
#include "./cpu/cpu_mmu.hpp"
#include "./cpu/ram.hpp"
#include "./memory_component/ppu/ppu_mmu.hpp"
#include "./memory_component/ppu/ppu.hpp"
#include "./memory_component/dma/dma.hpp"
#include "../util/util.h"

int startNes(char* path);

class Nes{
private:
    /*----------  Borrowed Resources  -----------*/
    Cartridge* cart;
    /*------------  Owned Resources  ------------*/
    //CPU
    CPU *cpu;
    PPU *ppu;
    //RAM
    Ram* cpu_wram;   //2k CPU general purpose RAM
    Ram* ppu_pram;   //32 bytes PPU palette RAM
    Ram* ppu_vram;   //2k PPU nametable VRAM
    Ram* ppu_oam;    //256 bytes PPU object attrubute Memory (OAM)

    //JOY joy
    //CPU MMU
    CPU_MMU *cpu_mmu;
    //PPU MMU
    PPU_MMU* ppu_mmu;
    //DMA MMU
    DMA* dma;
    
    /*-----------------  Flags  -----------------*/
    bool is_running;
    uint32 clock_cycles;
    
public:
    Nes();
    ~Nes();
    
    bool loadCartridge(Cartridge *cart);
    
    void power_cycle();  // Set all volatile components to default power_on state
    void reset();        // Set all volatile components to default reset state
    void start();        // start execution
    void stop();         // stop execution

    void cycle();        //run a single clock cycle
    void step_frame();   //run the nes until there is a new frame to display
                         //(calls cycle() internally)

    const uint8* getFrame() const;
    
    bool isRunning() const;
};

#endif /* nes_hpp */
