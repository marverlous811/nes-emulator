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

    startNes((char*)argv[1]);

    return 0;
}
