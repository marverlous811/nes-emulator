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
#include "./nes/nes.hpp"
#include "./util/util.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    if(argc < 2){
        std::cerr<<"usage: anes [./nes]\n";
        return -1;
    }
    
    std::ifstream rom_file(argv[1]);
    if(!rom_file.is_open()){
        std::cerr << "could not open '" << argv[1] << "'\n";
        return -1;
    }
    
//    // Read in data from data_stream of unknown size
//    uint8* data = nullptr;
//    uint32 data_len = 0;
//
//    const uint CHUNK_SIZE = 0x4000;
//    for (uint chunk = 0; rom_file_stream; chunk++) {
//        // Allocate new data
//        uint8* new_data = new uint8 [CHUNK_SIZE * (chunk + 1)];
//        data_len = CHUNK_SIZE * (chunk + 1);
//        // Copy old data into the new data
//        for (uint i = 0; i < CHUNK_SIZE * chunk; i++)
//            new_data[i] = data[i];
//
//        delete data;
//        data = new_data;
//
//        // Read another chunk into the data
//        rom_file_stream.read((char*) data + CHUNK_SIZE * chunk, CHUNK_SIZE);
//    }
    
    // get length of file
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
    BLOW_ON_CONTACTS(rom_cart);
    BLOW_ON_CONTACTS(rom_cart);
    
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
