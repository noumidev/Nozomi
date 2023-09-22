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

#include <cstdlib>
#include <cstring>
#include <ios>
#include <map>
#include <string>

#include <plog/Log.h>

#include "memory.hpp"
#include "result.hpp"

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

union HandleDescriptor {
    u32 raw;
    struct {
        u32 sendPID : 1;
        u32 numCopyHandles : 4;
        u32 numMoveHandles : 4;
        u32 : 23;
    };
};

static_assert(sizeof(Header) == sizeof(u64));

void sendSyncRequest(const char *name, u64 ipcMessage) {
    PLOG_INFO << "Sending sync request to " << name << " (IPC message* = " << std::hex << ipcMessage << ")";

    u64 ipcSize = sizeof(u64); // Header

    Header header{.raw = sys::memory::read64(ipcMessage)};

    PLOG_VERBOSE << "IPC header = " << std::hex << header.raw;

    if (header.type == CommandType::Close) {
        PLOG_FATAL << "Unimplemented Close";

        exit(0);
    }

    if ((header.numX | header.numA | header.numB | header.numW) != 0) {
        PLOG_FATAL << "Unimplemented buffer descriptors";

        exit(0);
    }

    if (header.flagsC != 0) {
        PLOG_FATAL << "Unimplemented C buffer descriptor";

        exit(0);
    }

    if (header.hasHandleDescriptor != 0) {
        HandleDescriptor handleDescriptor{.raw = sys::memory::read32(ipcMessage + ipcSize)};
        ipcSize += sizeof(u32);

        PLOG_VERBOSE << "Handle descriptor = " << std::hex << handleDescriptor.raw;

        if (handleDescriptor.sendPID != 0) {
            PLOG_VERBOSE << "PID = " << std::hex << sys::memory::read64(ipcMessage + ipcSize);
            ipcSize += sizeof(u64);
        }

        if ((handleDescriptor.numCopyHandles | handleDescriptor.numMoveHandles) != 0) {
            PLOG_FATAL << "Unimplemented copy/move handles";

            exit(0);
        }
    }

    // Get beginning of raw data
    u64 padding = TOTAL_PADDING;
    u64 alignment = padding - (ipcSize & 15);

    if (alignment != 0) {
        ipcSize += alignment;
        padding -= alignment;
    }

    const u32 dataSize = sizeof(u32) * header.dataSize;

    // Get data payload
    u32 data[header.dataSize];
    std::memcpy(data, sys::memory::getPointer(ipcMessage + ipcSize), dataSize);

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

    // Write data payload and output to memory
    std::memcpy(sys::memory::getPointer(ipcMessage + ipcSize), data, dataSize);
    std::memcpy(sys::memory::getPointer(ipcMessage + ipcSize + sizeof(u32) * DataPayloadOffset::Output), &output[0], output.size());
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
