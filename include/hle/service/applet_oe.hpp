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

#include "ipc_reply.hpp"
#include "object.hpp"
#include "types.hpp"

namespace hle::service::applet_oe {

Result handleRequest(u32 command, u32 *data, IPCReply &reply);

Result cmdOpenApplicationProxy(u32 *data, IPCReply &reply);

class ApplicationFunctions : public KService {
    Result cmdNotifyRunning(u32 *data, IPCReply &reply);

public:
    ApplicationFunctions();
    ~ApplicationFunctions();

    const char *getName() override {
        return "IApplicationFunctions";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class ApplicationProxy : public KService {
    Result cmdGetApplicationFunctions(u32 *data, IPCReply &reply);
    Result cmdGetAudioController(u32 *data, IPCReply &reply);
    Result cmdGetCommonStateGetter(u32 *data, IPCReply &reply);
    Result cmdGetDebugFunctions(u32 *data, IPCReply &reply);
    Result cmdGetDisplayController(u32 *data, IPCReply &reply);
    Result cmdGetLibraryAppletCreator(u32 *data, IPCReply &reply);
    Result cmdGetSelfController(u32 *data, IPCReply &reply);
    Result cmdGetWindowController(u32 *data, IPCReply &reply);

public:
    ApplicationProxy();
    ~ApplicationProxy();

    const char *getName() override {
        return "IApplicationProxy";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class AudioController : public KService {
public:
    AudioController();
    ~AudioController();

    const char *getName() override {
        return "IAudioController";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class CommonStateGetter : public KService {
    Handle event;

    Result cmdGetCurrentFocusState(u32 *data, IPCReply &reply);
    Result cmdGetEventHandle(u32 *data, IPCReply &reply);
    Result cmdGetOperationMode(u32 *data, IPCReply &reply);
    Result cmdGetPerformanceMode(u32 *data, IPCReply &reply);

public:
    CommonStateGetter();
    ~CommonStateGetter();

    const char *getName() override {
        return "ICommonStateGetter";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class DebugFunctions : public KService {
public:
    DebugFunctions();
    ~DebugFunctions();

    const char *getName() override {
        return "IDebugFunctions";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class DisplayController : public KService {
public:
    DisplayController();
    ~DisplayController();

    const char *getName() override {
        return "IDisplayController";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class LibraryAppletCreator : public KService {
public:
    LibraryAppletCreator();
    ~LibraryAppletCreator();

    const char *getName() override {
        return "LibraryAppletCreator";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class SelfController : public KService {
    Handle accumulatedSuspendedTickChangedEvent;

    Result cmdGetAccumulatedSuspendedTickChangedEvent(u32 *data, IPCReply &reply);
    Result cmdSetFocusHandlingMode(u32 *data, IPCReply &reply);
    Result cmdSetOperationModeChangedNotification(u32 *data, IPCReply &reply);
    Result cmdSetOutOfFocusSuspendingEnabled(u32 *data, IPCReply &reply);
    Result cmdSetPerformanceModeChangedNotification(u32 *data, IPCReply &reply);

public:
    SelfController();
    ~SelfController();

    const char *getName() override {
        return "ISelfController";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

class WindowController : public KService {
    Result cmdAcquireForegroundRights(u32 *data, IPCReply &reply);
    Result cmdGetAppletResourceUserId(u32 *data, IPCReply &reply);

public:
    WindowController();
    ~WindowController();

    const char *getName() override {
        return "IWindowController";
    }

    Result handleRequest(u32 command, u32 *data, IPCReply &reply) override;
};

}
