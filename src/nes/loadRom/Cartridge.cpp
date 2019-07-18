//
//  Cartridge.cpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "Cartridge.hpp"
#include "INes.hpp"

Cartridge::Cartridge(const uint8* data, uint32 data_len){
    this->rom_data = new INES(data, data_len);
    this->mapper = Mapper::Factory(*this->rom_data);
    if (this->rom_data->flags.has_4screen) {
        this->mirroring_type = Cartridge::Mirroring::FourScreen;
    }
    else if (this->rom_data->flags.mirror_type == 0) {
        this->mirroring_type = Cartridge::Mirroring::Vertical;
    }
    else /* (this->rom_data->flags.mirror_type == 1) */ {
        this->mirroring_type = Cartridge::Mirroring::Horizontal;
    }
}

Cartridge::~Cartridge(){
    delete this->rom_data;
    delete this->mapper;
}

uint8 Cartridge::peek(uint16 addr) const {
    return this->mapper->peek(addr);
}

uint8 Cartridge::read(uint16 addr){
    return this->mapper->read(addr);
}

void Cartridge::write(uint16 addr, uint8 val){
    this->mapper->write(addr, val);
}

bool Cartridge::is_valid() const{
    return this->rom_data->is_valid && this->mapper != nullptr;
}

Cartridge::Mirroring Cartridge::mirroring() const {
    return this->mirroring_type;
}

void Cartridge::blowOnContacts() const {

}