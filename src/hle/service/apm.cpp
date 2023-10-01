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

#include "apm.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::apm {

namespace Command {
    enum : u32 {
        OpenSession = 0,
    };
}

Result handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case Command::OpenSession:
            return cmdOpenSession(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result cmdOpenSession(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "OpenSession";

    const Handle handle = kernel::makeServiceSession("apm_manager");

    reply.write(handle.raw);

    return KernelResult::Success;
}

}
