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

#include "kernel.hpp"

#include <ios>
#include <map>
#include <type_traits>
#include <utility>

#include <plog/Log.h>

#include "handle_table.hpp"

namespace hle::kernel {

void init() {
    table::init();

    (void)makePort("sm:");
}

Handle getMainThreadHandle() {
    // TODO: return actual main thread handle
    return Handle{.index = 1, .type = HandleType::KThread};
}

Handle makePort(const char *name) {
    const Handle handle = table::add(HandleType::KPort, new KPort(name));

    ((KPort *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KPort (name = " << name << ", handle = " << std::hex << handle.raw << ")";

    return handle;
}

Handle makeSession(Handle portHandle) {
    const Handle handle = table::add(HandleType::KSession, new KSession(portHandle));

    ((KSession *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KSession (port handle = " << std::hex << portHandle.raw << ", handle = " << handle.raw << ")";

    return handle;
}

KPort *getPort(const char *name) {
    PLOG_DEBUG << "Searching port " << name;

    KPort *port = table::getPort(name);

    if (port != NULL) {
        return port;
    }

    PLOG_FATAL << "Unable to find port " << name;

    exit(0);
}

}

