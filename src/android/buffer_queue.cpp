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
        RequestBuffer = 1,
        DequeueBuffer = 3,
        QueueBuffer = 7,
        CancelBuffer,
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

namespace NativeWindowScalingMode {
    enum : u32 {
        Freeze,
        ScaleToWindow,
        ScaleCrop,
        NoScaleCrop,
        PreserveAspectRatio,
    };
}

namespace NativeWindowTransform {
    enum : u32 {
        None,
    };
}

std::array<BufferQueue, MAX_BUFFER_QUEUES> bufferQueues;

Status requestBuffer(Parcel &in, Parcel &out) {
    const u32 buf = in.read<u32>();

    PLOG_VERBOSE << "REQUEST_BUFFER (buffer = " << buf << ")";

    if ((buf < 0) || (buf > (u32)MAX_BUFFER_QUEUES)) {
        PLOG_FATAL << "Invalid buffer queue slot";

        exit(0);
    }

    const GraphicBuffer *gbuf = bufferQueues[buf].getGraphicBuffer();

    std::vector<u8> reply;
    reply.resize((10 + gbuf->numInts) * sizeof(u32));

    std::memcpy(reply.data(), gbuf, 10 * sizeof(u32));
    std::memcpy(&reply[10 * sizeof(u32)], gbuf->ints, gbuf->numInts * sizeof(u32));

    out.writeFlattenedObject(reply);

    return StatusCode::NoError;
}

Status dequeueBuffer(Parcel &in, Parcel &out) {
    const bool async = in.read<u32>() == 1;
    const u32 width = in.read<u32>();
    const u32 height = in.read<u32>();
    const u32 format = in.read<u32>();
    const u32 usage = in.read<u32>();

    PLOG_VERBOSE << "DEQUEUE_BUFFER (async = " << async << ", width = " << width << ", height = " << height << ", format = " << format << ", usage = " << std::hex << usage << ")";

    const u32 buf = findFreeBufferQueue();

    BufferQueue &bq = bufferQueues[buf];
    bq.setStatus(BufferQueueStatus::Dequeued);

    GraphicBuffer *gbuf = bq.getGraphicBuffer();
    if ((gbuf->width != width) || (gbuf->height != height) || (gbuf->format != format) || (gbuf->usage != usage)) {
        PLOG_ERROR << "GraphicBuffer configuration doesn't match incoming configuration";

        gbuf->width = width;
        gbuf->height = height;
        gbuf->format = format;
        gbuf->usage = usage;
    }

    out.write(buf);

    const NVMultiFence *fence = bq.getFence();

    std::vector<u8> reply;
    reply.resize(sizeof(NVMultiFence));

    std::memcpy(reply.data(), fence, sizeof(NVMultiFence));

    out.writeFlattenedObject(reply);

    return StatusCode::NoError;
}

Status queueBuffer(Parcel &in, Parcel &out) {
    (void)out;

    const u32 buf = in.read<u32>();

    if ((buf < 0) || (buf > (u32)MAX_BUFFER_QUEUES)) {
        PLOG_FATAL << "Invalid buffer queue slot";

        exit(0);
    }

    PLOG_VERBOSE << "QUEUE_BUFFER (buffer = " << buf << ")";
    
    BufferQueue &bq = bufferQueues[buf];
    //bq.setStatus(BufferQueueStatus::Queued);
    bq.setStatus(BufferQueueStatus::Unallocated);

    PLOG_WARNING << "HACK: BufferQueue is freed";

    const i64 timestamp = in.read<u64>();
    const bool isAutoTimestamp = in.read<u32>() == 1;

    i32 crop[4];
    for (int i = 0; i < 4; i++) {
        crop[i] = in.read<u32>();
    }
    
    const u32 scalingMode = in.read<u32>();
    const u32 transform = in.read<u32>();
    const u32 stickyTransform = in.read<u32>();
    const bool async = in.read<u32>() == 1;
    const i32 swapInterval = in.read<u32>();

    NVMultiFence *fence = bq.getFence();

    fence->numFences = in.read<u32>();
    if ((fence->numFences < 0) || (fence->numFences > 4)) {
        PLOG_FATAL << "Invalid number of fences (" << fence->numFences << ")";

        exit(0);
    }

    for (int i = 0; i < 4; i++) {
        fence->fences[i].id = in.read<u32>();
        fence->fences[i].value = in.read<u32>();
    }

    PLOG_VERBOSE << "Timestamp = " << timestamp << " (is auto = " << isAutoTimestamp << ")";
    PLOG_VERBOSE << "Crop (" << crop[0] << ", " << crop[1] << ", " << crop[2] << ", " << crop[3] << ")";
    PLOG_VERBOSE << "Scaling mode = " << scalingMode << ", transform = " << transform << " (sticky = " << stickyTransform << ")";
    PLOG_VERBOSE << "Async = " << async << ", swap interval = " << swapInterval;
    
    // TODO: write proper data to the output parcel
    out.write(1280);
    out.write(720);
    out.write(0);
    out.write(1);

    nvidia::nvflinger::render(bq.getGraphicBuffer()->ints[1]);

    return StatusCode::NoError;
}

Status cancelBuffer(Parcel &in, Parcel &out) {
    // Output parcel is unused
    (void)out;

    const i32 buf = in.read<u32>();

    PLOG_VERBOSE << "CANCEL_BUFFER (buffer = " << buf << ")";

    PLOG_VERBOSE << "Size = " << in.read<u32>() << ", FD count = " << in.read<u32>();

    BufferQueue &bq = bufferQueues[buf];
    bq.setStatus(BufferQueueStatus::Unallocated);

    NVMultiFence fence;
    fence.numFences = in.read<u32>();

    for (int i = 0; i < nvidia::MAX_FENCES; i++) {
        fence.fences[i].id = in.read<u32>();
        fence.fences[i].value = in.read<u32>();
    }

    bq.setFence(fence);

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
            out.write(1280);
            out.write(720);
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

        std::printf("INT%d = %X\n", i, gbuf.ints[i]);
    }

    if ((buf < 0) || (buf > (int)MAX_BUFFER_QUEUES)) {
        PLOG_FATAL << "Invalid buffer queue slot";

        exit(0);
    }

    NVMultiFence fence;
    fence.numFences = 1;
    for (NVFence &f : fence.fences) {
        f.id = -1;
        f.value = 0;
    }

    BufferQueue &bq = bufferQueues[buf];
    bq.setGraphicBuffer(gbuf);
    bq.setFence(fence);

    return StatusCode::NoError;
}

// https://android.googlesource.com/platform/frameworks/native/+/29a3e90879fd96404c971e7187cd0e05927bbce0/libs/gui/IGraphicBufferProducer.cpp
void transact(IPCContext &ctx, u32 code, u32 flags) {
    Parcel in, out;
    in.deserialize(ctx.readSend());

    Status status;
    switch (code) {
        case Code::RequestBuffer:
            status = requestBuffer(in, out);
            break;
        case Code::DequeueBuffer:
            status = dequeueBuffer(in, out);
            break;
        case Code::QueueBuffer:
            status = queueBuffer(in, out);
            break;
        case Code::CancelBuffer:
            status = cancelBuffer(in, out);
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
