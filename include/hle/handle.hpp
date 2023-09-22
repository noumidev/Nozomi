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

union Handle {
    u32 raw;
    struct {
        u32 index : 20; // Index into handle table
        u32 type : 8;
        u32 : 4;
    };
};

namespace HandleType {
    enum : u32 {
        None,
        KPort,
        KServiceSession,
        KSession,
        KThread, // Does not exist yet
        NumHandleTypes,
    };
}

namespace KernelHandles {
    enum : u32 {
        CurrentThread = 0xFFFF8000,
        CurrentProcess = 0xFFFF8001,
    };
}

inline Handle makeHandle(u32 n) {
    return Handle{.raw = n};
}

inline Handle makeHandle(u32 index, u32 type) {
    return Handle{.index = index, .type = type};
}

}
