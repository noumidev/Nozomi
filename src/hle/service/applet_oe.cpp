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

#include "applet_oe.hpp"

#include <cstring>

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::applet_oe {

namespace Command {
    enum : u32 {
        OpenApplicationProxy = 0,
    };
}

namespace ApplicationFunctionsCommand {
    enum : u32 {
        NotifyRunning = 40,
    };
}

namespace ApplicationProxyCommand {
    enum : u32 {
        GetCommonStateGetter = 0,
        GetSelfController,
        GetWindowController,
        GetAudioController,
        GetDisplayController,
        GetLibraryAppletCreator = 11,
        GetApplicationFunctions = 20,
        GetDebugFunctions = 1000,
    };
}

namespace CommonStateGetterCommand {
    enum : u32 {
        GetEventHandle = 0,
        ReceiveMessage,
        GetOperationMode = 5,
        GetPerformanceMode,
        GetCurrentFocusState = 9,
    };
}

namespace SelfControllerCommand {
    enum : u32 {
        SetOperationModeChangedNotification = 11,
        SetPerformanceModeChangedNotification,
        SetFocusHandlingMode,
        SetOutOfFocusSuspendingEnabled = 16,
        GetAccumulatedSuspendedTickChangedEvent = 91,
    };
}

namespace WindowControllerCommand {
    enum : u32 {
        GetAppletResourceUserId = 1,
        AcquireForegroundRights = 10,
    };
}

// For CommonStateGetter
namespace AppletMessage {
    enum : u32 {
        None,
    };
}

// For CommonStateGetter
namespace FocusState {
    enum : u8 {
        InFocus = 1,
        OutOfFocus,
        Background,
    };
}

// For CommonStateGetter
namespace OperationMode {
    enum : u8 {
        Handheld,
        Console,
    };
}

// For CommonStateGetter
namespace PerformanceMode {
    enum : int {
        Invalid = -1,
        Normal,
        Boost,
    };
}

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::OpenApplicationProxy:
            return cmdOpenApplicationProxy(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdOpenApplicationProxy(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "OpenApplicationProxy";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<ApplicationProxy>());
}

ApplicationProxy::ApplicationProxy() {}

ApplicationProxy::~ApplicationProxy() {}

void ApplicationProxy::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case ApplicationProxyCommand::GetCommonStateGetter:
            return cmdGetCommonStateGetter(ctx, reply);
        case ApplicationProxyCommand::GetSelfController:
            return cmdGetSelfController(ctx, reply);
        case ApplicationProxyCommand::GetWindowController:
            return cmdGetWindowController(ctx, reply);
        case ApplicationProxyCommand::GetAudioController:
            return cmdGetAudioController(ctx, reply);
        case ApplicationProxyCommand::GetDisplayController:
            return cmdGetDisplayController(ctx, reply);
        case ApplicationProxyCommand::GetLibraryAppletCreator:
            return cmdGetLibraryAppletCreator(ctx, reply);
        case ApplicationProxyCommand::GetApplicationFunctions:
            return cmdGetApplicationFunctions(ctx, reply);
        case ApplicationProxyCommand::GetDebugFunctions:
            return cmdGetDebugFunctions(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void ApplicationProxy::cmdGetApplicationFunctions(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetApplicationFunctions";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<ApplicationFunctions>());
}

void ApplicationProxy::cmdGetAudioController(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetAudioController";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<AudioController>());
}

void ApplicationProxy::cmdGetCommonStateGetter(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetCommonStateGetter";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<CommonStateGetter>());
}

void ApplicationProxy::cmdGetDebugFunctions(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetDebugFunctions";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<DebugFunctions>());
}

void ApplicationProxy::cmdGetDisplayController(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetDisplayController";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<DisplayController>());
}

void ApplicationProxy::cmdGetLibraryAppletCreator(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetLibraryAppletCreator";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<LibraryAppletCreator>());
}

void ApplicationProxy::cmdGetSelfController(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetSelfController";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<SelfController>());
}

void ApplicationProxy::cmdGetWindowController(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetWindowController";

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(kernel::makeService<WindowController>());
}

ApplicationFunctions::ApplicationFunctions() {}

ApplicationFunctions::~ApplicationFunctions() {}

void ApplicationFunctions::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case ApplicationFunctionsCommand::NotifyRunning:
            return cmdNotifyRunning(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void ApplicationFunctions::cmdNotifyRunning(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "NotifyRunning (stubbed)";

    reply.makeReply(3);
    reply.write(KernelResult::Success);
    reply.write((u8)0); // Supposedly ignored by user processes
}

AudioController::AudioController() {}

AudioController::~AudioController() {}

void AudioController::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

CommonStateGetter::CommonStateGetter() : event(Handle{.raw = 0ULL}) {}

CommonStateGetter::~CommonStateGetter() {}

void CommonStateGetter::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case CommonStateGetterCommand::GetEventHandle:
            return cmdGetEventHandle(ctx, reply);
        case CommonStateGetterCommand::ReceiveMessage:
            return cmdReceiveMessage(ctx, reply);
        case CommonStateGetterCommand::GetOperationMode:
            return cmdGetOperationMode(ctx, reply);
        case CommonStateGetterCommand::GetPerformanceMode:
            return cmdGetPerformanceMode(ctx, reply);
        case CommonStateGetterCommand::GetCurrentFocusState:
            return cmdGetCurrentFocusState(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void CommonStateGetter::cmdGetCurrentFocusState(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetCurrentFocusState (stubbed)";

    reply.makeReply(3);
    reply.write(KernelResult::Success);
    reply.write(FocusState::InFocus);
}

void CommonStateGetter::cmdGetEventHandle(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetEventHandle";

    if (event.type == HandleType::None) {
        event = kernel::makeEvent(true);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(event);
}

void CommonStateGetter::cmdGetOperationMode(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetOperationMode (stubbed)";

    reply.makeReply(3);
    reply.write(KernelResult::Success);
    reply.write(OperationMode::Handheld);
}

void CommonStateGetter::cmdGetPerformanceMode(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetPerformanceMode (stubbed)";

    reply.makeReply(3);
    reply.write(KernelResult::Success);
    reply.write(PerformanceMode::Normal);
}

void CommonStateGetter::cmdReceiveMessage(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "ReceiveMessage (stubbed)";

    reply.makeReply(3);
    reply.write(KernelResult::NoAppletMessages);
    reply.write(AppletMessage::None);
}

DebugFunctions::DebugFunctions() {}

DebugFunctions::~DebugFunctions() {}

void DebugFunctions::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

DisplayController::DisplayController() {}

DisplayController::~DisplayController() {}

void DisplayController::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

LibraryAppletCreator::LibraryAppletCreator() {}

LibraryAppletCreator::~LibraryAppletCreator() {}

void LibraryAppletCreator::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

SelfController::SelfController() : accumulatedSuspendedTickChangedEvent(Handle{.raw = 0ULL}) {}

SelfController::~SelfController() {}

void SelfController::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case SelfControllerCommand::SetOperationModeChangedNotification:
            return cmdSetOperationModeChangedNotification(ctx, reply);
        case SelfControllerCommand::SetPerformanceModeChangedNotification:
            return cmdSetPerformanceModeChangedNotification(ctx, reply);
        case SelfControllerCommand::SetFocusHandlingMode:
            return cmdSetFocusHandlingMode(ctx, reply);
        case SelfControllerCommand::SetOutOfFocusSuspendingEnabled:
            return cmdSetOutOfFocusSuspendingEnabled(ctx, reply);
        case SelfControllerCommand::GetAccumulatedSuspendedTickChangedEvent:
            return cmdGetAccumulatedSuspendedTickChangedEvent(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void SelfController::cmdGetAccumulatedSuspendedTickChangedEvent(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetAccumulatedSuspendedTickChangedEvent";

    if (accumulatedSuspendedTickChangedEvent.type == HandleType::None) {
        accumulatedSuspendedTickChangedEvent = kernel::makeEvent(true);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(accumulatedSuspendedTickChangedEvent);
}

void SelfController::cmdSetFocusHandlingMode(IPCContext &ctx, IPCContext &reply) {
    void *data = ctx.getData();

    u8 focusHandlingMode[3];
    for (u64 i = 0; i < sizeof(focusHandlingMode); i++) {
        focusHandlingMode[i] = ((u8 *)data)[i];
    }

    PLOG_INFO << "SetFocusHandlingMode ([0] = " << std::hex << (u32)focusHandlingMode[0] << ", [1] = " << (u32)focusHandlingMode[1] << ", [2] = " << (u32)focusHandlingMode[2] << ") (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void SelfController::cmdSetOperationModeChangedNotification(IPCContext &ctx, IPCContext &reply) {
    const u8 operationModeChangedNotification = ((u8 *)ctx.getData())[0];

    PLOG_INFO << "SetOperationModeChangedNotification (notification = " << std::hex << (u32)operationModeChangedNotification << ") (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void SelfController::cmdSetOutOfFocusSuspendingEnabled(IPCContext &ctx, IPCContext &reply) {
    const u8 outOfFocusHandlingEnabled = ((u8 *)ctx.getData())[0];

    PLOG_INFO << "SetOutOfFocusSuspendingEnabled (enabled = " << std::hex << (u32)outOfFocusHandlingEnabled << ") (stubbed)";
    
    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void SelfController::cmdSetPerformanceModeChangedNotification(IPCContext &ctx, IPCContext &reply) {
    const u8 performanceModeChangedNotification = ((u8 *)ctx.getData())[0];

    PLOG_INFO << "SetPerformanceModeChangedNotification (notification = " << std::hex << (u32)performanceModeChangedNotification << ") (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

WindowController::WindowController() {}

WindowController::~WindowController() {}

void WindowController::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case WindowControllerCommand::GetAppletResourceUserId:
            return cmdGetAppletResourceUserId(ctx, reply);
        case WindowControllerCommand::AcquireForegroundRights:
            return cmdAcquireForegroundRights(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void WindowController::cmdAcquireForegroundRights(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "AcquireForegroundRights";
    
    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void WindowController::cmdGetAppletResourceUserId(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetAppletResourceUserId (stubbed)";
    
    reply.makeReply(4);
    reply.write(KernelResult::Success);
    reply.write(0ULL); // TODO: return proper ID?
}

}
