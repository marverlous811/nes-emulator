//
//  mapper.cpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include "mapper.hpp"
#include "./mapper_000.hpp"

Mapper* Mapper::Factory(const INES *rom_file){
    if(rom_file->is_valid == false)
        return nullptr;
    
    switch (rom_file->mapper) {
        case 0:
            return new Mapper_000(rom_file);
            
        default:
            break;
    }
    
    return nullptr;
}
