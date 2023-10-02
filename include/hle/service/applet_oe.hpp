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
#include "object.hpp"
#include "types.hpp"

namespace hle::service::applet_oe {

void handleRequest(IPCContext &ctx, IPCContext &reply);

void cmdOpenApplicationProxy(IPCContext &ctx, IPCContext &reply);

class ApplicationFunctions : public KService {
    void cmdNotifyRunning(IPCContext &ctx, IPCContext &reply);

public:
    ApplicationFunctions();
    ~ApplicationFunctions();

    const char *getName() override {
        return "IApplicationFunctions";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class ApplicationProxy : public KService {
    void cmdGetApplicationFunctions(IPCContext &ctx, IPCContext &reply);
    void cmdGetAudioController(IPCContext &ctx, IPCContext &reply);
    void cmdGetCommonStateGetter(IPCContext &ctx, IPCContext &reply);
    void cmdGetDebugFunctions(IPCContext &ctx, IPCContext &reply);
    void cmdGetDisplayController(IPCContext &ctx, IPCContext &reply);
    void cmdGetLibraryAppletCreator(IPCContext &ctx, IPCContext &reply);
    void cmdGetSelfController(IPCContext &ctx, IPCContext &reply);
    void cmdGetWindowController(IPCContext &ctx, IPCContext &reply);

public:
    ApplicationProxy();
    ~ApplicationProxy();

    const char *getName() override {
        return "IApplicationProxy";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class AudioController : public KService {
public:
    AudioController();
    ~AudioController();

    const char *getName() override {
        return "IAudioController";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class CommonStateGetter : public KService {
    Handle event;

    void cmdGetCurrentFocusState(IPCContext &ctx, IPCContext &reply);
    void cmdGetEventHandle(IPCContext &ctx, IPCContext &reply);
    void cmdGetOperationMode(IPCContext &ctx, IPCContext &reply);
    void cmdGetPerformanceMode(IPCContext &ctx, IPCContext &reply);

public:
    CommonStateGetter();
    ~CommonStateGetter();

    const char *getName() override {
        return "ICommonStateGetter";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class DebugFunctions : public KService {
public:
    DebugFunctions();
    ~DebugFunctions();

    const char *getName() override {
        return "IDebugFunctions";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class DisplayController : public KService {
public:
    DisplayController();
    ~DisplayController();

    const char *getName() override {
        return "IDisplayController";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class LibraryAppletCreator : public KService {
public:
    LibraryAppletCreator();
    ~LibraryAppletCreator();

    const char *getName() override {
        return "LibraryAppletCreator";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class SelfController : public KService {
    Handle accumulatedSuspendedTickChangedEvent;

    void cmdGetAccumulatedSuspendedTickChangedEvent(IPCContext &ctx, IPCContext &reply);
    void cmdSetFocusHandlingMode(IPCContext &ctx, IPCContext &reply);
    void cmdSetOperationModeChangedNotification(IPCContext &ctx, IPCContext &reply);
    void cmdSetOutOfFocusSuspendingEnabled(IPCContext &ctx, IPCContext &reply);
    void cmdSetPerformanceModeChangedNotification(IPCContext &ctx, IPCContext &reply);

public:
    SelfController();
    ~SelfController();

    const char *getName() override {
        return "ISelfController";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class WindowController : public KService {
    void cmdAcquireForegroundRights(IPCContext &ctx, IPCContext &reply);
    void cmdGetAppletResourceUserId(IPCContext &ctx, IPCContext &reply);

public:
    WindowController();
    ~WindowController();

    const char *getName() override {
        return "IWindowController";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

}
