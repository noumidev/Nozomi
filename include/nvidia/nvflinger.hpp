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

#include <array>
#include <cstring>

#include "ipc.hpp"
#include "object.hpp"
#include "types.hpp"

namespace nvidia::nvflinger {

using hle::IPCContext;
using hle::KService;

using DisplayName = std::array<char, 0x40>;

inline DisplayName makeDisplayName(const char *name) {
    DisplayName displayName;

    std::strcpy(displayName.data(), name);

    return displayName;
}

class Display {
    DisplayName name;

    u64 id;

public:
    Display(DisplayName name, u64 id);
    ~Display();

    DisplayName getName();

    u64 getID();
};

class HOSDriverBinder : public KService {
public:
    HOSDriverBinder();
    ~HOSDriverBinder();

    const char *getName() override {
        return "IHOSDriverBinder";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

void init();

void makeDisplay(DisplayName name);

u64 openDisplay(DisplayName name);

}
