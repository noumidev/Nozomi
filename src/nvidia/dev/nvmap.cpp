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

#include "nvmap.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "memory.hpp"
#include "nvfile.hpp"

namespace nvidia::dev::nvmap {

constexpr u32 HANDLE_OFFSET = 128;

namespace IOC {
    enum : u32 {
        Create = 0xC0080101,
        Alloc = 0xC0200104,
        GetID = 0xC008010E,
    };
}

struct CreateParameters {
    u32 size, handle;
} __attribute__((packed));

static_assert(sizeof(CreateParameters) == 8);

struct AllocParameters {
    u32 handle, heapmask, flags, align;
    u8 kind, pad[7];
    u64 addr;
} __attribute__((packed));

static_assert(sizeof(AllocParameters) == 32);

struct GetIDParameters {
    u32 id, handle;
} __attribute__((packed));

static_assert(sizeof(GetIDParameters) == 8);

struct NVMAP {
    u64 address, size;
};

std::vector<NVMAP> nvmapObjects;

void writeReply(void *data, size_t size, IPCContext &ctx) {
    std::vector<u8> reply;
    reply.resize(size);

    std::memcpy(reply.data(), data, size);

    ctx.writeReceive(reply);
}

i32 create(IPCContext &ctx) {
    CreateParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(CreateParameters));

    PLOG_VERBOSE << "CREATE (size = " << std::hex << params.size << ")";

    if (!sys::memory::isAligned(params.size)) {
        PLOG_ERROR << "Size is not aligned";

        exit(0);
    }

    nvmapObjects.emplace_back(NVMAP{.size = params.size});

    params.handle = (u32)nvmapObjects.size() - 1 + HANDLE_OFFSET;

    writeReply((void *)&params, sizeof(CreateParameters), ctx);

    return NVResult::Success;
}

i32 alloc(IPCContext &ctx) {
    AllocParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(AllocParameters));

    PLOG_VERBOSE << "ALLOC (handle = " << params.handle << ", heapmask = " << std::hex << params.heapmask << ", flags = " << params.flags << ", align = " << params.align << ", kind = " << std::dec << (u32)params.kind << ", address = " << std::hex << params.addr << ") (stubbed)";

    if (!sys::memory::isAligned(params.align)) {
        PLOG_ERROR << "Alignment is not aligned";

        exit(0);
    }

    if (!sys::memory::isAligned(params.addr)) {
        PLOG_ERROR << "Address is not aligned";

        exit(0);
    }

    nvmapObjects[params.handle - HANDLE_OFFSET].address = params.addr;

    writeReply((void *)&params, sizeof(AllocParameters), ctx);

    return NVResult::Success;
}

i32 getID(IPCContext &ctx) {
    GetIDParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(GetIDParameters));

    PLOG_VERBOSE << "GET_ID (handle = " << params.handle << ")";

    // TODO: figure out if this is fine
    params.id = params.handle - HANDLE_OFFSET;

    writeReply((void *)&params, sizeof(GetIDParameters), ctx);

    return NVResult::Success;
}

i32 ioctl(u32 iocode, IPCContext &ctx) {
    switch (iocode) {
        case IOC::Create:
            return create(ctx);
        case IOC::Alloc:
            return alloc(ctx);
        case IOC::GetID:
            return getID(ctx);
        default:
            PLOG_FATAL << "Unimplemented ioctl (iocode = " << std::hex << iocode << ")";

            exit(0);
    }
}

u64 getAddressFromID(u32 nvmapID, bool isHandle) {
    if (isHandle) {
        nvmapID -= HANDLE_OFFSET;
    }

    if (nvmapID > nvmapObjects.size()) {
        PLOG_FATAL << "Invalid nvmap ID";

        exit(0);
    }

    return nvmapObjects[nvmapID].address;
}

u64 getSizeFromID(u32 nvmapID, bool isHandle) {
    if (isHandle) {
        nvmapID -= HANDLE_OFFSET;
    }

    if (nvmapID > nvmapObjects.size()) {
        PLOG_FATAL << "Invalid nvmap ID";

        exit(0);
    }

    return nvmapObjects[nvmapID].address;
}

}
