//
// Created by Ows on 2019-07-18.
//

#ifndef NES_EMULATOR_PPU_HPP
#define NES_EMULATOR_PPU_HPP

#include "util/util.h"
#include "util/bitfield.h"
#include "util/memory.h"

// http://wiki.nesdev.com/w/index.php/PPU_programmer_reference
class PPU final: public IMemory {
private:
    /*----------  "Hardware"  ----------*/
    // In quotes because technically, these things aren't located on the PPU, but
    // by coupling them with the PPU, it makes the emulator code cleaner

    uint8  frame [ 240 * 256 * 4]; //Pixel buffer
    IMemory& mem;       //PPU 16 bit address space (should be wired to ppu_mmu)
    IMemory& dma;
    IMemory& oam;       //PPU Object Attribute Memory

    uint32 cycles;

    // current pixel to draw
    struct {
        uint16 x;
        uint16 y;
    } scan;

    struct {
        // PPUCTRL   - 0x2000 - PPU control register
        union {
            uint8  raw;
            BitField<7> V; //NMI enable
            BitField<6> P; //PPU master/slave
            BitField<5> H; //sprite height
            BitField<4> B; //background tile select
            BitField<3> S; //sprite tile select
            BitField<2> I; //increment mode
            BitField<0, 2> N; //nametable select
        } ppuctrl;
        // PPUMASK   - 0x2001 - PPU mask register
        union {
            uint8 raw;
            BitField<7> B; // color emphasis Blue
            BitField<6> G; // color emphasis Green
            BitField<5> R; // color emphasis Red
            BitField<4> s; // sprite enable
            BitField<3> b; // background enable
            BitField<2> M; // sprite left column enable
            BitField<1> m; // background left column enable
            BitField<0> g; // greyscale
        } ppumask;
        // PPUSTATUS - 0x2002 - PPU status register
        union {
            uint8 raw;
            BitField<7> V; // vblank
            BitField<6> S; // sprite 0 hit
            BitField<5> O; // sprite overflow
            // the rest are irrelevant
        } ppustatus;

        uint8 oamaddr;   // OAMADDR   - 0x2003 - OAM address port
        uint8 oamdata;   // OAMDATA   - 0x2004 - OAM data port

        // PPUSCROLL - 0x2005 - PPU scrolling position register
        struct {
            bool write_to; // 0 = write to x, 1 = write to y
            uint8 x;
            uint8 y;
        } ppuscroll;
        // PPUADDR   - 0x2006 - PPU VRAM address register
        struct {
            bool write_to; // 0 = write to hi, 1 = write to lo
            union {
                uint16 val;         //16 bit address
                BitField<8, 8> hi;  //hi byte of addr
                BitField<0, 8> lo;  //lo byte of addr
            } addr;
        } ppuaddr;

        uint8 ppudata;   // PPUDATA   - 0x2007 - PPU VRAM data port
    } reg;
public:
    ~PPU();
    PPU(IMemory& mem, IMemory& oam, IMemory& dma);

    void power_cycle();
    void reset();

    void cycle();

    const uint8* getFrame() const;

    //<Memory>
    uint8 read(uint16 addr) override ;
    uint8 peek(uint16 addr) const override ;
    void write(uint16 addr, uint8 val) override ;
    //</Memory>
};


#endif //NES_EMULATOR_PPU_HPP
