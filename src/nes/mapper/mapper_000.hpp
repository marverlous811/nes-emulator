//
//  mapper_000.hpp
//  MyNes
//
//  Created by Ows on 7/2/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef mapper_000_hpp
#define mapper_000_hpp

#include "mapper.hpp"
#include "../../util/util.h"

class Mapper_000 final : public Mapper{
public:
    Mapper_000(const INES* rom_file);
    ~Mapper_000();
    
    uint8 read(uint16 addr) override;
    void write(uint16 addr, uint8 val) override;
};

#endif /* mapper_000_hpp */
