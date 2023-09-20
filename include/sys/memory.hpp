/*
    Nozomi is an experimental HLE Switch emulator.
    Copyright (C) 2023  noumidev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "types.hpp"

namespace sys::memory {

constexpr u64 PAGE_SIZE = 0x4000;
constexpr u64 PAGE_MASK = PAGE_SIZE - 1;

namespace MemoryBase {
    enum : u64 {
        Application = 0x80000000,
    };
}

namespace MemoryPermission {
    enum : u32 {
        None = 0,
        R = 1,
        W = 2,
        X = 4,
        RW = 3,
        RX = 5,
        DontCare = 1 << 28,
    };
}

union MemoryAttribute {
    u32 raw;
    struct {
        u32 locked : 1;
        u32 ipcLocked : 1;
        u32 deviceShared : 1;
        u32 uncached : 1;
        u32 : 28;
    };
};

struct MemoryBlock {
    u32 baseAddress, size;
    u32 permission;
};

bool isAligned(u64 n) {
    return (n & PAGE_MASK) == 0;
}

}
