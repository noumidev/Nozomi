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

#include "ipc_manager.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <map>
#include <string>

#include <plog/Log.h>

#include "kernel.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "result.hpp"

#include "apm.hpp"
#include "applet_oe.hpp"
#include "fsp_srv.hpp"
#include "hid.hpp"
#include "set_sys.hpp"
#include "sm.hpp"
#include "time.hpp"

namespace hle::ipc {

using ServiceFunction = void(*)(IPCContext &, IPCContext &);

constexpr u16 POINTER_BUFFER_SIZE = 0x8000; // Value taken from Yuzu

static std::map<std::string, ServiceFunction> requestFuncMap {
    {std::string("apm"), &service::apm::handleRequest},
    {std::string("appletOE"), &service::applet_oe::handleRequest},
    {std::string("fsp-srv"), &service::fsp_srv::handleRequest},
    {std::string("hid"), &service::hid::handleRequest},
    {std::string("set:sys"), &service::set_sys::handleRequest},
    {std::string("sm:"), &service::sm::handleRequest},
    {std::string("time:u"), &service::time::handleRequest},
};

namespace Command {
    enum : u32 {
        ConvertCurrentObjectToDomain,
        CloneCurrentObject = 2,
        QueryPointerBufferSize,
    };
}

void sendSyncRequest(Handle handle, u64 ipcMessage) {
    // Reset ctx
    IPCContext ctx(sys::memory::getPointer(ipcMessage), kernel::getObject(handle));

    KObject *session = ctx.getService();

    // Get service name from handle
    const char *name;
    switch (handle.type) {
        case HandleType::KService:
            name = ((KService *)session)->getName();
            break;
        case HandleType::KServiceSession:
            name = ((KServiceSession *)session)->getName();
            break;
        case HandleType::KSession:
            name = ((KPort *)kernel::getObject(((KSession *)session)->getPortHandle()))->getName();
            break;
        default:
            PLOG_FATAL << "Unimplemented handle type " << handle.type;

            exit(0);
    }

    PLOG_INFO << "Sending sync request to " << name << " (IPC message* = " << std::hex << ipcMessage << ")";

    ctx.unmarshal();

    const u64 commandType = ctx.getCommandType();

    if (commandType == CommandType::Close) {
        PLOG_INFO << "Closing service session (handle = " << std::hex << handle.raw << ")";
        
        return;
    }

    IPCContext reply(ctx.getIPCPointer(), ctx.getService());

    switch (commandType) {
        case CommandType::Invalid: // Wow
            PLOG_FATAL << "Invalid IPC type";

            exit(0);
        case CommandType::Request:
            {
                const int objectID = (int)ctx.getObjectID();
                if (ctx.isDomain() && (objectID > 1)) {
                    ((KServiceSession *)session)->handleRequest(objectID, ctx, reply);
                } else {
                    if (session->getHandle().type == HandleType::KService) { // Bypass service table
                        ((KService *)session)->handleRequest(ctx, reply);
                    } else {
                        const auto requestFunc = requestFuncMap.find(std::string(name));
                        if (requestFunc == requestFuncMap.end()) {
                            PLOG_FATAL << "Request to unimplemented service " << name;

                            exit(0);
                        }
                        
                        requestFunc->second(ctx, reply);
                    }
                }
            }
            break;
        case CommandType::Control:
            return handleControl(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented IPC type " << commandType;

            exit(0);
    }

    reply.marshal();
}

void handleControl(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::ConvertCurrentObjectToDomain:
            {
                PLOG_INFO << "ConvertCurrentObjectToDomain";

                if (ctx.getService()->getHandle().type != HandleType::KServiceSession) {
                    PLOG_FATAL << "Cannot convert current object to domain";

                    exit(0);
                }

                KServiceSession *serviceSession = (KServiceSession *)ctx.getService();

                reply.makeReply(3);
                reply.write(KernelResult::Success);
                reply.write(serviceSession->add(serviceSession->getHandle()));

                reply.marshal();

                serviceSession->makeDomain();
            }
            break;
        case Command::CloneCurrentObject:
            PLOG_INFO << "CloneCurrentObject";

            // TODO: create new object, don't just copy the handle

            reply.makeReply(2, 0, 1, true);
            reply.write(KernelResult::Success);
            reply.moveHandle(kernel::copyHandle(ctx.getService()->getHandle()));

            reply.marshal();
            break;
        case Command::QueryPointerBufferSize:
            PLOG_INFO << "QueryPointerBufferSize (stubbed)";

            reply.makeReply(3);
            reply.write(KernelResult::Success);
            reply.write(POINTER_BUFFER_SIZE);

            reply.marshal();
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

}
