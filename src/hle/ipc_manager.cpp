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
#include "kernel.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "result.hpp"

#include "set_sys.hpp"
#include "sm.hpp"

namespace hle::ipc {

constexpr u32 makeMagic(const char *magic) {
    return (u32)magic[0] | ((u32)magic[1] << 8) | ((u32)magic[2] << 16) | ((u32)magic[3] << 24);
}

constexpr u16 POINTER_BUFFER_SIZE = 8;
constexpr u64 TOTAL_PADDING = 16;

constexpr u32 INPUT_HEADER_MAGIC = makeMagic("SFCI");
constexpr u32 OUTPUT_HEADER_MAGIC = makeMagic("SFCO");

static std::map<std::string, void (*)(u32, u32 *, std::vector<u8> &)> requestFuncMap {
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

void sendSyncRequest(Handle handle, u64 ipcMessage) {
    // Get service name from handle
    const char *name;
    switch (handle.type) {
        case HandleType::KServiceSession:
            name = ((KServiceSession *)kernel::getObject(handle))->getName();
            break;
        case HandleType::KSession:
            name = ((KPort *)kernel::getObject(((KSession *)kernel::getObject(handle))->getPortHandle()))->getName();
            break;
        default:
            PLOG_FATAL << "Unimplemented handle type " << handle.type;

            exit(0);
    }

    PLOG_INFO << "Sending sync request to " << name << " (IPC message* = " << std::hex << ipcMessage << ")";

    IPCBuffer ipcBuffer(ipcMessage);

    Header header{.raw = ipcBuffer.read<u64>()};

    PLOG_VERBOSE << "IPC header = " << std::hex << header.raw;

    if (header.type == CommandType::Close) {
        PLOG_INFO << "Closing service session (handle = " << std::hex << handle.raw << ")";
        
        return;
    }

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

        if ((handleDescriptor.numCopyHandles | handleDescriptor.numMoveHandles) != 0) {
            PLOG_FATAL << "Unimplemented copy/move handles";

            exit(0);
        }
    }

    // Get beginning of raw data
    u64 padding = TOTAL_PADDING;
    u64 alignment = padding - (ipcBuffer.getOffset() & 15);

    if (alignment != 0) {
        ipcBuffer.advance(alignment);

        padding -= alignment;
    }

    const u32 dataSize = sizeof(u32) * header.dataSize;

    // Get data payload
    u32 data[header.dataSize];
    std::memcpy(data, ipcBuffer.get(), dataSize);

    // Confirm input header and write output header
    if (data[DataPayloadOffset::Magic] != INPUT_HEADER_MAGIC) {
        PLOG_FATAL << "Invalid data payload header";

        exit(0);
    }

    // Service writes result
    data[DataPayloadOffset::Magic] = OUTPUT_HEADER_MAGIC;
    data[DataPayloadOffset::Version] = 0;

    std::vector<u8> output;

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
                
                requestFunc->second(data[DataPayloadOffset::Command], data, output);
            }
            break;
        case CommandType::Control:
            handleControl(data[DataPayloadOffset::Command], data, output);
            break;
        default:
            PLOG_FATAL << "Unimplemented IPC type " << header.type;

            exit(0);
    }

    // Write data payload to memory
    std::memcpy(ipcBuffer.get(), data, dataSize);

    // Skip over data payload
    ipcBuffer.advance(dataSize + padding);

    // Write output to memory
    switch (header.flagsC) {
        case 0: // No C buffer (data goes after header??)
            ipcBuffer.setOffset(sizeof(Header));
        case 1: // Inlined C buffer
            std::memcpy(ipcBuffer.get(), &output[0], output.size());
            return;
        case 2: // ?
            PLOG_FATAL << "No C buffers to write output to";

            exit(0);
        default:
            break;
    }

    const u64 numC = header.flagsC - 2;

    PLOG_VERBOSE << "Number of C buffers = " << numC;
    
    CBufferDescriptor descriptors[numC];

    // Note: if I follow the information in https://switchbrew.org/wiki/IPC_Marshalling,
    // the C buffer descriptors are all empty because they reside in the data payload (which gets skipped over).
    // TODO: figure out how to get it working without doing this:
    ipcBuffer.retire(16);

    for (u64 i = 0; i < numC; i++) {
        auto &descriptor = descriptors[i];

        descriptor = CBufferDescriptor{.raw = ipcBuffer.read<u64>()};

        PLOG_VERBOSE << "C buffer descriptor (num = " << i << ", addr = " << std::hex << descriptor.address << ", size = " << descriptor.size << ")";
    }

    const auto &descriptor = descriptors[0];
    
    std::memcpy(sys::memory::getPointer(descriptor.address), &output[0], std::min(descriptor.size, (u64)output.size()));
}

void handleControl(u32 command, u32 *data, std::vector<u8> &output) {
    (void)data;
    (void)output;

    switch (command) {
        case Command::QueryPointerBufferSize:
            PLOG_INFO << "QueryPointerBufferSize";

            output.resize(sizeof(u16));
            std::memcpy(&output[0], &POINTER_BUFFER_SIZE, sizeof(u16));
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }

    data[DataPayloadOffset::Result] = Result::Success;
}

}
