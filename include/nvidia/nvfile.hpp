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

#include "ipc.hpp"
#include "types.hpp"

namespace nvidia {

using hle::IPCContext;

using FileDescriptor = i32;

namespace NVResult {
    enum : i32 {
        Success = 0,
        Timeout = 5,
    };
}

constexpr FileDescriptor NO_FD = -1;

class NVFile {
    bool isOpen;

    FileDescriptor fd;

public:
    NVFile(FileDescriptor fd);
    ~NVFile();

    i32 (*ioctl)(u32 iocode, IPCContext &ctx);

    void open();
    void close();

    bool isClosed();

    FileDescriptor getFD();
};

}
