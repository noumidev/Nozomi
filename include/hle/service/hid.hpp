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

namespace hle::service::hid {

void handleRequest(IPCContext &ctx, IPCContext &reply);

void cmdCreateAppletResource(IPCContext &ctx, IPCContext &reply);

class AppletResource : public KService {
public:
    AppletResource();
    ~AppletResource();

    const char *getName() override {
        return "IAppletResource";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

}
