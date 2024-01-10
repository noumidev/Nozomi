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

#pragma once

#include "ipc.hpp"
#include "types.hpp"

#include "nvfence.hpp"

namespace android::buffer_queue {

using hle::IPCContext;

using nvidia::NVMultiFence;

constexpr size_t MAX_BUFFER_QUEUES = 16; // Subject to change

constexpr size_t MAX_INTS = 81;

struct GraphicBuffer {
    u32 magic;
    u32 width, height, stride, format, usage;
    u32 pid;
    u32 refcount;
    u32 numFD, numInts;
    u32 ints[MAX_INTS]; // What is this?
};

enum class BufferQueueStatus {
    Unallocated,
    Dequeued,
    Queued,
};

class BufferQueue {
    BufferQueueStatus status;

    GraphicBuffer gbuf;

    NVMultiFence fence;

public:
    BufferQueue() : status(BufferQueueStatus::Unallocated) {}
    ~BufferQueue() {}

    BufferQueueStatus getStatus() {
        return status;
    }

    GraphicBuffer *getGraphicBuffer() {
        return &gbuf;
    }

    NVMultiFence *getFence() {
        return &fence;
    }

    void setStatus(BufferQueueStatus status) {
        this->status = status;
    }

    void setGraphicBuffer(const GraphicBuffer &gbuf) {
        this->gbuf = gbuf;
    }

    void setFence(const NVMultiFence &fence) {
        this->fence = fence;
    }
};

void transact(IPCContext &ctx, u32 code, u32 flags);

u32 findFreeBufferQueue();

}
