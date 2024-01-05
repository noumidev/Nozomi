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

#include "buffer_queue.hpp"

#include <array>
#include <cstdlib>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "parcel.hpp"

#include "nvflinger.hpp"

namespace android::buffer_queue {

using parcel::Parcel;

namespace Code {
    enum : u32 {
        DequeueBuffer = 3,
        Connect = 10,
        SetPreallocatedBuffer = 14,
    };
}

using Status = i32;

namespace StatusCode {
    enum : Status {
        NoError,
    };
}

namespace NativeWindowAPI {
    enum : u32 {
        CPU = 2,
    };
}

std::array<BufferQueue, MAX_BUFFER_QUEUES> bufferQueues;

Status dequeueBuffer(Parcel &in, Parcel &out) {
    const bool async = in.read<u32>() == 1;
    const u32 width = in.read<u32>();
    const u32 height = in.read<u32>();
    const u32 format = in.read<u32>();
    const u32 usage = in.read<u32>();

    PLOG_VERBOSE << "DEQUEUE_BUFFER (async = " << async << ", width = " << width << ", height = " << height << ", format = " << format << ", usage = " << std::hex << usage << ")";

    const u32 slot = findFreeBufferQueue();

    BufferQueue &bq = bufferQueues[slot];
    bq.setStatus(BufferQueueStatus::Dequeued);

    const GraphicBuffer *gbuf = bq.getGraphicBuffer();
    if ((gbuf->width != width) || (gbuf->height != height) || (gbuf->format != format) || (gbuf->usage != usage)) {
        PLOG_FATAL << "GraphicBuffer configuration doesn't match incoming configuration";

        exit(0);
    }

    out.write(slot);

    if (bq.getFence()->numFences > 0) {
        std::vector<u8> reply;
        reply.resize(sizeof(NVMultiFence));

        std::memcpy(reply.data(), bq.getFence(), sizeof(NVMultiFence));

        out.writeFlattenedObject(reply);
    }

    return StatusCode::NoError;
}

Status connect(Parcel &in, Parcel &out) {
    const bool enableListener = in.read<u32>() == 1;
    const u32 api = in.read<u32>();
    const bool producerControlledByApp = in.read<u32>() == 1;

    PLOG_VERBOSE << "CONNECT (Enable listener = " << enableListener << ", API = " << api << ", producer controlled by app = " << producerControlledByApp << ")";

    if (enableListener) {
        PLOG_ERROR << "Unimplemented listener";

        exit(0);
    }

    switch (api) {
        // TODO: write proper data to the output parcel
        case NativeWindowAPI::CPU:
            out.write(0);
            out.write(0);
            out.write(0);
            out.write(0);
            break;
        default:
            PLOG_ERROR << "Unimplemented native window API " << api;

            exit(0);
    }
    
    return StatusCode::NoError;
}

Status setPreallocatedBuffer(Parcel &in, Parcel &out) {
    // Output parcel is unused
    (void)out;

    const i32 buf = in.read<u32>();
    const bool hasInput = in.read<u32>() == 1;

    PLOG_VERBOSE << "SET_PREALLOCATED_BUFFER (buffer = " << buf << ", has input = " << hasInput << ")";

    if (!hasInput) {
        PLOG_FATAL << "SET_PREALLOCATED_BUFFER without input";

        exit(0);
    }

    PLOG_VERBOSE << "Size = " << in.read<u32>() << ", FD count = " << in.read<u32>();

    GraphicBuffer gbuf;
    gbuf.magic = in.read<u32>();
    gbuf.width = in.read<u32>();
    gbuf.height = in.read<u32>();
    gbuf.stride = in.read<u32>();
    gbuf.format = in.read<u32>();
    gbuf.usage = in.read<u32>();
    gbuf.pid = in.read<u32>();
    gbuf.refcount = in.read<u32>();
    gbuf.numFD = in.read<u32>();
    gbuf.numInts = in.read<u32>();

    PLOG_VERBOSE << "GraphicBuffer magic = " << std::hex << gbuf.magic;
    PLOG_VERBOSE << "GraphicBuffer width = " << gbuf.width << ", height = " << gbuf.height << ", stride = " << gbuf.stride << ", format = " << gbuf.format << ", usage = " << std::hex << gbuf.usage;
    PLOG_VERBOSE << "GraphicBuffer PID = " << gbuf.pid;
    PLOG_VERBOSE << "GraphicBuffer refcount = " << gbuf.refcount;
    PLOG_VERBOSE << "GraphicBuffer num FDs = " << gbuf.numFD << ", num ints = " << gbuf.numInts;

    if (gbuf.numInts > MAX_INTS) {
        PLOG_FATAL << "Too many ints";

        exit(0);
    }

    // Still don't know what this is
    for (u32 i = 0; i < gbuf.numInts; i++) {
        gbuf.ints[i] = in.read<u32>();
    }

    if ((buf < 0) || (buf > (int)MAX_BUFFER_QUEUES)) {
        PLOG_FATAL << "Invalid buffer queue slot";

        exit(0);
    }

    bufferQueues[buf].setGraphicBuffer(gbuf);

    return StatusCode::NoError;
}

// https://android.googlesource.com/platform/frameworks/native/+/29a3e90879fd96404c971e7187cd0e05927bbce0/libs/gui/IGraphicBufferProducer.cpp
void transact(IPCContext &ctx, u32 code, u32 flags) {
    Parcel in, out;
    in.deserialize(ctx.readSend());

    Status status;
    switch (code) {
        case Code::DequeueBuffer:
            status = dequeueBuffer(in, out);
            break;
        case Code::Connect:
            status = connect(in, out);
            break;
        case Code::SetPreallocatedBuffer:
            status = setPreallocatedBuffer(in, out);
            break;
        default:
            PLOG_FATAL << "Unimplemented transaction (code = " << code << ", flags = " << std::hex << flags << ")";

            exit(0);
    }

    out.write(status);
    ctx.writeReceive(out.serialize());
}

u32 findFreeBufferQueue() {
    for (size_t i = 0; i < MAX_BUFFER_QUEUES; i++) {
        if (bufferQueues[i].getStatus() == BufferQueueStatus::Unallocated) {
            return i;
        }
    }

    PLOG_FATAL << "Unable to find free buffer queue";

    exit(0);
}

}
