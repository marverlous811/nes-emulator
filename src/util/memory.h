//
//  memory.h
//  MyNes
//
//  Created by Ows on 7/5/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef memory_h
#define memory_h

#include "util.h"

class IMemory{
public:
    virtual uint8 peek(uint16 addr) const = 0; //read, but without the side-effects
    virtual uint8 read(uint16 addr) = 0;
    virtual void write(uint16 addr, uint8 val) = 0;
};

#endif /* memory_h */
