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
#include "types.hpp"

namespace hle::service::time {

void handleRequest(IPCContext &ctx, IPCContext &reply);

void cmdGetSharedMemoryNativeHandle(IPCContext &ctx, IPCContext &reply);
void cmdGetStandardLocalSystemClock(IPCContext &ctx, IPCContext &reply);
void cmdGetStandardNetworkSystemClock(IPCContext &ctx, IPCContext &reply);
void cmdGetStandardSteadyClock(IPCContext &ctx, IPCContext &reply);
void cmdGetStandardUserSystemClock(IPCContext &ctx, IPCContext &reply);
void cmdGetTimeZoneService(IPCContext &ctx, IPCContext &reply);

class SteadyClock : public KService {
public:
    SteadyClock();
    ~SteadyClock();

    const char *getName() override {
        return "ISteadyClock";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class SystemClock : public KService {
public:
    SystemClock();
    ~SystemClock();

    const char *getName() override {
        return "ISystemClock";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class TimeZoneService : public KService {
public:
    TimeZoneService();
    ~TimeZoneService();

    const char *getName() override {
        return "ITimeZoneService";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

}
