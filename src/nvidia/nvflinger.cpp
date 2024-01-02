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

#include "nvflinger.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>

#include <plog/Log.h>

#include "buffer_queue.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace nvidia::nvflinger {

using android::buffer_queue::BufferQueue;

using namespace hle;

namespace NativeWindowAPI {
    enum : u32 {
        CPU = 2,
    };
}

namespace HOSDriverBinderCommand {
    enum : u32 {
        AdjustRefcount = 1,
        GetNativeHandle,
        TransactParcelAuto,
    };
}

std::vector<Display> displays;
std::vector<BufferQueue> bufferQueues;

void init() {
    makeDisplay(makeDisplayName("Default"));
}

void makeDisplay(DisplayName name) {
    static u64 displayID = 0;

    PLOG_DEBUG << "Making display (name = " << name.data() << ", ID = " << displayID << ")";

    displays.emplace_back(Display(name, displayID++));
}

Display *getDisplay(u64 id) {
    if (id > displays.size()) {
        PLOG_FATAL << "Invalid display ID";

        exit(0);
    }

    return &displays[id];
}

u64 openDisplay(DisplayName name) {
    PLOG_DEBUG << "Opening display (name = " << name.data() << ")";

    for (auto &display : displays) {
        if (std::strcmp(display.getName().data(), name.data()) == 0) {
            return display.getID();
        }
    }

    PLOG_FATAL << "Display does not exist";

    exit(0);
}

u32 makeBufferQueue() {
    static u32 bufferQueueID = 0;

    PLOG_DEBUG << "Making buffer queue";

    bufferQueues.emplace_back(BufferQueue{});

    return bufferQueueID++;
}

u64 makeLayer(u64 displayID) {
    static u64 layerID = 0;

    PLOG_DEBUG << "Making layer (display ID = " << displayID << ")";

    getDisplay(displayID)->makeLayer(layerID);

    return layerID++;
}

u32 getBufferQueueID(u64 displayID, u64 layerID) {
    return getDisplay(displayID)->getLayer(layerID)->getBufferQueueID();
}

void connect(bool enableListener, u32 api, bool producerControlledByApp, Parcel &out) {
    (void)producerControlledByApp;
    (void)out;

    if (enableListener) {
        PLOG_FATAL << "Unimplemented listener";

        exit(0);
    }

    switch (api) {
        case NativeWindowAPI::CPU:
            break;
        default:
            PLOG_FATAL << "Unimplemented API " << api;

            exit(0);
    }

    PLOG_FATAL << "Unimplemented CONNECT";

    exit(0);
}

Layer::Layer(u64 id) : id(id), bufferQueueID(makeBufferQueue()) {}

Layer::~Layer() {}

u64 Layer::getID() {
    return id;
}

u32 Layer::getBufferQueueID() {
    return bufferQueueID;
}

Display::Display(DisplayName name, u64 id) : name(name), id(id) {}

Display::~Display() {}

DisplayName Display::getName() {
    return name;
}

u64 Display::getID() {
    return id;
}

Layer *Display::getLayer(u64 layerID) {
    for (auto &layer : layers) {
        if (layer.getID() == layerID) {
            return &layer;
        }
    }

    PLOG_FATAL << "Unable to find layer with ID " << layerID;

    exit(0);
}

void Display::makeLayer(u64 id) {
    layers.emplace_back(Layer(id));
}

HOSDriverBinder::HOSDriverBinder() : event(Handle{.raw = 0ULL}), strongRefcount(0), weakRefcount(0) {}

HOSDriverBinder::~HOSDriverBinder() {}

void HOSDriverBinder::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        case HOSDriverBinderCommand::AdjustRefcount:
            cmdAdjustRefcount(ctx, reply);
            break;
        case HOSDriverBinderCommand::GetNativeHandle:
            cmdGetNativeHandle(ctx, reply);
            break;
        case HOSDriverBinderCommand::TransactParcelAuto:
            cmdTransactParcelAuto(ctx, reply);
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void HOSDriverBinder::cmdAdjustRefcount(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    i32 id, addval, type;
    std::memcpy(&id, &data[0], sizeof(i32));
    std::memcpy(&addval, &data[4], sizeof(i32));
    std::memcpy(&type, &data[8], sizeof(i32));

    PLOG_INFO << "AdjustRefcount (ID = " << id << ", addval = " << addval << ", type = " << type << ")";

    if ((type < 0) || (type > 1)) {
        PLOG_FATAL << "Invalid type";

        exit(0);
    }

    const bool adjustWeak = type == 0;
    if (adjustWeak) {
        weakRefcount += addval;

        PLOG_DEBUG << "New weak refcount = " << weakRefcount;
    } else {
        strongRefcount += addval;

        PLOG_DEBUG << "New strong refcount = " << strongRefcount;
    }

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void HOSDriverBinder::cmdGetNativeHandle(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    i32 id, unknown;
    std::memcpy(&id, &data[0], sizeof(i32));
    std::memcpy(&unknown, &data[4], sizeof(i32));

    PLOG_INFO << "GetNativeHandle (ID = " << id << ", unknown = " << unknown << ")";

    if (event.type == HandleType::None) {
        event = kernel::makeEvent(true); // Maybe?
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(event);
}

void HOSDriverBinder::cmdTransactParcelAuto(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u8 *data = (u8 *)ctx.getData();

    i32 id;
    u32 code, flags;
    std::memcpy(&id, &data[0], sizeof(i32));
    std::memcpy(&code, &data[4], sizeof(u32));
    std::memcpy(&flags, &data[8], sizeof(u32));

    PLOG_INFO << "cmdTransactParcelAuto (ID = " << id << ", code = " << code << ", flags = " << std::hex << flags << ")";

    android::buffer_queue::transact(ctx, code, flags);

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

NativeWindow::NativeWindow(u32 bufferQueueID) : bufferQueueID((u64)bufferQueueID) {}

NativeWindow::~NativeWindow() {}

std::vector<u8> NativeWindow::serialize() {
    std::vector<u8> data;

    data.resize(SIZE);

    // NativeWindow binary format (https://github.com/yuzu-emu/yuzu/blob/12178c694ab20898c2d007e0efb30a28d1aee100/src/core/hle/service/vi/vi.cpp#L68-L73)
    std::memcpy(&data[0x00], &MAGIC, sizeof(MAGIC));
    std::memcpy(&data[0x04], &PROCESS_ID, sizeof(PROCESS_ID));
    std::memcpy(&data[0x08], &bufferQueueID, sizeof(bufferQueueID));
    std::memcpy(&data[0x10], DISPDRV_NAME, 8);

    return data;
}

}
