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

#pragma once

#include "types.hpp"

namespace hle {

namespace KernelResult {
    enum : Result {
        Success = 0,
        NoDataInChannel = 0x480,
        NoAppletMessages = 0x680,
        PortSdCardNoDevice = 0xFA202,
    };
}

namespace Module {
    enum : u32 {
        Kernel = 1,
        Libnx = 345,
        HomebrewAbi,
        HomebrewLoader,
        LibnxNvidia,
        LibnxBinder,
    };
}

inline const char *getModuleName(u32 module) {
    switch (module) {
        case Module::Kernel:
            return "Kernel";
        case Module::Libnx:
            return "Libnx";
        case Module::HomebrewAbi:
            return "HomebrewAbi";
        case Module::HomebrewLoader:
            return "HomebrewLoader";
        case Module::LibnxNvidia:
            return "LibnxNvidia";
        case Module::LibnxBinder:
            return "LibnxBinder";
        default:
            exit(0);
    }
}

inline u32 getModule(Result result) {
    return result & 0x1FF;
}

inline u32 getDescription(Result result) {
    return (result >> 9) & 0x1FFF;
}

}
