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

#include "nro.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

#include "memory.hpp"

namespace loader::nro {

constexpr const char *NRO_MAGIC = "NRO0";

constexpr u64 NRO_HEADER_SIZE = 0x80;

namespace HeaderOffset {
    enum {
        Magic = 0x10,
        TextSegment = 0x20,
        RoSegment = 0x28,
        DataSegment = 0x30,
        BssSize = 0x38,
        ApiInfoSegment = 0x68,
        DynstrSegment = 0x70,
        DynsymSegment = 0x78,
    };
}

namespace HeaderFieldSize {
    enum {
        Magic = 4,
        SegmentHeader = 8,
        BssSize = 4,
    };
}

union Segment {
    u64 raw;
    struct {
        u64 offset : 32;
        u64 size : 32;
    };
};

static_assert(sizeof(Segment) == HeaderFieldSize::SegmentHeader);

namespace EnvContextKey {
    enum : u32 {
        EndOfList = 0,
        MainThreadHandle = 1,
        AppletType = 7,
    };
}

struct EnvContextEntry {
    u32 key, flags;
    u64 value[2];
} __attribute__((packed));

// Dummy list of entries for NRO/homebrew
constexpr EnvContextEntry envContextTable[] = {
    EnvContextEntry{.key = EnvContextKey::MainThreadHandle, .flags = 1, .value{1, 0}}, // Main thread handle = 1
    EnvContextEntry{.key = EnvContextKey::AppletType, .flags = 1, .value{0, 0}},       // Applet type = Application
    EnvContextEntry{.key = EnvContextKey::EndOfList, .flags = 1, .value{0, 0}},
};

void load(FILE *file) {
    // Try loading NRO header into buffer
    std::fseek(file, 0, SEEK_SET);

    u8 header[NRO_HEADER_SIZE];
    if (std::fread(header, sizeof(u8), sizeof(header), file) < sizeof(header)) {
        PLOG_FATAL << "Failed to read NRO header";

        exit(0);
    }

    // Load segments
    Segment text, ro, data;
    u32 bssSize;

    std::memcpy(&text.raw, &header[HeaderOffset::TextSegment], HeaderFieldSize::SegmentHeader);
    std::memcpy(&ro.raw, &header[HeaderOffset::RoSegment], HeaderFieldSize::SegmentHeader);
    std::memcpy(&data.raw, &header[HeaderOffset::DataSegment], HeaderFieldSize::SegmentHeader);
    std::memcpy(&bssSize, &header[HeaderOffset::BssSize], HeaderFieldSize::BssSize);

    PLOG_DEBUG << ".text (offset = " << std::hex << text.offset << ", size = " << text.size << ")";
    PLOG_DEBUG << ".ro (offset = " << std::hex << ro.offset << ", size = " << ro.size << ")";
    PLOG_DEBUG << ".data (offset = " << std::hex << data.offset << ", size = " << data.size << ")";
    PLOG_DEBUG << ".bss (size = " << std::hex << bssSize << ")";

    const u64 applicationBase = sys::memory::MemoryBase::Application;

    // Load .text
    if (!sys::memory::isAligned(text.offset) || !sys::memory::isAligned(text.size)) {
        PLOG_FATAL << ".text offset/size not aligned";

        exit(0);
    }

    void *textPointer = sys::memory::allocate(applicationBase, text.size / sys::memory::PAGE_SIZE, 0, 0, sys::memory::MemoryPermission::RX);

    if (textPointer == NULL) {
        PLOG_FATAL << "Failed to allocate memory for .text";

        exit(0);
    }

    std::fseek(file, text.offset, SEEK_SET);

    if (std::fread(textPointer, sizeof(u8), text.size, file) < text.size) {
        PLOG_FATAL << "Failed to read .text segment from NRO";

        exit(0);
    }

    // Load .ro
    if (!sys::memory::isAligned(ro.offset) || !sys::memory::isAligned(ro.size)) {
        PLOG_FATAL << ".ro offset/size not aligned";

        exit(0);
    }

    void *roPointer = sys::memory::allocate(applicationBase + text.size, ro.size / sys::memory::PAGE_SIZE, 0, 0, sys::memory::MemoryPermission::R);

    if (roPointer == NULL) {
        PLOG_FATAL << "Failed to allocate memory for .ro";

        exit(0);
    }

    std::fseek(file, ro.offset, SEEK_SET);
    
    if (std::fread(roPointer, sizeof(u8), ro.size, file) < ro.size) {
        PLOG_FATAL << "Failed to read .ro segment from NRO";

        exit(0);
    }

    // Load .data/bss
    const u32 dataSize = data.size + bssSize;
    if (!sys::memory::isAligned(data.offset) || !sys::memory::isAligned(dataSize)) {
        PLOG_FATAL << ".data offset/size not aligned";

        exit(0);
    }

    void *dataPointer = sys::memory::allocate(applicationBase + text.size + ro.size, dataSize / sys::memory::PAGE_SIZE, 0, 0, sys::memory::MemoryPermission::RW);

    if (dataPointer == NULL) {
        PLOG_FATAL << "Failed to allocate memory for .data";

        exit(0);
    }

    std::fseek(file, data.offset, SEEK_SET);
    
    if (std::fread(dataPointer, sizeof(u8), data.size, file) < data.size) {
        PLOG_FATAL << "Failed to read .data segment from NRO";

        exit(0);
    }

    // Clear .bss
    std::memset(&((u8 *)dataPointer)[data.size], 0, bssSize);

    // Check the three extra segments
    Segment apiInfo, dynstr, dynsym;

    std::memcpy(&apiInfo.raw, &header[HeaderOffset::ApiInfoSegment], HeaderFieldSize::SegmentHeader);
    std::memcpy(&dynstr.raw, &header[HeaderOffset::DynstrSegment], HeaderFieldSize::SegmentHeader);
    std::memcpy(&dynsym.raw, &header[HeaderOffset::DynsymSegment], HeaderFieldSize::SegmentHeader);

    if ((apiInfo.size != 0) || (dynstr.size != 0) || (dynsym.size != 0)) {
        PLOG_WARNING << "Unimplemented extra segments";
    }

    // TODO: check for assets
}

void makeHomebrewEnv() {
    void *homebrewEnv = sys::memory::allocate(sys::memory::MemoryBase::HomebrewEnv, 1, 0, 0, sys::memory::MemoryPermission::R);
    std::memcpy(homebrewEnv, envContextTable, sizeof(envContextTable));
}

bool isNRO(FILE *file) {
    // Load magic from file, compare to NRO_MAGIC
    std::fseek(file, HeaderOffset::Magic, SEEK_SET);

    char magic[HeaderFieldSize::Magic];
    if (std::fread(magic, sizeof(char), sizeof(magic), file) < sizeof(magic)) {
        PLOG_FATAL << "Failed to read NRO header magic";

        exit(0);
    }

    return std::strncmp(magic, NRO_MAGIC, sizeof(magic)) == 0;
}

}
