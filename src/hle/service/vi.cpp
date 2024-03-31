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

#include "vi.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>

#include <plog/Log.h>

#include "nvflinger.hpp"
#include "parcel.hpp"
#include "result.hpp"

namespace hle::service::vi {

using android::parcel::Parcel;

using namespace nvidia;
using nvflinger::DisplayName;
using nvflinger::HOSDriverBinder;
using nvflinger::NativeWindow;

namespace ManagerCommand {
    enum : u32 {
        GetDisplayService = 2,
    };
}

namespace ApplicationDisplayServiceCommand {
    enum : u32 {
        GetRelayService = 100,
        GetSystemDisplayService,
        GetManagerDisplayService,
        GetIndirectDisplayTransactionService,
        OpenDisplay = 1010,
        CloseDisplay = 1020,
        CloseLayer = 2021,
        SetLayerScalingMode = 2101,
    };
}

namespace ManagerDisplayServiceCommand {
    enum : u32 {
        CreateStrayLayer = 2012,
    };
}

namespace VIScalingMode {
    enum : u64 {
        None,
        FitToLayer = 2,
        PreserveAspectRatio = 4,
    };
}

const char *getScalingModeName(u64 scalingMode) {
    switch (scalingMode) {
        case VIScalingMode::None:
            return "None";
        case VIScalingMode::FitToLayer:
            return "Fit to Layer";
        case VIScalingMode::PreserveAspectRatio:
            return "Preserve Aspect Ratio";
        default:
            PLOG_FATAL << "Invalid scaling mode";

            exit(0);
    }
}

// vi:m
void managerHandleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case ManagerCommand::GetDisplayService:
            return cmdGetDisplayService(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdGetDisplayService(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetDisplayService";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<ApplicationDisplayService>());
}

ApplicationDisplayService::ApplicationDisplayService() {}

ApplicationDisplayService::~ApplicationDisplayService() {}

void ApplicationDisplayService::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case ApplicationDisplayServiceCommand::GetRelayService:
            return cmdGetRelayService(ctx, reply);
        case ApplicationDisplayServiceCommand::GetSystemDisplayService:
            return cmdGetSystemDisplayService(ctx, reply);
        case ApplicationDisplayServiceCommand::GetManagerDisplayService:
            return cmdGetManagerDisplayService(ctx, reply);
        case ApplicationDisplayServiceCommand::GetIndirectDisplayTransactionService:
            return cmdGetIndirectDisplayTransactionService(ctx, reply);
        case ApplicationDisplayServiceCommand::OpenDisplay:
            return cmdOpenDisplay(ctx, reply);
        case ApplicationDisplayServiceCommand::CloseDisplay:
            return cmdCloseDisplay(ctx, reply);
        case ApplicationDisplayServiceCommand::CloseLayer:
            return cmdCloseLayer(ctx, reply);
        case ApplicationDisplayServiceCommand::SetLayerScalingMode:
            return cmdSetLayerScalingMode(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void ApplicationDisplayService::cmdCloseDisplay(IPCContext &ctx, IPCContext &reply) {
    u64 displayID;
    std::memcpy(&displayID, ctx.getData(), sizeof(u64));

    PLOG_INFO << "CloseDisplay (display ID = " << displayID << ") (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void ApplicationDisplayService::cmdCloseLayer(IPCContext &ctx, IPCContext &reply) {
    u64 layerID;
    std::memcpy(&layerID, ctx.getData(), sizeof(u64));

    PLOG_INFO << "CloseLayer (layer ID = " << layerID << ") (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void ApplicationDisplayService::cmdGetIndirectDisplayTransactionService(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetIndirectDisplayTransactionService";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<HOSDriverBinder>());
}

void ApplicationDisplayService::cmdGetManagerDisplayService(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetManagerDisplayService";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<ManagerDisplayService>());
}

void ApplicationDisplayService::cmdGetRelayService(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetRelayService";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<HOSDriverBinder>());
}

void ApplicationDisplayService::cmdGetSystemDisplayService(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetSystemDisplayService";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<SystemDisplayService>());
}

void ApplicationDisplayService::cmdOpenDisplay(IPCContext &ctx, IPCContext &reply) {
    DisplayName name = nvflinger::makeDisplayName((char *)ctx.getData());

    PLOG_INFO << "OpenDisplay (name = " << name.data() << ")";

    reply.makeReply(4);
    reply.write(KernelResult::Success);
    reply.write(nvflinger::openDisplay(name));
}

void ApplicationDisplayService::cmdSetLayerScalingMode(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    u64 scalingMode, layerID;
    std::memcpy(&scalingMode, data, sizeof(u64));
    std::memcpy(&layerID, &data[8], sizeof(u64));

    PLOG_INFO << "SetLayerScalingMode (Scaling mode = " << getScalingModeName(scalingMode) << ", layer ID = " << layerID << ") (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

ManagerDisplayService::ManagerDisplayService() {}

ManagerDisplayService::~ManagerDisplayService() {}

void ManagerDisplayService::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case ManagerDisplayServiceCommand::CreateStrayLayer:
            return cmdCreateStrayLayer(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void ManagerDisplayService::cmdCreateStrayLayer(IPCContext &ctx, IPCContext &reply) {
    u8 *data = (u8 *)ctx.getData();

    u32 flags;
    u64 displayID;

    std::memcpy(&flags, data, sizeof(flags));
    std::memcpy(&displayID, &data[8], sizeof(displayID));

    PLOG_INFO << "CreateStrayLayer (flags = " << std::hex << flags << ", display ID = " << displayID << ")";

    /* Thanks to Yuzu for this */

    const u64 layerID = nvflinger::makeLayer(displayID);
    const u32 bufferQueueID = nvflinger::getBufferQueueID(displayID, layerID);

    Parcel parcel;

    parcel.writeObject(NativeWindow(bufferQueueID).serialize());

    const u64 size = ctx.writeReceive(parcel.serialize());

    reply.makeReply(6);
    reply.write(KernelResult::Success);
    reply.write(layerID);
    reply.write(size);
}

SystemDisplayService::SystemDisplayService() {}

SystemDisplayService::~SystemDisplayService() {}

void SystemDisplayService::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

}
