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

#include "nvhost_ctrl_gpu.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "nvfile.hpp"

namespace nvidia::nvhost_ctrl_gpu {

namespace IOC {
    enum : u32 {
        ZcullGetCtxSize = 0x80044701,
        ZcullGetInfo = 0x80284702,
        GetTPCMasks = 0xc0184706,
        GetCharacteristics = 0xC0B04705,
    };
}

struct ZcullGetInfoParameters {
    u32 widthAlignPixels, heightAlignPixels;
    u32 pixelSquaresByAliquots, aliquotTotal;
    u32 regionByteMultiplier, regionHeaderSize;
    u32 subregionHeaderSize, subregionWidthAlignPixels, subregionHeightAlignPixels, subregionCount;
} __attribute__((packed));

struct GetTPCMasksParameters {
    u32 maskBufSize;
    u32 reserved[3];
    u64 maskBuf;
} __attribute__((packed));

struct GPUCharacteristics {
    u32 arch, impl, rev;
    u32 numGPC;
    u64 l2CacheSize, videoMemorySize;
    u32 numTPCPerGPC;
    u32 busType;
    u32 bigPageSize, compressionPageSize;
    u32 pdeCoverageBitCount;
    u32 availableBigPageSizes;
    u32 gpcMask;
    u32 smVersion, spaVersion, warpCount;
    u32 gpuVaBitCount;
    u32 reserved;
    u64 flags;
    u32 twodClass, threedClass, computeClass, gpfifoClass, inlineToMemoryClass, dmaCopyClass;
    u32 maxFBPSCount, fbpEnMask;
    u32 maxLTCPerFBP, maxLTSPerLTC, maxTexPerTPC;
    u32 maxGPCCount;
    u64 ropL2EnMask;
    u64 chipName;
    u64 gpCompBitStoreBaseHW;
} __attribute__((packed));

static_assert(sizeof(GPUCharacteristics) == 0xA0);

struct GetCharacteristicsParameters {
    u64 bufSize, bufAddr;
    GPUCharacteristics characteristics;
} __attribute__((packed));

void writeReply(void *data, size_t size, IPCContext &ctx) {
    std::vector<u8> reply;
    reply.resize(size);

    std::memcpy(reply.data(), data, size);

    ctx.writeReceive(reply);
}

i32 zcullGetCtxSize(IPCContext &ctx) {
    PLOG_VERBOSE << "ZCULL_GET_CTX_SIZE (stubbed)";

    // Yuzu returns 1
    u32 size = 1;

    writeReply(&size, sizeof(u32), ctx);

    return NVResult::Success;
}

i32 zcullGetInfo(IPCContext &ctx) {
    PLOG_VERBOSE << "ZCULL_GET_INFO (stubbed)";

    ZcullGetInfoParameters params;
    params.widthAlignPixels = params.heightAlignPixels = 0x20;
    params.pixelSquaresByAliquots = 0x400;
    params.aliquotTotal = 0x800;
    params.regionByteMultiplier = params.regionHeaderSize = 0x20;
    params.subregionHeaderSize = 0xC0;
    params.subregionWidthAlignPixels = 0x20;
    params.subregionHeightAlignPixels = 0x40;
    params.subregionCount = 0x10;

    writeReply(&params, sizeof(ZcullGetInfoParameters), ctx);

    return NVResult::Success;
}

i32 getTPCMasks(IPCContext &ctx) {
    GetTPCMasksParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(GetTPCMasksParameters));

    PLOG_VERBOSE << "GET_TPC_MASKS (stubbed)";

    params.maskBuf = 3;

    writeReply(&params, sizeof(GetTPCMasksParameters), ctx);

    return NVResult::Success;
}

i32 getCharacteristics(IPCContext &ctx) {
    GetCharacteristicsParameters params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(GetCharacteristicsParameters));

    PLOG_VERBOSE << "GET_CHARACTERISTICS";

    params.bufSize = sizeof(GPUCharacteristics);

    // Set GPU characteristics (large)
    GPUCharacteristics &characteristics = params.characteristics;

    // https://switchbrew.org/wiki/NV_services#NVGPU_GPU_IOCTL_GET_CHARACTERISTICS
    characteristics.arch = 0x120;
    characteristics.impl = 0xB;
    characteristics.rev = 0xA1;
    characteristics.numGPC = 1;
    characteristics.l2CacheSize = 0x40000;
    characteristics.videoMemorySize = 0;
    characteristics.numTPCPerGPC = 2;
    characteristics.busType = 0x20;
    characteristics.bigPageSize = characteristics.compressionPageSize = 0x20000;
    characteristics.pdeCoverageBitCount = 0x1B;
    characteristics.availableBigPageSizes = 0x30000;
    characteristics.gpcMask = 1;
    characteristics.smVersion = characteristics.spaVersion = 0x503;
    characteristics.warpCount = 0x80;
    characteristics.gpuVaBitCount = 0x28;
    characteristics.reserved = 0;
    characteristics.flags = 0x55;
    characteristics.twodClass = 0x902D;
    characteristics.threedClass = 0xB197;
    characteristics.computeClass = 0xB1C0;
    characteristics.gpfifoClass = 0xB06F;
    characteristics.inlineToMemoryClass = 0xA140;
    characteristics.dmaCopyClass = 0xB0B5;
    characteristics.maxFBPSCount = 1;
    characteristics.fbpEnMask = 0;
    characteristics.maxLTCPerFBP = 2;
    characteristics.maxLTSPerLTC = 1;
    characteristics.maxTexPerTPC = 0;
    characteristics.maxGPCCount = 1;
    characteristics.ropL2EnMask = 0x21D70;
    characteristics.chipName = 0x6230326D67;
    characteristics.gpCompBitStoreBaseHW = 0;

    writeReply(&params, sizeof(GetCharacteristicsParameters), ctx);

    return NVResult::Success;
}

i32 ioctl(u32 iocode, IPCContext &ctx) {
    switch (iocode) {
        case IOC::ZcullGetCtxSize:
            return zcullGetCtxSize(ctx);
        case IOC::ZcullGetInfo:
            return zcullGetInfo(ctx);
        case IOC::GetTPCMasks:
            return getTPCMasks(ctx);
        case IOC::GetCharacteristics:
            return getCharacteristics(ctx);
        default:
            PLOG_FATAL << "Unimplemented ioctl (iocode = " << std::hex << iocode << ")";

            exit(0);
    }
}

}
