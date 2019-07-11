//
//  util.h
//  MyNes
//
//  Created by Ows on 6/27/19.
//  Copyright © 2019 hieunq. All rights reserved.
//

#ifndef util_h
#define util_h
#pragma once

#include <cstdint>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

template <typename T>
inline bool nth_bit(T x, uint8 n) {
    return (x >> n ) & 1;
}

//check if number is in range
template <typename T1, typename T2>
bool in_range (T1 x, T2 min, T2 max) { return x >= min && x <= max; }
template <typename T1, typename T2>
bool in_range (T1 x, T2 val) { return x == val; }

#define BLOW_ON_CONTACTS(cartridge)
#endif /* util_h */
