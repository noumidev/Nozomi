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

#include "memory.hpp"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <list>

#include <plog/Log.h>

namespace sys::memory {

// Page tables
std::array<u8 *, PAGE_NUM> readTable, writeTable;

std::list<MemoryBlock> memoryBlockRecord;

u64 appSize = 0;
u64 heapSize = 0;
u64 usedMemorySize = 0;

const char *getPermissionString(u32 permission) {
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

void init() {
    // Clear page tables
    for (auto &i : readTable) {
        i = NULL;
    }

    for (auto &i : writeTable) {
        i = NULL;
    }
}

u64 getAppSize() {
    return appSize;
}

u64 getHeapSize() {
    return heapSize;
}

u64 getUsedMemorySize() {
    return usedMemorySize;
}

void setAppSize(u64 size) {
    appSize = size;
}

void setHeapSize(u64 size) {
    if (heapSize != 0) {
        PLOG_FATAL << "Unimplemented heap resizing";

        exit(0);
    }

    PLOG_DEBUG << "Set heap size (size = " << std::hex << size << ")";

    if (allocate(MemoryBase::Heap, size / PAGE_SIZE, 0, 0, MemoryPermission::RW) == NULL) {
        PLOG_FATAL << "Failed to allocate heap";

        exit(0);
    }

    heapSize = size;
}

u8 read8(u64 vaddr) {
    if (vaddr >= MemoryBase::AddressSpace) {
        PLOG_FATAL << "Read8 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (readTable[page] != NULL) {
        return readTable[page][vaddr & PAGE_MASK];
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized read8 (addr = " << std::hex << vaddr << ")";

                exit(0);
        }
    }
}

u16 read16(u64 vaddr) {
    if (vaddr > (MemoryBase::AddressSpace - sizeof(u16))) {
        PLOG_FATAL << "Read16 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (readTable[page] != NULL) {
        u16 data;
        std::memcpy(&data, &readTable[page][vaddr & PAGE_MASK], sizeof(u16));
        
        return data;
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized read16 (addr = " << std::hex << vaddr << ")";

                exit(0);
        }
    }
}

u32 read32(u64 vaddr) {
    if (vaddr > (MemoryBase::AddressSpace - sizeof(u32))) {
        PLOG_FATAL << "Read32 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (readTable[page] != NULL) {
        u32 data;
        std::memcpy(&data, &readTable[page][vaddr & PAGE_MASK], sizeof(u32));
        
        return data;
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized read32 (addr = " << std::hex << vaddr << ")";

                exit(0);
        }
    }
}

u64 read64(u64 vaddr) {
    if (vaddr > (MemoryBase::AddressSpace - sizeof(u64))) {
        PLOG_FATAL << "Read64 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (readTable[page] != NULL) {
        u64 data;
        std::memcpy(&data, &readTable[page][vaddr & PAGE_MASK], sizeof(u64));
        
        return data;
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized read64 (addr = " << std::hex << vaddr << ")";

                exit(0);
        }
    }
}

void write8(u64 vaddr, u8 data) {
    if (vaddr >= MemoryBase::AddressSpace) {
        PLOG_FATAL << "Write8 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (writeTable[page] != NULL) {
        writeTable[page][vaddr & PAGE_MASK] = data;
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized write8 (addr = " << std::hex << vaddr << ", data = " << data << ")";

                exit(0);
        }
    }
}

void write16(u64 vaddr, u16 data) {
    if (vaddr > (MemoryBase::AddressSpace - sizeof(u16))) {
        PLOG_FATAL << "Write16 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (writeTable[page] != NULL) {
        std::memcpy(&writeTable[page][vaddr & PAGE_MASK], &data, sizeof(u16));
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized write16 (addr = " << std::hex << vaddr << ", data = " << data << ")";

                exit(0);
        }
    }
}

void write32(u64 vaddr, u32 data) {
    if (vaddr > (MemoryBase::AddressSpace - sizeof(u32))) {
        PLOG_FATAL << "Write16 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (writeTable[page] != NULL) {
        std::memcpy(&writeTable[page][vaddr & PAGE_MASK], &data, sizeof(u32));
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized write32 (addr = " << std::hex << vaddr << ", data = " << data << ")";

                exit(0);
        }
    }
}

void write64(u64 vaddr, u64 data) {
    if (vaddr > (MemoryBase::AddressSpace - sizeof(u64))) {
        PLOG_FATAL << "Write16 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    if (writeTable[page] != NULL) {
        std::memcpy(&writeTable[page][vaddr & PAGE_MASK], &data, sizeof(u64));
    } else {
        switch (vaddr) {
            default:
                PLOG_FATAL << "Unrecognized write64 (addr = " << std::hex << vaddr << ", data = " << data << ")";

                exit(0);
        }
    }
}

// Allocates linear block of memory, returns pointer to allocated block (or NULL)
void *allocate(u64 baseAddress, u64 pageNum, u32 type, u32 attribute, u32 permission) {
    PLOG_DEBUG << "Allocating " << pageNum << " pages @ " << std::hex << baseAddress << " " << getPermissionString(permission);

    if (!isAligned(baseAddress)) {
        PLOG_ERROR << "Base address is not aligned";

        return NULL;
    }

    if (pageNum == 0) {
        PLOG_ERROR << "Requested zero pages";

        return NULL;
    }

    MemoryBlock memoryBlock{.baseAddress = baseAddress, .size = pageNum, .type = type, .attribute = attribute, .permission = permission, .mem = NULL};
    memoryBlock.mem = std::malloc(pageNum * PAGE_SIZE);

    if (memoryBlock.mem == NULL) {
        PLOG_ERROR << "Failed to allocate memory";

        return NULL;
    }

    const u64 basePage = baseAddress >> PAGE_SHIFT;

    if (((memoryBlock.permission & MemoryPermission::R) != 0) || ((memoryBlock.permission & MemoryPermission::X) != 0)) {
        for (u64 page = 0; page < pageNum; page++) {
            const u64 readPage = page + basePage;

            if (readTable[readPage] != NULL) {
                PLOG_FATAL << "Failed to map read page";

                exit(0);
            }

            readTable[readPage] = &((u8 *)memoryBlock.mem)[page * PAGE_SIZE];
        }
    }

    if ((memoryBlock.permission & MemoryPermission::W) != 0) {
        for (u64 page = 0; page < pageNum; page++) {
            const u64 writePage = page + basePage;

            if (writeTable[writePage] != NULL) {
                PLOG_FATAL << "Failed to map write page";

                exit(0);
            }

            writeTable[writePage] = &((u8 *)memoryBlock.mem)[page * PAGE_SIZE];
        }
    }

    usedMemorySize += pageNum * PAGE_SIZE;

    if (usedMemorySize > TOTAL_MEMORY_SIZE) {
        PLOG_FATAL << "Ran out of memory";

        exit(0);
    }

    memoryBlockRecord.push_back(memoryBlock);

    return memoryBlock.mem;
}

}
