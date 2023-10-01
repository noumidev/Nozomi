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

Result handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case Command::OpenApplicationProxy:
            return cmdOpenApplicationProxy(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result cmdOpenApplicationProxy(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "OpenApplicationProxy";

    const Handle handle = kernel::makeService<ApplicationProxy>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

ApplicationProxy::ApplicationProxy() {}

ApplicationProxy::~ApplicationProxy() {}

Result ApplicationProxy::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case ApplicationProxyCommand::GetCommonStateGetter:
            return cmdGetCommonStateGetter(data, reply);
        case ApplicationProxyCommand::GetSelfController:
            return cmdGetSelfController(data, reply);
        case ApplicationProxyCommand::GetWindowController:
            return cmdGetWindowController(data, reply);
        case ApplicationProxyCommand::GetAudioController:
            return cmdGetAudioController(data, reply);
        case ApplicationProxyCommand::GetDisplayController:
            return cmdGetDisplayController(data, reply);
        case ApplicationProxyCommand::GetLibraryAppletCreator:
            return cmdGetLibraryAppletCreator(data, reply);
        case ApplicationProxyCommand::GetApplicationFunctions:
            return cmdGetApplicationFunctions(data, reply);
        case ApplicationProxyCommand::GetDebugFunctions:
            return cmdGetDebugFunctions(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result ApplicationProxy::cmdGetApplicationFunctions(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetApplicationFunctions";

    const Handle handle = kernel::makeService<ApplicationFunctions>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetAudioController(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetAudioController";

    const Handle handle = kernel::makeService<AudioController>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetCommonStateGetter(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetCommonStateGetter";

    const Handle handle = kernel::makeService<CommonStateGetter>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetDebugFunctions(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetDebugFunctions";

    const Handle handle = kernel::makeService<DebugFunctions>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetDisplayController(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetDisplayController";

    const Handle handle = kernel::makeService<DisplayController>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetLibraryAppletCreator(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetLibraryAppletCreator";

    const Handle handle = kernel::makeService<LibraryAppletCreator>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetSelfController(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetSelfController";

    const Handle handle = kernel::makeService<SelfController>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

Result ApplicationProxy::cmdGetWindowController(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetWindowController";

    const Handle handle = kernel::makeService<WindowController>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

ApplicationFunctions::ApplicationFunctions() {}

ApplicationFunctions::~ApplicationFunctions() {}

Result ApplicationFunctions::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case ApplicationFunctionsCommand::NotifyRunning:
            return cmdNotifyRunning(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result ApplicationFunctions::cmdNotifyRunning(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "NotifyRunning (stubbed)";

    reply.write((u8)0); // Supposedly ignored by user processes

    return KernelResult::Success;
}

AudioController::AudioController() {}

AudioController::~AudioController() {}

Result AudioController::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    (void)data;
    (void)reply;

    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

CommonStateGetter::CommonStateGetter() : event(Handle{.raw = 0ULL}) {}

CommonStateGetter::~CommonStateGetter() {}

Result CommonStateGetter::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case CommonStateGetterCommand::GetEventHandle:
            return cmdGetEventHandle(data, reply);
        case CommonStateGetterCommand::GetOperationMode:
            return cmdGetOperationMode(data, reply);
        case CommonStateGetterCommand::GetPerformanceMode:
            return cmdGetPerformanceMode(data, reply);
        case CommonStateGetterCommand::GetCurrentFocusState:
            return cmdGetCurrentFocusState(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result CommonStateGetter::cmdGetCurrentFocusState(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetCurrentFocusState (stubbed)";

    reply.write(FocusState::InFocus);

    return KernelResult::Success;
}

Result CommonStateGetter::cmdGetEventHandle(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetEventHandle";

    if (event.type == HandleType::None) {
        event = kernel::makeEvent(true);
    }

    reply.write(event.raw);

    return KernelResult::Success;
}

Result CommonStateGetter::cmdGetOperationMode(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetOperationMode (stubbed)";

    reply.write(OperationMode::Handheld);

    return KernelResult::Success;
}

Result CommonStateGetter::cmdGetPerformanceMode(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetPerformanceMode (stubbed)";

    reply.write(PerformanceMode::Normal);

    return KernelResult::Success;
}

DebugFunctions::DebugFunctions() {}

DebugFunctions::~DebugFunctions() {}

Result DebugFunctions::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    (void)data;
    (void)reply;

    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

DisplayController::DisplayController() {}

DisplayController::~DisplayController() {}

Result DisplayController::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    (void)data;
    (void)reply;

    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

LibraryAppletCreator::LibraryAppletCreator() {}

LibraryAppletCreator::~LibraryAppletCreator() {}

Result LibraryAppletCreator::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    (void)data;
    (void)reply;

    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

SelfController::SelfController() : accumulatedSuspendedTickChangedEvent(Handle{.raw = 0ULL}) {}

SelfController::~SelfController() {}

Result SelfController::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case SelfControllerCommand::SetOperationModeChangedNotification:
            return cmdSetOperationModeChangedNotification(data, reply);
        case SelfControllerCommand::SetPerformanceModeChangedNotification:
            return cmdSetPerformanceModeChangedNotification(data, reply);
        case SelfControllerCommand::SetFocusHandlingMode:
            return cmdSetFocusHandlingMode(data, reply);
        case SelfControllerCommand::SetOutOfFocusSuspendingEnabled:
            return cmdSetOutOfFocusSuspendingEnabled(data, reply);
        case SelfControllerCommand::GetAccumulatedSuspendedTickChangedEvent:
            return cmdGetAccumulatedSuspendedTickChangedEvent(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result SelfController::cmdGetAccumulatedSuspendedTickChangedEvent(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetAccumulatedSuspendedTickChangedEvent";

    if (accumulatedSuspendedTickChangedEvent.type == HandleType::None) {
        accumulatedSuspendedTickChangedEvent = kernel::makeEvent(true);
    }

    reply.write(accumulatedSuspendedTickChangedEvent.raw);

    return KernelResult::Success;
}

Result SelfController::cmdSetFocusHandlingMode(u32 *data, IPCReply &reply) {
    (void)reply;

    u8 focusHandlingMode[3];
    for (u64 i = 0; i < sizeof(focusHandlingMode); i++) {
        focusHandlingMode[i] = ((u8 *)data)[i];
    }

    PLOG_INFO << "SetFocusHandlingMode ([0] = " << std::hex << (u32)focusHandlingMode[0] << ", [1] = " << (u32)focusHandlingMode[1] << ", [2] = " << (u32)focusHandlingMode[2] << ") (stubbed)";

    return KernelResult::Success;
}

Result SelfController::cmdSetOperationModeChangedNotification(u32 *data, IPCReply &reply) {
    (void)reply;

    const u8 operationModeChangedNotification = ((u8 *)data)[0];

    PLOG_INFO << "SetOperationModeChangedNotification (notification = " << std::hex << (u32)operationModeChangedNotification << ") (stubbed)";

    return KernelResult::Success;
}

Result SelfController::cmdSetOutOfFocusSuspendingEnabled(u32 *data, IPCReply &reply) {
    (void)reply;

    const u8 outOfFocusHandlingEnabled = ((u8 *)data)[0];

    PLOG_INFO << "SetOutOfFocusSuspendingEnabled (enabled = " << std::hex << (u32)outOfFocusHandlingEnabled << ") (stubbed)";

    return KernelResult::Success;
}

Result SelfController::cmdSetPerformanceModeChangedNotification(u32 *data, IPCReply &reply) {
    (void)reply;

    const u8 performanceModeChangedNotification = ((u8 *)data)[0];

    PLOG_INFO << "SetPerformanceModeChangedNotification (notification = " << std::hex << (u32)performanceModeChangedNotification << ") (stubbed)";

    return KernelResult::Success;
}

WindowController::WindowController() {}

WindowController::~WindowController() {}

Result WindowController::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case WindowControllerCommand::GetAppletResourceUserId:
            return cmdGetAppletResourceUserId(data, reply);
        case WindowControllerCommand::AcquireForegroundRights:
            return cmdAcquireForegroundRights(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result WindowController::cmdAcquireForegroundRights(u32 *data, IPCReply &reply) {
    (void)data;
    (void)reply;

    PLOG_INFO << "AcquireForegroundRights";

    return KernelResult::Success;
}

Result WindowController::cmdGetAppletResourceUserId(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "GetAppletResourceUserId (stubbed)";

    reply.write(0ULL); // ??

    return KernelResult::Success;
}

}
