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

constexpr u64 PAGE_SHIFT = 12;
constexpr u64 PAGE_SIZE = 1ULL << PAGE_SHIFT;
constexpr u64 PAGE_MASK = PAGE_SIZE - 1;

namespace MemoryBase {
    enum : u64 {
        Application = 0x80000000,
        AddressSpace = 1ULL << 36,
    };
}

constexpr u64 PAGE_NUM = MemoryBase::AddressSpace >> PAGE_SHIFT;

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

namespace MemoryAttribute {
    enum : u32 {
        Locked = 1 << 0,
        IPCLocked = 1 << 1,
        DeviceShared = 1 << 2,
        Uncached = 1 << 3,
    };
}

struct MemoryBlock {
    u64 baseAddress;
    u64 size; // In pages
    u32 type;
    u32 attribute;
    u32 permission;

    void *mem;
};

inline bool isAligned(u64 n) {
    return (n & PAGE_MASK) == 0;
}

void init();

u8 read8(u64 vaddr);
u16 read16(u64 vaddr);
u32 read32(u64 vaddr);
u64 read64(u64 vaddr);

void write8(u64 vaddr, u8 data);
void write16(u64 vaddr, u16 data);
void write32(u64 vaddr, u32 data);
void write64(u64 vaddr, u64 data);

void *allocate(u64 baseAddress, u64 pageNum, u32 type, u32 attribute, u32 permission);

}
