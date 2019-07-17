//
//  nes.cpp
//  MyNes
//
//  Created by Ows on 7/6/19.
//  Copyright © 2019 hieunq. All rights reserved.
//
#include <iostream>
#include <fstream>
#include "nes.hpp"
#include "../util/fakeMemory.h"

int startNes(char* path){
    std::ifstream rom_file(path, std::ios::binary);
    if(!rom_file.is_open()){
        std::cerr << "could not open '" << path << "'\n";
        return -1;
    }

    rom_file.seekg(0, rom_file.end);
    uint32 data_len = rom_file.tellg();
    rom_file.seekg(0, rom_file.beg);

    uint8* data = new uint8 [data_len];
    rom_file.read((char*) data, data_len);

    // Generate cartridge from data
    Cartridge* rom_cart = new Cartridge(data, data_len);

    if(rom_cart->is_valid()){
        std::cout<< "iNES file loaded successfully!\n";
    }
    else{
        std::cerr << "Given file was not an iNES file!\n";
        return -1;
    }

    Nes* nes = new Nes();
    bool res = nes->loadCartridge(rom_cart);
    std::cout<<"ines load cartidge: "<<res<<std::endl;
    nes->start();

    while (true) {
        nes->step();
        if(nes->isRunning() == false){
            break;
        }
    }

    delete rom_cart;
    return 0;
}

Nes::Nes(){
    this->cart = nullptr;
    // Init RAM modules
    this->cpu_ram = new Ram(0x800);
    
    // Init MMUs
    this->cpu_mmu = new CPU_MMU(
        *this->cpu_ram,
        *VoidMemory::Get(),
        *VoidMemory::Get(),
        *VoidMemory::Get(),
        *VoidMemory::Get(),
        this->cart
    );
    
    //create processor
    this->cpu = new CPU(*this->cpu_mmu);
    
    this->is_running = false;
}

Nes::~Nes(){
    delete this->cpu;
    delete this->cpu_mmu;
    delete this->cpu_ram;
}

bool Nes::loadCartridge(Cartridge *cart){
    if(cart == nullptr || !cart->is_valid()){
        return false;
    }
    
    this->cart = cart;
    this->cpu_mmu->addCartridge(this->cart);
    
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

// Power Cycling initializes all the components to their "power on" state
void Nes::power_cycle(){
    this->is_running = true;
    this->cpu->power_cycle();
    this->cpu_ram->clear();
}

void Nes::reset(){
    this->is_running = true;
    this->cpu->reset();
}

void Nes::step(){
    uint8 cpu_cycles = this->cpu->step();
    if(this->cpu->getState() == CPU::State::Halted){
        this->is_running = false;
    }
}
