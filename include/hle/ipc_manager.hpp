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

#include "handle.hpp"
#include "types.hpp"

namespace hle::ipc {

namespace DataPayloadOffset {
    enum : u32 {
        Magic = 0,
        Version = 1,
        Command = 2, // Input
        Result = 2, // Output
        Padding = 3,
        Parameters = 4,
        Output = 4,
    };
}

void sendSyncRequest(Handle handle, u64 ipcMessage);

Result handleControl(u32 command, u32 *data);

}
