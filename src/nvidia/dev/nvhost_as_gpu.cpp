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

#include "nvhost_as_gpu.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "nvfile.hpp"

namespace nvidia::dev::nvhost_as_gpu {

namespace IOC {
    enum : u32 {
        BindChannel = 0x40044101,
        AllocASEx = 0x40284109,
        MapBufferEx = 0xC0284106,
    };
}

struct AllocASExParameters {
    u32 bigPageSize;
    i32 asFD;
    u32 flags, reserved;
    u64 vaRangeStart, vaRangeEnd, vaRangeSplit;
} __attribute__((packed));

static_assert(sizeof(AllocASExParameters) == 40);

struct MapBufferExParameters {
    u32 flags;
    u32 kind;
    u32 memID;
    u32 reserved;
    u64 bufferOffset, mappingSize, align;
} __attribute__((packed));

static_assert(sizeof(MapBufferExParameters) == 40);

FileDescriptor channelFD = NO_FD;

void writeReply(void *data, size_t size, IPCContext &ctx) {
    std::vector<u8> reply;
    reply.resize(size);

    std::memcpy(reply.data(), data, size);

    ctx.writeReceive(reply);
}

i32 bindChannel(IPCContext &ctx) {
    if (channelFD != NO_FD) {
        PLOG_FATAL << "GPU channel already bound to address space";

        exit(0);
    }

    std::memcpy(&channelFD, ctx.readSend().data(), sizeof(FileDescriptor));

    PLOG_VERBOSE << "BIND_CHANNEL (channel FD = " << channelFD << ")";

    return NVResult::Success;
}

i32 allocASEx(IPCContext &ctx) {
    AllocASExParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(AllocASExParameters));

    PLOG_VERBOSE << "ALLOC_AS_EX (big page size = " << params.bigPageSize << ", VA range (start = " << std::hex << params.vaRangeStart << ", end = " << params.vaRangeEnd << ", split = " << params.vaRangeSplit << "))";

    return NVResult::Success;
}

i32 mapBufferEx(IPCContext &ctx) {
    MapBufferExParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(MapBufferExParameters));

    PLOG_VERBOSE << "MAP_BUFFER_EX (flags = " << std::hex << params.flags << ", kind = " << params.kind << ", mem ID = " << std::dec << params.memID << ", buffer offset = " << std::hex << params.bufferOffset << ", mapping size = " << params.mappingSize << ", align = " << params.align << ") (stubbed)";

    // TODO: figure out what to do with this
    params.align = 0;

    writeReply(&params, sizeof(MapBufferExParameters), ctx);

    return NVResult::Success;
}

i32 ioctl(u32 iocode, IPCContext &ctx) {
    switch (iocode) {
        case IOC::BindChannel:
            return bindChannel(ctx);
        case IOC::AllocASEx:
            return allocASEx(ctx);
        case IOC::MapBufferEx:
            return mapBufferEx(ctx);
        default:
            PLOG_FATAL << "Unimplemented ioctl (iocode = " << std::hex << iocode << ")";

            exit(0);
    }
}

}
