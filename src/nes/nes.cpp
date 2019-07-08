//
//  nes.cpp
//  MyNes
//
//  Created by Ows on 7/6/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "nes.hpp"
#include "../util/fakeMemory.h"

Nes::Nes(){
    this->cart = nullptr;
    // Init RAM modules
    this->cpu_ram = new Ram(0x800);
    
    // Init MMUs
    this->cpu_mmu = new CPU_MMU(
        *new MemorySniffer("CPU -> RAM", this->cpu_ram),
        *new MemorySniffer("CPU -> RAM", VoidMemory::Get()),
        *new MemorySniffer("CPU -> RAM", VoidMemory::Get()),
        *new MemorySniffer("CPU -> RAM", VoidMemory::Get()),
        *new MemorySniffer("CPU -> RAM", VoidMemory::Get()),
        new MemorySniffer("CPU -> RAM", this->cart)
    );
}

Nes::~Nes(){
    delete this->cpu_mmu;
    delete this->cpu_ram;
}

bool Nes::loadCartridge(Cartridge *cart){
    if(cart == nullptr || !cart->is_valid()){
        return false;
    }
    
    this->cart = cart;
    this->cpu_mmu->addCartridge(cart);
    
    return true;
}

void Nes::start(){
    this->is_running = true;
}

void Nes::stop(){
    this->is_running = false;
}

bool Nes::isRunning() const{
    return this->is_running;
}
