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

    //Derived memory accesss
    uint16 peek16(uint16 addr) const {
        return this->peek(addr + 0) | (this->peek(addr + 1) << 8);
    }

    uint16 read16(uint16 addr) {
        return this->read(addr + 0) | (this->read(addr + 1) << 8);
    }

    uint16 peek16_zpg(uint16 addr){
        return this->peek(addr + 0) | (this->peek((addr & 0xFF00) | (addr + 1 & 0x00FF)) << 8);
    }

    uint16 read16_zpg(uint16 addr){
        return this->read(addr + 1) | (this->read((addr & 0xFF00) | (addr + 1 & 0x00FF)) << 8);
    }

    void write16(uint16 addr, uint8 val){
        this->write(addr + 0, val);
        this->write(addr + 1, val);
    }

    // ---- Overloaded Array Subscript Operator ---- //
    // If we want to define this overload at the interface level, things get hard.
    // See, the Array Subscript operator is supposed to return a &u8, so that
    // reads and write to it modify that value.
    // But that's not good at all.
    //
    // How would we detect when reads / writes happen (for side-effects)?
    // How do we handle memory objects that don't have a "physical" value
    //
    // Easy! Instead of returning the u8 reference, return something that "looks"
    // like a u8 reference!
    // These thin "ref" classes overload `operator u8()` and `operator =` with
    // calls to Memory::read and Memory::write instead!
    class ref;          //allow read and write
    class const_ref;    //only for peek

    ref operator[](uint16 addr) { return ref(this, addr); }
    const_ref operator[] (uint16 addr) const{ return const_ref(this, addr); }

    class ref{
    private:
        friend class IMemory;

        IMemory* self;
        uint16 addr;

        ref(IMemory* self, uint16 addr)
            : self(self) , addr(addr) {}

    public:
        ~ref() = default;

        //read
        operator uint8 () const { return self->read(addr); }

        //write
        ref& operator= (uint8 val) { self->write(addr, val); return *this; }
        ref& operator= (const ref& val) { self->write(addr, val); return *this;}
    };

    class const_ref {
    private:
        friend class IMemory;

        const IMemory* self;
        uint16 addr;

        const_ref(const IMemory* self, uint16 addr)
            : self(self) , addr(addr) {}

    public:
        ~const_ref() = default;

        //Peek
        operator uint8() const { return self->peek(addr); }

        // Disallow all write operations
        template <typename T>
        const_ref& operator= (const T&) = delete;
    };
};

#endif /* memory_h */
