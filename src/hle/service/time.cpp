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

#include "time.hpp"

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::time {

namespace Command {
    enum : u32 {
        GetStandardUserSystemClock,
        GetStandardNetworkSystemClock,
        GetStandardSteadyClock,
        GetTimeZoneService,
        GetStandardLocalSystemClock,
        GetSharedMemoryNativeHandle = 20,
    };
}

Handle sharedMemory{.raw = 0ULL};

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::GetStandardUserSystemClock:
            return cmdGetStandardUserSystemClock(ctx, reply);
        case Command::GetStandardNetworkSystemClock:
            return cmdGetStandardNetworkSystemClock(ctx, reply);
        case Command::GetStandardSteadyClock:
            return cmdGetStandardSteadyClock(ctx, reply);
        case Command::GetTimeZoneService:
            return cmdGetTimeZoneService(ctx, reply);
        case Command::GetStandardLocalSystemClock:
            return cmdGetStandardLocalSystemClock(ctx, reply);
        case Command::GetSharedMemoryNativeHandle:
            return cmdGetSharedMemoryNativeHandle(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdGetSharedMemoryNativeHandle(IPCContext &ctx, IPCContext &reply) {
    constexpr u64 SHARED_MEMORY_SIZE = 0x1000;

    (void)ctx;

    PLOG_INFO << "GetSharedMemoryNativeHandle";

    if (sharedMemory.type == HandleType::None) {
        sharedMemory = kernel::makeSharedMemory(SHARED_MEMORY_SIZE);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(sharedMemory);
}

void cmdGetStandardLocalSystemClock(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetStandardLocalSystemClock";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<SystemClock>());
}

void cmdGetStandardNetworkSystemClock(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetStandardNetworkSystemClock";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<SystemClock>());
}

void cmdGetStandardSteadyClock(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetStandardSteadyClock";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<SteadyClock>());
}

void cmdGetStandardUserSystemClock(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetStandardUserSystemClock";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<SystemClock>());
}

void cmdGetTimeZoneService(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetTimeZoneService";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<TimeZoneService>());
}

SteadyClock::SteadyClock() {}

SteadyClock::~SteadyClock() {}

void SteadyClock::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

SystemClock::SystemClock() {}

SystemClock::~SystemClock() {}

void SystemClock::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

TimeZoneService::TimeZoneService() {}

TimeZoneService::~TimeZoneService() {}

void TimeZoneService::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

}
