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

#include <cstdlib>
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

namespace LibraryAppletAccessorCommand {
    enum : u32 {
        GetAppletStateChangedEvent = 0,
        Start = 10,
        GetResult = 30,
        PushInData = 100,
        PopOutData,
    };
}

namespace LibraryAppletCreatorCommand {
    enum : u32 {
        CreateLibraryApplet = 0,
        CreateStorage = 10,
    };
}

namespace SelfControllerCommand {
    enum : u32 {
        Exit = 0,
        GetLibraryAppletLaunchableEvent = 9,
        SetOperationModeChangedNotification = 11,
        SetPerformanceModeChangedNotification,
        SetFocusHandlingMode,
        SetOutOfFocusSuspendingEnabled = 16,
        GetAccumulatedSuspendedTickChangedEvent = 91,
    };
}

namespace StorageCommand {
    enum : u32 {
        Open = 0,
    };
}

namespace StorageAccessorCommand {
    enum : u32 {
        Write = 10,
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

LibraryAppletAccessor::LibraryAppletAccessor() {}

LibraryAppletAccessor::~LibraryAppletAccessor() {}

void LibraryAppletAccessor::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case LibraryAppletAccessorCommand::GetAppletStateChangedEvent:
            return cmdGetAppletStateChangedEvent(ctx, reply);
        case LibraryAppletAccessorCommand::Start:
            return cmdStart(ctx, reply);
        case LibraryAppletAccessorCommand::GetResult:
            return cmdGetResult(ctx, reply);
        case LibraryAppletAccessorCommand::PushInData:
            return cmdPushInData(ctx, reply);
        case LibraryAppletAccessorCommand::PopOutData:
            return cmdPopOutData(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void LibraryAppletAccessor::makeApplet(u32 appletID) {
    applet = applet::makeApplet(appletID);
}

void LibraryAppletAccessor::cmdGetAppletStateChangedEvent(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetAppletStateChangedEvent";

    if (appletStateChangedEvent.type == HandleType::None) {
        appletStateChangedEvent = kernel::makeEvent(true);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(appletStateChangedEvent);
}

void LibraryAppletAccessor::cmdGetResult(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetResult (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void LibraryAppletAccessor::cmdPopOutData(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "PopOutData (stubbed)";

    // TODO: what does this do?

    reply.makeReply(2);
    reply.write(KernelResult::NoDataInChannel);
}

void LibraryAppletAccessor::cmdPushInData(IPCContext &ctx, IPCContext &reply) {
    int objectID;
    std::memcpy(&objectID, ctx.getData(), sizeof(int));

    KService *storage = ctx.getDomainObject(objectID);

    PLOG_INFO << "PushInData (handle = " << std::hex << storage->getHandle().raw << ")";

    applet.pushInDataBase(((Storage *)storage)->data);

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void LibraryAppletAccessor::cmdStart(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "Start (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

LibraryAppletCreator::LibraryAppletCreator() {}

LibraryAppletCreator::~LibraryAppletCreator() {}

void LibraryAppletCreator::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case LibraryAppletCreatorCommand::CreateLibraryApplet:
            return cmdCreateLibraryApplet(ctx, reply);
        case LibraryAppletCreatorCommand::CreateStorage:
            return cmdCreateStorage(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void LibraryAppletCreator::cmdCreateLibraryApplet(IPCContext &ctx, IPCContext &reply) {
    u32 appletID;
    u32 libraryAppletMode;

    std::memcpy(&appletID, &((u8 *)ctx.getData())[0], sizeof(u32));
    std::memcpy(&libraryAppletMode, &((u8 *)ctx.getData())[4], sizeof(u32));

    PLOG_INFO << "CreateLibraryApplet (applet ID = " << std::hex << appletID << ", library applet mode = " << libraryAppletMode << ")";

    Handle handle = kernel::makeService<LibraryAppletAccessor>();

    LibraryAppletAccessor *libraryApplet = (LibraryAppletAccessor *)kernel::getObject(handle);
    libraryApplet->makeApplet(appletID);

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(handle);
}

void LibraryAppletCreator::cmdCreateStorage(IPCContext &ctx, IPCContext &reply) {
    u64 size;
    std::memcpy(&size, ctx.getData(), sizeof(u64));

    PLOG_INFO << "CreateStorage (size = " << std::hex << size << ")";

    const Handle handle = kernel::makeService<Storage>();
    ((Storage *)kernel::getObject(handle))->data.resize(size);

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(handle);
}

SelfController::SelfController() : accumulatedSuspendedTickChangedEvent(Handle{.raw = 0ULL}) {}

SelfController::~SelfController() {}

void SelfController::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case SelfControllerCommand::Exit:
            return cmdExit(ctx, reply);
        case SelfControllerCommand::GetLibraryAppletLaunchableEvent:
            return cmdGetLibraryAppletLaunchableEvent(ctx, reply);
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

void SelfController::cmdExit(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "Exit (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
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

void SelfController::cmdGetLibraryAppletLaunchableEvent(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetLibraryAppletLaunchableEvent";

    if (libraryAppletLaunchableEvent.type == HandleType::None) {
        libraryAppletLaunchableEvent = kernel::makeEvent(true);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(libraryAppletLaunchableEvent);
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

Storage::Storage() {}

Storage::~Storage() {}

void Storage::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case StorageCommand::Open:
            return cmdOpen(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void Storage::cmdOpen(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "Open";

    const Handle handle = kernel::makeService<StorageAccessor>();
    ((StorageAccessor *)kernel::getObject(handle))->setStorageHandle(getHandle());

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(handle);
}

StorageAccessor::StorageAccessor() {}

StorageAccessor::~StorageAccessor() {}

void StorageAccessor::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case StorageAccessorCommand::Write:
            return cmdWrite(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void StorageAccessor::setStorageHandle(Handle handle) {
    storageHandle = handle;
}

void StorageAccessor::cmdWrite(IPCContext &ctx, IPCContext &reply) {
    PLOG_INFO << "Write";

    Storage *storage = (Storage *)kernel::getObject(storageHandle);

    const std::vector<u8> data = ctx.readSend();

    std::memcpy(storage->data.data(), data.data(), data.size());
    
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
