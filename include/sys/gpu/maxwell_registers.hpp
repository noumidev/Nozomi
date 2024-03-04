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

namespace sys::gpu::maxwell {

namespace Register {
    enum : u32 {
        SetObject,
        SetSmTimeoutInterval = 0x379,
        SetCtSelect = 0x487,
        SetAntiAliasEnable = 0x54D,
        SetRenderEnableA = 0x554,
        SetRenderEnableB,
        SetRenderEnableC,
        SetCsaa = 0x56D,
        SetZCompression = 0x673,
        SetColorCompression = 0x678,
        SetCtWrite = 0x680,
    };
}

}
