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

#include "memory_manager.hpp"

#include <cstdlib>
#include <ios>
#include <unordered_map>

#include <plog/Log.h>

#include "memory.hpp"

namespace sys::gpu::memory_manager {

std::unordered_map<u64, u8 *> pages;

void *getPage(u64 iova) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        return pages[page];
    }

    PLOG_FATAL << "Invalid GPU page " << std::hex << page;

    exit(0);
}

u8 read8(u64 iova) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        return pages[page][iova & sys::memory::PAGE_MASK];
    }

    PLOG_FATAL << "Unrecognized GPU read8 (address = " << std::hex << iova << ")";

    exit(0);
}

u16 read16(u64 iova) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        u16 data;
        std::memcpy(&data, &pages[page][iova & sys::memory::PAGE_MASK], sizeof(u16));
        
        return data;
    }

    PLOG_FATAL << "Unrecognized GPU read16 (address = " << std::hex << iova << ")";

    exit(0);
}

u32 read32(u64 iova) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        u32 data;
        std::memcpy(&data, &pages[page][iova & sys::memory::PAGE_MASK], sizeof(u32));
        
        return data;
    }

    PLOG_FATAL << "Unrecognized GPU read32 (address = " << std::hex << iova << ")";

    exit(0);
}

u64 read64(u64 iova) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        u64 data;
        std::memcpy(&data, &pages[page][iova & sys::memory::PAGE_MASK], sizeof(u64));
        
        return data;
    }

    PLOG_FATAL << "Unrecognized GPU read64 (address = " << std::hex << iova << ")";

    exit(0);
}

void write8(u64 iova, u8 data) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        pages[page][iova & sys::memory::PAGE_MASK] = data;

        return;
    }

    PLOG_FATAL << "Unrecognized GPU write8 (address = " << std::hex << iova << ", data = " << (u32)data <<  ")";

    exit(0);
}

void write16(u64 iova, u16 data) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        std::memcpy(&pages[page][iova & sys::memory::PAGE_MASK], &data, sizeof(u16));

        return;
    }

    PLOG_FATAL << "Unrecognized GPU write16 (address = " << std::hex << iova << ", data = " << data <<  ")";

    exit(0);
}

void write32(u64 iova, u32 data) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        std::memcpy(&pages[page][iova & sys::memory::PAGE_MASK], &data, sizeof(u32));

        return;
    }

    PLOG_FATAL << "Unrecognized GPU write32 (address = " << std::hex << iova << ", data = " << data <<  ")";

    exit(0);
}

void write64(u64 iova, u64 data) {
    const u64 page = iova >> sys::memory::PAGE_SHIFT;

    if (pages.find(page) != pages.end()) {
        std::memcpy(&pages[page][iova & sys::memory::PAGE_MASK], &data, sizeof(u64));

        return;
    }

    PLOG_FATAL << "Unrecognized GPU write64 (address = " << std::hex << iova << ", data = " << data <<  ")";

    exit(0);
}

void map(u64 iova, u64 address, u64 size, u64 align) {
    (void)align;

    const u64 page = iova >> sys::memory::PAGE_SHIFT;
    const u64 pageNum = size / sys::memory::PAGE_SIZE;

    PLOG_INFO << "Mapping " << pageNum << " pages (IOVA = " << std::hex << iova << ", address = " << address << ")";

    for (u64 i = 0; i < pageNum; i++) {
        if (pages.find(page + i) != pages.end()) {
            PLOG_FATAL << "GPU page already mapped";

            exit(0);
        }

        pages[page + i] = (u8 *)sys::memory::getPointer(address + i * sys::memory::PAGE_SIZE);
    }
}

}
