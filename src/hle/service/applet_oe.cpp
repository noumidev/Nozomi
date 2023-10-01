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

#include "applet_oe.hpp"

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::applet_oe {

namespace Command {
    enum : u32 {
        OpenApplicationProxy = 0,
    };
}

Result handleRequest(u32 command, u32 *data, IPCReply &reply) {
    switch (command) {
        case Command::OpenApplicationProxy:
            return cmdOpenApplicationProxy(data, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

Result cmdOpenApplicationProxy(u32 *data, IPCReply &reply) {
    (void)data;

    PLOG_INFO << "OpenApplicationProxy";

    const Handle handle = kernel::makeService<ApplicationProxy>();

    reply.write(handle.raw);

    return KernelResult::Success;
}

ApplicationProxy::ApplicationProxy() {}

ApplicationProxy::~ApplicationProxy() {}

Result ApplicationProxy::handleRequest(u32 command, u32 *data, IPCReply &reply) {
    (void)data;
    (void)reply;

    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

}
