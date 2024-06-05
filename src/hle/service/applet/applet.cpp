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

#include "applet.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>

#include <plog/Log.h>

#include "error_applet.hpp"

namespace hle::service::applet {

constexpr u64 COMMON_ARGS_SIZE = 0x20;

struct CommonArguments {
    u32 version;
    u32 size;
    u32 apiVersion;
    u32 padding[3];
    u64 systemTick;
} __attribute__((packed));

static_assert(sizeof(CommonArguments) == COMMON_ARGS_SIZE);

Applet::Applet() : pushCount(0) {}

Applet::~Applet() {}

void Applet::parseCommonArgs(const std::vector<u8> &data) {
    if (data.size() != COMMON_ARGS_SIZE) {
        PLOG_FATAL << "Invalid Common Arguments size";

        exit(0);
    }

    CommonArguments commonArgs;
    std::memcpy(&commonArgs, data.data(), COMMON_ARGS_SIZE);

    PLOG_INFO << "Common arguments (version = " << commonArgs.version << ", size = " << std::hex << commonArgs.size << ", API version = " << std::dec << commonArgs.apiVersion << ", system tick = " << commonArgs.systemTick << ")";
}

void Applet::pushInDataBase(const std::vector<u8> &data) {
    pushCount++;

    if (pushCount == 1) {
        return parseCommonArgs(data);
    }

    pushInData(data);
}

Applet makeApplet(u32 appletID) {
    PLOG_INFO << "Creating applet (applet ID = " << std::hex << appletID << ")";

    Applet applet;
    switch (appletID) {
        case AppletID::ErrorApplet:
            applet.pushInData = &error_applet::pushInData;
            break;
        default:
            PLOG_FATAL << "Unrecognized applet ID " << std::hex << appletID;

            exit(0);
    }

    return applet;
}

}
