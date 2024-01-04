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

#include "nvmap.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "memory.hpp"
#include "nvfile.hpp"

namespace nvidia::nvmap {

namespace IOC {
    enum : u32 {
        Create = 0xC0080101,
    };
}

struct NVMAP {
    u64 size;
};

std::vector<NVMAP> nvmapObjects;

i32 create(IPCContext &ctx) {
    u64 size;
    std::memcpy(&size, ctx.readSend().data(), sizeof(size));

    PLOG_VERBOSE << "CREATE (size = " << std::hex << size << ")";

    if (!sys::memory::isAligned(size)) {
        PLOG_ERROR << "Size is not aligned";

        exit(0);
    }

    nvmapObjects.emplace_back(NVMAP{.size = size});

    const u64 handle = (u64)nvmapObjects.size() - 1;

    std::vector<u8> h;
    h.resize(sizeof(u64));
    std::memcpy(h.data(), &handle, sizeof(handle));

    ctx.writeReceive(h);

    return NVResult::Success;
}

i32 ioctl(u32 iocode, IPCContext &ctx) {
    switch (iocode) {
        case IOC::Create:
            return create(ctx);
        default:
            PLOG_FATAL << "Unimplemented ioctl (iocode = " << std::hex << iocode << ")";

            exit(0);
    }
}

}
