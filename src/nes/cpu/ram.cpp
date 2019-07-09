//
//  ram.cpp
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include <assert.h>
#include "ram.hpp"

Ram::Ram(uint32 ram_size) {
    if(ram_size > 0xFFFF + 1){
        ram_size = 0xFFFF + 1;
    }
 
    this->size = ram_size;
    this->ram = new uint8[this->size];
    
    //Init Ram
    for(uint32 addr = 0; addr < this->size; addr++){
        this->ram[addr] = 0x00;
    }
}

Ram::~Ram(){
    this->size = 0;
    delete[] this->ram;
}

uint8 Ram::read(uint16 addr){
    assert(addr < this->size);
    
    return this->ram[addr];
}

void Ram::write(uint16 addr, uint8 val){
    assert(addr < this->size);
    
    this->ram[addr] = val;
}

void Ram::clear(){
    //Reset ram to default value
    for(uint32 addr = 0; addr < this->size; addr++){
        this->ram[addr] = 0x00;
    }
}
