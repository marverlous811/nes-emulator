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
#include "./ppu/ppu_mmu.hpp"
#include "./ppu/ppu.hpp"
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
    Ram* ppu_pram;  //32 bytes PPU palette RAM
    Ram* ppu_vram; //2k PPU nametable VRAM

    //JOY joy
    //CPU MMU
    CPU_MMU *cpu_mmu;
    PPU_MMU* ppu_mmu;
    //PPU MMU
    //DMA MMU
    
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
