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
#include <vector>

#include "handle.hpp"
#include "ipc.hpp"
#include "object.hpp"
#include "types.hpp"

namespace nvidia::nvflinger {

using hle::Handle;
using hle::IPCContext;
using hle::KService;

using DisplayName = std::array<char, 0x40>;

inline DisplayName makeDisplayName(const char *name) {
    DisplayName displayName;

    std::strcpy(displayName.data(), name);

    return displayName;
}

class Layer {
    u64 id;
    u32 bufferQueueID;

public:
    Layer(u64 id);
    ~Layer();

    u64 getID();
    u32 getBufferQueueID();
};

class Display {
    DisplayName name;

    u64 id;

    std::vector<Layer> layers;

public:
    Display(DisplayName name, u64 id);
    ~Display();

    DisplayName getName();

    u64 getID();

    Layer *getLayer(u64 layerID);

    void makeLayer(u64 id);
};

class HOSDriverBinder : public KService {
    Handle event;

    i32 strongRefcount, weakRefcount;

    void cmdAdjustRefcount(IPCContext &ctx, IPCContext &reply);
    void cmdGetNativeHandle(IPCContext &ctx, IPCContext &reply);
    void cmdTransactParcelAuto(IPCContext &ctx, IPCContext &reply);
public:
    HOSDriverBinder();
    ~HOSDriverBinder();

    const char *getName() override {
        return "IHOSDriverBinder";
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

class NativeWindow {
    static constexpr size_t SIZE = 0x28;

    static constexpr u32 MAGIC = 2;
    static constexpr u32 PROCESS_ID = 1;

    static constexpr const char *DISPDRV_NAME = "dispdrv";

    u64 bufferQueueID;

public:
    NativeWindow(u32 bufferQueueID);
    ~NativeWindow();

    std::vector<u8> serialize();
};

void init();

void makeDisplay(DisplayName name);

u64 openDisplay(DisplayName name);

u64 makeLayer(u64 displayID);

u32 getBufferQueueID(u64 displayID, u64 layerID);

}
