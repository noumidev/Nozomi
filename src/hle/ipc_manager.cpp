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

#include "ipc_buffer.hpp"
#include "ipc_reply.hpp"
#include "kernel.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "result.hpp"

#include "apm.hpp"
#include "set_sys.hpp"
#include "sm.hpp"

namespace hle::ipc {

using ServiceFunction = Result(*)(u32, u32 *, IPCReply &);

constexpr u32 makeMagic(const char *magic) {
    return (u32)magic[0] | ((u32)magic[1] << 8) | ((u32)magic[2] << 16) | ((u32)magic[3] << 24);
}

constexpr u16 POINTER_BUFFER_SIZE = 0x8000; // Value taken from Yuzu

constexpr u32 INPUT_HEADER_MAGIC = makeMagic("SFCI");
constexpr u32 OUTPUT_HEADER_MAGIC = makeMagic("SFCO");

static std::map<std::string, ServiceFunction> requestFuncMap {
    {std::string("apm"), &service::apm::handleRequest},
    {std::string("set:sys"), &service::set_sys::handleRequest},
    {std::string("sm:"), &service::sm::handleRequest},
};

namespace CommandType {
    enum : u32 {
        Invalid,
        LegacyRequest,
        Close,
        LegacyControl,
        Request,
        Control,
        RequestWithContext,
        ControlWithContext,
    };
}

namespace Command {
    enum : u32 {
        ConvertCurrentObjectToDomain = 0,
        QueryPointerBufferSize = 3,
    };
}

union Header {
    u64 raw;
    struct {
        u64 type : 16;
        u64 numX : 4;
        u64 numA : 4;
        u64 numB : 4;
        u64 numW : 4;
        u64 dataSize : 10;
        u64 flagsC : 4;
        u64 : 17;
        u64 hasHandleDescriptor : 1;
    };
};

static_assert(sizeof(Header) == sizeof(u64));

union HandleDescriptor {
    u32 raw;
    struct {
        u32 sendPID : 1;
        u32 numCopyHandles : 4;
        u32 numMoveHandles : 4;
        u32 : 23;
    };
};

static_assert(sizeof(HandleDescriptor) == sizeof(u32));

union CBufferDescriptor {
    u64 raw;
    struct {
        u64 address : 48;
        u64 size : 16;
    };
};

static_assert(sizeof(CBufferDescriptor) == sizeof(u64));

KObject *context;

bool isDomain() {
    KDomain *domain = dynamic_cast<KDomain *>(context);

    if (domain == NULL) {
        return false;
    }

    return domain->isDomainObject();
}

void writeOutputHeader(u32 *data, Result result) {
    data[DataPayloadOffset::Magic] = OUTPUT_HEADER_MAGIC;
    data[DataPayloadOffset::Version] = 0;
    data[DataPayloadOffset::Result] = result;
}

void writeCBuffer(IPCBuffer &ipcBuffer, IPCReply &reply, u64 numC) {
    if (numC == 0) {
        PLOG_FATAL << "Invalid number of C buffer descriptors";

        exit(0);
    }

    CBufferDescriptor descriptors[numC];

    for (u64 i = 0; i < numC; i++) {
        auto &descriptor = descriptors[i];

        descriptor = CBufferDescriptor{.raw = ipcBuffer.read<u64>()};

        PLOG_VERBOSE << "C buffer descriptor (num = " << i << ", addr = " << std::hex << descriptor.address << ", size = " << descriptor.size << ")";
    }

    const auto &descriptor = descriptors[0];
    
    std::memcpy(sys::memory::getPointer(descriptor.address), reply.get(), std::min(descriptor.size, reply.getSize()));
}

void sendSyncRequest(Handle handle, u64 ipcMessage) {
    context = kernel::getObject(handle);

    // Get service name from handle
    const char *name;
    switch (handle.type) {
        case HandleType::KServiceSession:
            name = ((KServiceSession *)context)->getName();
            break;
        case HandleType::KSession:
            name = ((KPort *)kernel::getObject(((KSession *)context)->getPortHandle()))->getName();
            break;
        default:
            PLOG_FATAL << "Unimplemented handle type " << handle.type;

            exit(0);
    }

    PLOG_INFO << "Sending sync request to " << name << " (IPC message* = " << std::hex << ipcMessage << ")";

    if (isDomain()) {
        PLOG_FATAL << "Unimplemented domain";

        exit(0);
    }

    IPCBuffer ipcBuffer(ipcMessage);

    Header header{.raw = ipcBuffer.read<u64>()};

    PLOG_VERBOSE << "IPC header = " << std::hex << header.raw;

    if ((header.numX | header.numA | header.numB | header.numW) != 0) {
        PLOG_FATAL << "Unimplemented buffer descriptors";

        exit(0);
    }

    if (header.hasHandleDescriptor != 0) {
        HandleDescriptor handleDescriptor{.raw = ipcBuffer.read<u32>()};

        PLOG_VERBOSE << "Handle descriptor = " << std::hex << handleDescriptor.raw;

        if (handleDescriptor.sendPID != 0) {
            PLOG_VERBOSE << "PID = " << std::hex << ipcBuffer.read<u32>();
        }

        for (u32 copyHandle = 0; copyHandle < handleDescriptor.numCopyHandles; copyHandle++) {
            PLOG_VERBOSE << "Copy handle " << std::hex << ipcBuffer.read<u32>();
        }

        for (u32 moveHandle = 0; moveHandle < handleDescriptor.numMoveHandles; moveHandle++) {
            PLOG_VERBOSE << "Move handle " << std::hex << ipcBuffer.read<u32>();
        }
    }

    if (header.type == CommandType::Close) {
        PLOG_INFO << "Closing service session (handle = " << std::hex << handle.raw << ")";
        
        return;
    }

    // Get beginning of raw data
    ipcBuffer.alignUp();

    u32 *data = (u32 *)ipcBuffer.get();

    // Confirm input header and write output header
    if (data[DataPayloadOffset::Magic] != INPUT_HEADER_MAGIC) {
        PLOG_FATAL << "Invalid data payload header (" << std::hex << data[DataPayloadOffset::Magic] << ")";

        exit(0);
    }

    IPCReply reply;

    Result result;
    switch (header.type) {
        case CommandType::Invalid: // Wow
            PLOG_FATAL << "Invalid IPC type";

            exit(0);
        case CommandType::Request:
            {
                const auto requestFunc = requestFuncMap.find(std::string(name));
                if (requestFunc == requestFuncMap.end()) {
                    PLOG_FATAL << "Request to unimplemented service " << name;

                    exit(0);
                }
                
                result = requestFunc->second(data[DataPayloadOffset::Command], &data[DataPayloadOffset::Parameters], reply);
            }
            break;
        case CommandType::Control:
            return writeOutputHeader(data, handleControl(data[DataPayloadOffset::Command], &data[DataPayloadOffset::Parameters]));
        default:
            PLOG_FATAL << "Unimplemented IPC type " << header.type;

            exit(0);
    }

    writeOutputHeader(data, result);

    // Skip over command packet
    ipcBuffer.setOffset(sizeof(u32) * header.dataSize);
    ipcBuffer.alignUp();

    // Write output to memory
    switch (header.flagsC) {
        case 0: // No C buffer
            // Note: the reply appears to go after the command packet header
            // if there is no handle descriptor
            if (!header.hasHandleDescriptor) {
                ipcBuffer.setOffset(sizeof(header.raw));
            }
        case 1: // Inlined C buffer
            std::memcpy(ipcBuffer.get(), reply.get(), reply.getSize());
            return;
        default:
            break;
    }

    writeCBuffer(ipcBuffer, reply, header.flagsC - 2);
}

Result handleControl(u32 command, u32 *data) {
    switch (command) {
        case Command::ConvertCurrentObjectToDomain:
            {
                PLOG_INFO << "ConvertCurrentObjectToDomain";
                
                if (context->getHandle().type != HandleType::KServiceSession) {
                    PLOG_FATAL << "Cannot convert current object to domain";

                    exit(0);
                }

                KServiceSession *serviceSession = (KServiceSession *)context;

                serviceSession->makeDomain();
                serviceSession->add(serviceSession->getHandle());

                data[0] = 0; // Is always 0?
            }
            break;
        case Command::QueryPointerBufferSize:
            PLOG_INFO << "QueryPointerBufferSize";

            data[0] = POINTER_BUFFER_SIZE;
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }

    return KernelResult::Success;
}

}
