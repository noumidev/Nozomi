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

#include "nvhost_ctrl.hpp"

#include <array>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "memory.hpp"
#include "nvfence.hpp"
#include "nvfile.hpp"

namespace nvidia::nvhost_ctrl {

namespace IOC {
    enum : u32 {
        SyncptWaitEventEx = 0xC010001E,
        SyncptAllocEvent = 0xC004001F,
        SyncptFreeEvent = 0xC0040020,
    };
}

constexpr u32 NO_SYNCPT = -1;

constexpr u32 MAX_EVENTS = 0x40;

struct SyncpointEvent {
    u32 syncptID;

    bool isAllocated;
};

struct SyncptWaitEventParams {
    NVFence fence;
    i32 timeout;
    u32 value;
} __attribute__((packed));

static_assert(sizeof(SyncptWaitEventParams) == (4 * sizeof(u32)));

std::array<SyncpointEvent, MAX_EVENTS> events;

void writeReply(void *data, size_t size, IPCContext &ctx) {
    std::vector<u8> reply;
    reply.resize(size);

    std::memcpy(reply.data(), data, size);

    ctx.writeReceive(reply);
}

i32 syncptWaitEventEx(IPCContext &ctx) {
    SyncptWaitEventParams params;
    std::memcpy(&params, ctx.readSend().data(), sizeof(SyncptWaitEventParams));

    PLOG_VERBOSE << "SYNCPT_WAIT_EVENT_EX (syncpt ID = " << params.fence.id << ", syncpt value = " << std::hex << params.fence.value << ", timeout = " << std::dec << params.timeout << ", event slot = " << params.value << ") (stubbed)";

    const u32 eventSlot = params.value;
    if (eventSlot >= MAX_EVENTS) {
        PLOG_FATAL << "Invalid event slot";

        exit(0);
    }

    SyncpointEvent &event = events[eventSlot];

    if (event.syncptID != NO_SYNCPT) {
        PLOG_WARNING << "Event already has a syncpoint associated with it";
    } else {
        event.syncptID = params.fence.id;
    }

    params.value = eventSlot | (event.syncptID << 4);

    writeReply(&params, sizeof(SyncptWaitEventParams), ctx);

    return NVResult::Timeout;
}

i32 syncptAllocEvent(IPCContext &ctx) {
    u32 eventSlot;
    std::memcpy(&eventSlot, ctx.readSend().data(), sizeof(u32));

    PLOG_VERBOSE << "SYNCPT_ALLOC_EVENT (event slot = " << eventSlot << ")";

    if (eventSlot >= MAX_EVENTS) {
        PLOG_FATAL << "Invalid event slot";

        exit(0);
    }

    SyncpointEvent &event = events[eventSlot];

    if (event.isAllocated) {
        PLOG_FATAL << "Event is already allocated";

        exit(0);
    }

    event.syncptID = NO_SYNCPT;
    event.isAllocated = true;

    return NVResult::Success;
}

i32 syncptFreeEvent(IPCContext &ctx) {
    u32 eventSlot;
    std::memcpy(&eventSlot, ctx.readSend().data(), sizeof(u32));

    PLOG_VERBOSE << "SYNCPT_FREE_EVENT (event slot = " << eventSlot << ")";

    if (eventSlot >= MAX_EVENTS) {
        PLOG_FATAL << "Invalid event slot";

        exit(0);
    }

    SyncpointEvent &event = events[eventSlot];

    if (!event.isAllocated) {
        PLOG_WARNING << "Event is deallocated";
    }

    event.isAllocated = false;

    return NVResult::Success;
}

i32 ioctl(u32 iocode, IPCContext &ctx) {
    switch (iocode) {
        case IOC::SyncptWaitEventEx:
            return syncptWaitEventEx(ctx);
        case IOC::SyncptAllocEvent:
            return syncptAllocEvent(ctx);
        case IOC::SyncptFreeEvent:
            return syncptFreeEvent(ctx);
        default:
            PLOG_FATAL << "Unimplemented ioctl (iocode = " << std::hex << iocode << ")";

            exit(0);
    }
}

}
