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
#include "./cpu/ram.hpp"
#include "../util/util.h"

class Nes{
private:
    /*----------  Borrowed Resources  -----------*/
    Cartridge* cart;
    /*------------  Owned Resources  ------------*/
    //CPU
    Ram *cpu_ram;
    CPU_MMU *cpu_mmu;
    
    /*-----------------  Flags  -----------------*/
    bool is_running;
    
public:
    Nes();
    ~Nes();
    
    bool loadCartridge(Cartridge *cart);
    
    void start();
    void stop();
    
    bool isRunning() const;
};

#endif /* nes_hpp */
