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

namespace hle::service::vi {

void managerHandleRequest(IPCContext &ctx, IPCContext &reply);

void cmdGetDisplayService(IPCContext &ctx, IPCContext &reply);

class ApplicationDisplayService : public KService {
    void cmdCloseDisplay(IPCContext &ctx, IPCContext &reply);
    void cmdCloseLayer(IPCContext &ctx, IPCContext &reply);
    void cmdGetIndirectDisplayTransactionService(IPCContext &ctx, IPCContext &reply);
    void cmdGetManagerDisplayService(IPCContext &ctx, IPCContext &reply);
    void cmdGetRelayService(IPCContext &ctx, IPCContext &reply);
    void cmdGetSystemDisplayService(IPCContext &ctx, IPCContext &reply);
    void cmdOpenDisplay(IPCContext &ctx, IPCContext &reply);
    void cmdSetLayerScalingMode(IPCContext &ctx, IPCContext &reply);

public:
    ApplicationDisplayService();
    ~ApplicationDisplayService();

    const char *getName() override {
        return "IApplicationDisplayService";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class ManagerDisplayService : public KService {
    void cmdCreateStrayLayer(IPCContext &ctx, IPCContext &reply);

public:
    ManagerDisplayService();
    ~ManagerDisplayService();

    const char *getName() override {
        return "IManagerDisplayService";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class SystemDisplayService : public KService {
public:
    SystemDisplayService();
    ~SystemDisplayService();

    const char *getName() override {
        return "ISystemDisplayService";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

}
