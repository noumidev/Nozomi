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

#include "hid.hpp"

#include <ios>

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::hid {

namespace Command {
    enum : u32 {
        CreateAppletResource,
    };
}

namespace AppletResourceCommand {
    enum : u32 {
        GetSharedMemoryHandle,
    };
}

Handle appletResource{.raw = 0ULL};

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::CreateAppletResource:
            return cmdCreateAppletResource(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdCreateAppletResource(IPCContext &ctx, IPCContext &reply) {
    const u32 *data = (u32 *)ctx.getData();

    const u32 pid = data[0];
    const u64 appletResourceUserID = (u64)data[0] | ((u64)data[1] << 32);

    PLOG_INFO << "CreateAppletResource (PID = " << std::hex << pid << ", applet resource user ID = " << appletResourceUserID << ")";

    if (appletResource.type == HandleType::None) {
        appletResource = kernel::makeService<AppletResource>();
    }

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(appletResource);
}

AppletResource::AppletResource() {}

AppletResource::~AppletResource() {}

void AppletResource::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case AppletResourceCommand::GetSharedMemoryHandle:
            return cmdGetSharedMemoryHandle(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void AppletResource::cmdGetSharedMemoryHandle(IPCContext &ctx, IPCContext &reply) {
    constexpr u64 SHARED_MEMORY_SIZE = 0x40000;

    (void)ctx;

    PLOG_INFO << "GetSharedMemoryHandle";

    if (sharedMemory.type == HandleType::None) {
        sharedMemory = kernel::makeSharedMemory(SHARED_MEMORY_SIZE);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(sharedMemory);
}

}
