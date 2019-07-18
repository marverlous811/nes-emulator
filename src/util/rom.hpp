//
// Created by Ows on 2019-07-18.
//

#ifndef NES_EMULATOR_ROM_HPP
#define NES_EMULATOR_ROM_HPP

#include "util.h"
#include "memory.h"

class ROM final : public IMemory {
private:
    uint8* rom;
    uint32  size;
public:
    ROM(uint32 rom_size, const uint8* content);
    ~ROM();

    //<Memory>
    uint8 read(uint16 addr) override;
    uint8 peek(uint16 addr) const override;
    void write(uint16 addr, uint8 val) override;
    //</Memory>

    // Also provide a const read method (for when there is a `const ROM` type)
    uint8 read(uint16 addr) const;
};


#endif //NES_EMULATOR_ROM_HPP
