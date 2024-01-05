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

#include "nvdrv.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

#include "nvhost_ctrl.hpp"
#include "nvmap.hpp"

namespace hle::service::nvdrv {

namespace Command {
    enum : u32 {
        Open = 0,
        Ioctl,
        Initialize = 3,
        QueryEvent,
    };
}

Handle transferMemory = Handle{.raw = 0};

std::vector<NVFile> files;

FileDescriptor open(const char *path) {
    static FileDescriptor nextFD = 0;

    PLOG_VERBOSE << "Opening file " << path;

    files.emplace_back(NVFile(nextFD));

    NVFile &file = files[files.size() - 1];

    if (std::strcmp(path, "/dev/nvmap") == 0) {
        file.ioctl = nvmap::ioctl;
    } else if (std::strcmp(path, "/dev/nvhost-ctrl") == 0) {
        file.ioctl = nvhost_ctrl::ioctl;
    } else {
        PLOG_FATAL << "Unrecognized file path";

        exit(0);
    }

    return nextFD++;
}

i32 ioctl(FileDescriptor fd, u32 iocode, IPCContext &ctx, IPCContext &reply) {
    (void)ctx;
    (void)reply;

    PLOG_VERBOSE << "ioctl (fd = " << fd << ", iocode = " << std::hex << iocode << ")";

    if (fd > (i32)files.size()) {
        PLOG_FATAL << "Invalid file descriptor";

        exit(0);
    }

    NVFile &file = files[fd];

    if (file.isClosed()) {
        PLOG_FATAL << "Cannot send ioctl to closed file";

        exit(0);
    }

    if (file.ioctl == NULL) {
        PLOG_FATAL << "Invalid ioctl handler";

        exit(0);
    }

    return file.ioctl(iocode, ctx);
}

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::Open:
            cmdOpen(ctx, reply);
            break;
        case Command::Ioctl:
            cmdIoctl(ctx, reply);
            break;
        case Command::Initialize:
            cmdInitialize(ctx, reply);
            break;
        case Command::QueryEvent:
            cmdQueryEvent(ctx, reply);
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdInitialize(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    u64 size;
    std::memcpy(&size, data, sizeof(u64));

    const std::vector<Handle> copyHandles = ctx.getCopyHandles();

    if (copyHandles.size() < 2) {
        PLOG_FATAL << "Invalid number of copy handles";

        exit(0);
    }

    PLOG_INFO << "Initialize (process = " << std::hex << copyHandles[0].raw << ", transfer memory = " << copyHandles[1].raw << ", size = " << size << ")";

    if (transferMemory.type != HandleType::None) {
        PLOG_FATAL << "nvdrv already initialized";

        exit(0);
    }

    transferMemory = kernel::copyHandle(copyHandles[1]);

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void cmdIoctl(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    u32 fd, iocode;
    std::memcpy(&fd, &data[0], sizeof(u32));
    std::memcpy(&iocode, &data[4], sizeof(u32));

    PLOG_INFO << "Ioctl (fd = " << fd << ", iocode = " << std::hex << iocode << ")";

    reply.makeReply(3);
    reply.write(KernelResult::Success);
    reply.write(ioctl(fd, iocode, ctx, reply));
}

void cmdOpen(IPCContext &ctx, IPCContext &reply) {
    char path[32];
    std::memset(path, 0, sizeof(path));
    std::strncpy(path, (const char *)ctx.readSend().data(), sizeof(path));

    PLOG_INFO << "Open (path = " << path << ")";

    FileDescriptor fd = open(path);

    reply.makeReply(4);
    reply.write(KernelResult::Success);
    reply.write(fd);

    if (fd < 0) {
        PLOG_FATAL << "Invalid file descriptor";

        exit(0);
    } else {
        reply.write(NVResult::Success);
    }
}

void cmdQueryEvent(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    u32 fd, evtID;
    std::memcpy(&fd, &data[0], sizeof(u32));
    std::memcpy(&evtID, &data[4], sizeof(u32));

    PLOG_INFO << "QueryEvent (fd = " << fd << ", event ID = " << std::hex << evtID << ") (stubbed)";

    PLOG_ERROR << "Unimplemented QueryEvent";

    reply.makeReply(2, 0);
    reply.write(-1LL);
    //reply.write(NVResult::Success);
    //reply.copyHandle(kernel::makeEvent(true));
}

}
