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

#endif /* util_h */
