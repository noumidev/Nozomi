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

#include "set_sys.hpp"

#include <cstdlib>
#include <cstring>
#include <vector>

#include <plog/Log.h>

#include "result.hpp"

namespace hle::service::set_sys {

// Values taken from https://switchbrew.org/wiki/System_Version_Title
constexpr const char PLATFORM_STRING[] = "NX";
constexpr const char VERSION_HASH[] = "9b87ee6cd509f49e7df100cae8b31bdcf628ebcb";
constexpr const char DISPLAY_VERSION[] = "13.0.0";
constexpr const char DISPLAY_TITLE[] = "NintendoSDK Firmware for NX 13.0.0-4.0";

namespace Command {
    enum : u32 {
        GetFirmwareVersion = 3,
    };
}

namespace FirmwareVersion {
    enum : u8 {
        Major = 13,
        Minor = 0,
        Micro = 0,
        RevisionMajor = 4,
        RevisionMinor = 0,
    };
}

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::GetFirmwareVersion:
            return cmdGetFirmwareVersion(ctx, reply);
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdGetFirmwareVersion(IPCContext &ctx, IPCContext &reply) {
    PLOG_INFO << "GetFirmwareVersion";

    std::vector<u8> output;
    output.resize(0x100);

    output[0] = FirmwareVersion::Major;
    output[1] = FirmwareVersion::Minor;
    output[2] = FirmwareVersion::Micro;
    output[4] = FirmwareVersion::RevisionMajor;
    output[5] = FirmwareVersion::RevisionMinor;

    std::strcpy((char *)&output[8], PLATFORM_STRING);
    std::strcpy((char *)&output[0x28], VERSION_HASH);
    std::strcpy((char *)&output[0x68], DISPLAY_VERSION);
    std::strcpy((char *)&output[0x80], DISPLAY_TITLE);

    ctx.writeReceive(output);

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

}
