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

constexpr u64 TOTAL_MEMORY_SIZE = 1LLU << 32;

namespace MemoryBase {
    enum : u64 {
        HomebrewEnv = 1ULL << 28,
        Stack = 1ULL << 29,
        TLSBase = 1ULL << 30,
        Application = 1ULL << 31,
        Heap = 1ULL << 32,
        AddressSpace = 1ULL << 36,
    };
}

constexpr u64 PAGE_NUM = MemoryBase::AddressSpace >> PAGE_SHIFT;
constexpr u64 STACK_PAGES = 32;

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

inline bool isAlignedHeap(u64 n) {
    constexpr u64 HEAP_PAGE_MASK = 0x200000 - 1;

    return (n & HEAP_PAGE_MASK) == 0;
}


inline const char *getPermissionString(u32 permission) {
    switch (permission) {
        case MemoryPermission::R:
            return "(R/-/-)";
        case MemoryPermission::W:
            return "(-/W/-)";
        case MemoryPermission::X:
            return "(-/-/X)";
        case MemoryPermission::RW:
            return "(R/W/-)";
        case MemoryPermission::RX:
            return "(R/-/X)";
        case MemoryPermission::None:
        case MemoryPermission::DontCare:
        default:
            return "(-/-/-)";
    }
}

void init();

u64 getAppSize();
u64 getHeapSize();
u64 getUsedMemorySize();

void setAppSize(u64 size);
void setHeapSize(u64 size);

u8 read8(u64 vaddr);
u16 read16(u64 vaddr);
u32 read32(u64 vaddr);
u64 read64(u64 vaddr);

void write8(u64 vaddr, u8 data);
void write16(u64 vaddr, u16 data);
void write32(u64 vaddr, u32 data);
void write64(u64 vaddr, u64 data);

void *getPointer(u64 vaddr);

void map(void *mem, u64 address, u64 pageNum, u32 type, u32 attribute, u32 permission);
void remap(u64 srcAddress, u64 dstAddress, u64 pageNum);
void unmap(u64 address, u64 pageNum);

void *allocate(u64 baseAddress, u64 pageNum, u32 type, u32 attribute, u32 permission);

MemoryBlock queryMemory(u64 addr);

}
