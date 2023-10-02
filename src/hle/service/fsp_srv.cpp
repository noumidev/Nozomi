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

#include "fsp_srv.hpp"

#include <plog/Log.h>

#include "result.hpp"

namespace hle::service::fsp_srv {

namespace Command {
    enum : u32 {
        SetCurrentProcess = 1,
    };
}

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::SetCurrentProcess:
            return cmdSetCurrentProcess(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdSetCurrentProcess(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "SetCurrentProcess (stubbed)";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

}
