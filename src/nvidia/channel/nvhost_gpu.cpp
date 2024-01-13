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

#include "nvhost_gpu.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "host1x.hpp"
#include "nvfence.hpp"
#include "nvfile.hpp"

namespace nvidia::channel::nvhost_gpu {

namespace IOC {
    enum : u32 {
        SetNvmapFD = 0x40044801,
        AllocObjCtx = 0xC0104809,
        ChannelZCULLBind = 0xC010480B,
        SetErrorNotifier = 0xC018480C,
        SetPriority = 0x4004480D,
        AllocGPFIFOEx = 0x40204818,
        AllocGPFIFOEx2 = 0xC020481A,
        SubmitGPFIFO2 = 0xC018481B,
    };
}

namespace Class {
    enum : u32 {
        TwoD = 0x902D,
        Kepler = 0xA140,
        GPFIFO = 0xB06F,
        DMA = 0xB0B5,
        ThreeD = 0xB197,
        Compute = 0xB1C0,
    };
}

struct AllocGPFIFOExParameters {
    u32 numEntries, numJobs;
    u32 flags;
    NVFence fence;
    u32 reserved[3];
} __attribute__((packed));

static_assert(sizeof(AllocGPFIFOExParameters) == 32);

struct AllocObjCtxParameters {
    u32 classNum;
    u32 flags;
    u64 objID;
} __attribute__((packed));

static_assert(sizeof(AllocObjCtxParameters) == 16);

struct ChannelZCULLBindParameters {
    u64 gpuVA;
    u32 mode;
    u32 reserved;
} __attribute__((packed));

static_assert(sizeof(ChannelZCULLBindParameters) == 16);

struct SetErrorNotifierParameters {
    u64 offset, size;
    u32 mem;
    u32 reserved;
} __attribute__((packed));

static_assert(sizeof(SetErrorNotifierParameters) == 24);

struct SubmitGPFIFOParameters {
    u64 gpfifo;
    u32 numEntries;
    u32 flags;
    NVFence fence;
    // GPFIFO entries handled seperately
} __attribute__((packed));

static_assert(sizeof(SubmitGPFIFOParameters) == 24);

NVFence allocFence, submitFence;

FileDescriptor nvmapFD = NO_FD;

const char *getClassName(u32 classNum) {
    switch (classNum) {
        case Class::TwoD:
            return "2D";
        case Class::Kepler:
            return "Kepler";
        case Class::GPFIFO:
            return "GPFIFO";
        case Class::DMA:
            return "DMA";
        case Class::ThreeD:
            return "3D";
        case Class::Compute:
            return "Compute";
        default:
            PLOG_FATAL << "Invalid class number";

            exit(0);
    }
}

void writeReply(void *data, size_t size, IPCContext &ctx) {
    std::vector<u8> reply;
    reply.resize(size);

    std::memcpy(reply.data(), data, size);

    ctx.writeReceive(reply);
}

i32 setNvmapFD(IPCContext &ctx) {
    if (nvmapFD != NO_FD) {
        PLOG_FATAL << "nvmap object is already bound to this channel";

        exit(0);
    }

    std::memcpy(&nvmapFD, ctx.readSend().data(), sizeof(FileDescriptor));

    PLOG_VERBOSE << "SET_NVMAP_FD (FD = " << nvmapFD << ")";

    return NVResult::Success;
}

i32 allocObjCtx(IPCContext &ctx) {
    AllocObjCtxParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(AllocObjCtxParameters));

    PLOG_VERBOSE << "ALLOC_OBJ_CTX (class = " << getClassName(params.classNum) << ", flags = " << params.flags << ") (stubbed)";

    writeReply(&params, sizeof(AllocObjCtxParameters), ctx);

    return NVResult::Success;
}

i32 channelZCULLBind(IPCContext &ctx) {
    ChannelZCULLBindParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(ChannelZCULLBindParameters));

    PLOG_VERBOSE << "CHANNEL_ZCULL_BIND (VA = " << std::hex << params.gpuVA << ", mode = " << std::dec << params.mode << ") (stubbed)";

    writeReply(&params, sizeof(AllocObjCtxParameters), ctx);

    return NVResult::Success;
}

// Doesn't seem to do anything on the Switch?
i32 setErrorNotifier(IPCContext &ctx) {
    PLOG_VERBOSE << "SET_ERROR_NOTIFIER";

    writeReply(ctx.readSend().data(), sizeof(SetErrorNotifierParameters), ctx);

    return NVResult::Success;
}

i32 setPriority(IPCContext &ctx) {
    u32 priority;
    std::memcpy(&priority, ctx.readSend().data(), sizeof(u32));

    PLOG_VERBOSE << "SET_PRIORITY (priority = " << std::hex << priority << ")";

    return NVResult::Success;
}

i32 allocGPFIFOEx(IPCContext &ctx) {
    AllocGPFIFOExParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(AllocGPFIFOExParameters));

    PLOG_VERBOSE << "ALLOC_GPFIFO_EX (entries = " << params.numEntries << ", jobs = " << params.numJobs << ", flags = " << params.flags << ") (stubbed)";

    params.fence = allocFence;

    // TODO: implement the PFIFO
    writeReply(&params, sizeof(AllocGPFIFOExParameters), ctx);

    return NVResult::Success;
}

i32 submitGPFIFO(IPCContext &ctx) {
    SubmitGPFIFOParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(SubmitGPFIFOParameters));

    PLOG_VERBOSE << "SUBMIT_GPFIFO (GPFIFO = " << std::hex << params.gpfifo << ", entries = " << std::dec << params.numEntries << ", flags = " << std::hex << params.flags << ")";

    std::vector<u8> entries = ctx.readSend(1);

    for (size_t i = 0; i < (entries.size() / sizeof(u64)); i++) {
        u64 entry;
        std::memcpy(&entry, &entries[sizeof(u64) * i], sizeof(u64));

        PLOG_VERBOSE << "GPFIFO entry " << i << " (IOVA = " << std::hex << (entry & 0xFFFFFFFFFFULL) << ", flags = " << (entry >> 40) << ")";
    }

    params.flags = 0;
    params.fence = submitFence;

    return NVResult::Success;
}

void init() {
    allocFence = host1x::makeFence();
    submitFence = host1x::makeFence();
}

i32 ioctl(u32 iocode, IPCContext &ctx) {
    switch (iocode) {
        case IOC::SetNvmapFD:
            return setNvmapFD(ctx);
        case IOC::AllocObjCtx:
            return allocObjCtx(ctx);
        case IOC::ChannelZCULLBind:
            return channelZCULLBind(ctx);
        case IOC::SetErrorNotifier:
            return setErrorNotifier(ctx);
        case IOC::SetPriority:
            return setPriority(ctx);
        case IOC::AllocGPFIFOEx:
        case IOC::AllocGPFIFOEx2:
            return allocGPFIFOEx(ctx);
        case IOC::SubmitGPFIFO2:
            return submitGPFIFO(ctx);
        default:
            PLOG_FATAL << "Unimplemented ioctl (iocode = " << std::hex << iocode << ")";

            exit(0);
    }
}

}
