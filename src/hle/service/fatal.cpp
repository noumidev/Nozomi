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

#include "fatal.hpp"

#include <cstring>
#include <ios>

#include <plog/Log.h>

#include "result.hpp"

namespace hle::service::fatal {

namespace FatalType {
    enum : u32 {
        ErrorReportAndScreen,
        ErrorReport,
        ErrorScreen,
    };
}

namespace Command {
    enum : u32 {
        ThrowFatalWithPolicy = 1,
    };
}

const char *getFatalTypeName(u32 fatalType) {
    switch (fatalType) {
        case FatalType::ErrorReportAndScreen:
            return "ErrorReportAndScreen";
        case FatalType::ErrorReport:
            return "ErrorReport";
        case FatalType::ErrorScreen:
            return "ErrorScreen";
        default:
            PLOG_FATAL << "Invalid fatal type";

            exit(0);
    }
}

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::ThrowFatalWithPolicy:
            cmdThrowFatalWithPolicy(ctx, reply);
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdThrowFatalWithPolicy(IPCContext &ctx, IPCContext &reply) {
    const u8 *data = (u8 *)ctx.getData();

    u32 errorCode, fatalType;
    std::memcpy(&errorCode, &data[0], sizeof(u32));
    std::memcpy(&fatalType, &data[4], sizeof(u32));

    PLOG_ERROR << "ThrowFatalWithPolicy (module = " << getModuleName(getModule(errorCode)) << ", description = " << getDescription(errorCode) << ", fatal type = " << getFatalTypeName(fatalType) << ")";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

}
