//
//  main.cpp
//  MyNes
//
//  Created by Ows on 6/27/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include "./nes/loadRom/Cartridge.hpp"
#include "./util/util.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    if(argc < 2){
        std::cerr<<"usage: anes [./nes]\n";
        return -1;
    }
    
    std::ifstream rom_file_stream(argv[1]);
    if(!rom_file_stream.is_open()){
        std::cerr << "could not open '" << argv[1] << "'\n";
        return -1;
    }
    
    // Read in data from data_stream of unknown size
    uint8* data = nullptr;
    uint32 data_len = 0;
    
    const uint CHUNK_SIZE = 0x4000;
    for (uint chunk = 0; rom_file_stream; chunk++) {
        // Allocate new data
        uint8* new_data = new uint8 [CHUNK_SIZE * (chunk + 1)];
        data_len = CHUNK_SIZE * (chunk + 1);
        // Copy old data into the new data
        for (uint i = 0; i < CHUNK_SIZE * chunk; i++)
            new_data[i] = data[i];
        
        delete data;
        data = new_data;
        
        // Read another chunk into the data
        rom_file_stream.read((char*) data + CHUNK_SIZE * chunk, CHUNK_SIZE);
    }
    
    // Generate cartridge from data
    Cartridge rom_cart (data, data_len);
    
    if(rom_cart.is_valid()){
        std::cout<< "iNES file loaded successfully!\n";
    }
    else{
        std::cerr << "Given file was not an iNES file!\n";
        return -1;
    }
    return 0;
}
