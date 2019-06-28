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
#include "./nes/loadRom/LoadRom.hpp"
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
    
    INES rom_file(rom_file_stream);
    if(rom_file.is_valid){
        std::cout<< "iNES file loaded successfully!\n";
    }
    else{
        std::cerr << "Given file was not an iNES file!\n";
        return -1;
    }
    return 0;
}
