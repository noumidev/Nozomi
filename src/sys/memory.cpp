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
        PLOG_FATAL << "Write32 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

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
        PLOG_FATAL << "Write64 address outside of address space bounds (addr = " << std::hex << vaddr << ")";

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

void *getPointer(u64 vaddr) {
    if (vaddr >= MemoryBase::AddressSpace) {
        PLOG_FATAL << "Pointer address outside of address space bounds (addr = " << std::hex << vaddr << ")";

        exit(0);
    }

    const u64 page = vaddr >> PAGE_SHIFT;

    // Check both read and write tables
    if (readTable[page] != 0) {
        return (void *)&readTable[page][vaddr & PAGE_MASK];
    } else if (writeTable[page] != 0) {
        return (void *)&writeTable[page][vaddr & PAGE_MASK];
    }

    PLOG_FATAL << "Invalid pointer (addr = " << std::hex << vaddr << ")";

    exit(0);
}

void map(void *mem, u64 address, u64 pageNum, u32 type, u32 attribute, u32 permission) {
    PLOG_DEBUG << "Mapping " << pageNum << " pages @ " << std::hex << address << " " << getPermissionString(permission);

    MemoryBlock memoryBlock{.baseAddress = address, .size = pageNum, .type = type, .attribute = attribute, .permission = permission, .mem = mem};

    const u64 basePage = address >> PAGE_SHIFT;

    if (((memoryBlock.permission & MemoryPermission::R) != 0) || ((memoryBlock.permission & MemoryPermission::X) != 0)) {
        for (u64 page = 0; page < pageNum; page++) {
            const u64 readPage = page + basePage;

            if (readTable[readPage] != NULL) {
                PLOG_FATAL << "Read page " << std::hex << readPage << " is already mapped!";

                exit(0);
            }

            readTable[readPage] = &((u8 *)memoryBlock.mem)[page * PAGE_SIZE];
        }
    }

    if ((memoryBlock.permission & MemoryPermission::W) != 0) {
        for (u64 page = 0; page < pageNum; page++) {
            const u64 writePage = page + basePage;

            if (writeTable[writePage] != NULL) {
                PLOG_FATAL << "Write page " << std::hex << writePage << " is already mapped!";

                exit(0);
            }

            writeTable[writePage] = &((u8 *)memoryBlock.mem)[page * PAGE_SIZE];
        }
    }

    memoryBlockRecord.push_back(memoryBlock);
}

void remap(u64 srcAddress, u64 dstAddress, u64 pageNum) {
    PLOG_DEBUG << "Remapping " << pageNum << " pages from " << std::hex << srcAddress << " to " << dstAddress;

    const MemoryBlock memoryBlock = queryMemory(srcAddress);

    const u64 srcPage = srcAddress >> PAGE_SHIFT;
    const u64 dstPage = dstAddress >> PAGE_SHIFT;

    if (((memoryBlock.permission & MemoryPermission::R) != 0) || ((memoryBlock.permission & MemoryPermission::X) != 0)) {
        for (u64 page = 0; page < pageNum; page++) {
            const u64 readPage = page + dstPage;

            if (readTable[readPage] != NULL) {
                PLOG_FATAL << "Read page " << std::hex << readPage << " is already mapped!";

                exit(0);
            }

            readTable[readPage] = readTable[page + srcPage];
        }
    }

    if ((memoryBlock.permission & MemoryPermission::W) != 0) {
        for (u64 page = 0; page < pageNum; page++) {
            const u64 writePage = page + dstPage;

            if (writeTable[writePage] != NULL) {
                PLOG_FATAL << "Write page " << std::hex << writePage << " is already mapped!";

                exit(0);
            }

            writeTable[writePage] = writeTable[page + srcPage];
        }
    }

    unmap(srcAddress, pageNum);
}

void unmap(u64 address, u64 pageNum) {
    PLOG_DEBUG << "Unmapping " << pageNum << " pages @ " << std::hex << address;

    // Delete memory block
    for (auto it = memoryBlockRecord.begin(); it != memoryBlockRecord.end(); it++) {
        if (it->baseAddress == address) {
            memoryBlockRecord.erase(it);
            break;
        }

        if (it == memoryBlockRecord.end()) {
            PLOG_FATAL << "Unable to find memory block with base address " << std::hex << address;

            exit(0);
        }
    }

    const u64 basePage = address >> PAGE_SHIFT;

    for (u64 page = basePage; page < (basePage + pageNum); page++) {
        readTable[page] = NULL;
        writeTable[page] = NULL;
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

u64 allocateTLS() {
    static u64 TLS_BASE = MemoryBase::TLSBase;

    (void)allocate(TLS_BASE, 1, 0, 0, MemoryPermission::RW);

    const u64 tlsBase = TLS_BASE;

    TLS_BASE += PAGE_SIZE;

    return tlsBase;
}

MemoryBlock queryMemory(u64 addr) {
    PLOG_VERBOSE << "Querying memory (addr = " << std::hex << addr << ")";

    for (const auto &memoryBlock : memoryBlockRecord) {
        if ((addr >= memoryBlock.baseAddress) && (addr < (memoryBlock.baseAddress + PAGE_SIZE * memoryBlock.size))) {
            return memoryBlock;
        }
    }

    PLOG_VERBOSE << "Memory block does not exist";

    // Returning an address-size pair outside the address space seems to do the trick
    return MemoryBlock{.baseAddress = MemoryBase::AddressSpace, .size = MemoryBase::AddressSpace, .type = 0, .attribute = 0, .permission = 0, .mem = NULL};
}

}
