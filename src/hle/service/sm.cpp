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

#include "sm.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <map>

#include <plog/Log.h>

#include "handle.hpp"
#include "ipc_manager.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::sm {

static std::map<u64, const char *> serviceNameMap {
    {0x7672732d707366, "fsp-srv"},
};

namespace Command {
    enum : u32 {
        GetServiceHandle = 1,
    };
}

const char *getServiceName(u64 service) {
    const auto serviceName = serviceNameMap.find(service);
    if (serviceName == serviceNameMap.end()) {
        PLOG_FATAL << "Unknown service " << std::hex << service;

        exit(0);
    }

    return serviceName->second;
}

void handleRequest(u32 command, u32 *data, std::vector<u8> &output) {
    switch (command) {
        case Command::GetServiceHandle:
            cmdGetServiceHandle(data, output);
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdGetServiceHandle(u32 *data, std::vector<u8> &output) {
    (void)output;

    u64 service;
    std::memcpy(&service, &data[ipc::DataPayloadOffset::Parameters], sizeof(u64));

    // Get service name
    const char *serviceName = getServiceName(service);

    PLOG_INFO << "GetServiceHandle (service = " << serviceName << ")";

    const Handle handle = kernel::makeServiceSession(serviceName);

    output.resize(sizeof(Handle));
    std::memcpy(output.data(), &handle.raw, sizeof(Handle));

    data[ipc::DataPayloadOffset::Result] = Result::Success;
}

}
