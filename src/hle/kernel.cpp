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
#include <utility>

#include <plog/Log.h>

#include "object.hpp"

namespace hle::kernel {

std::vector<KObject> kernelObjects;

std::map<Handle, KObject *> kernelObjectMap;

void init() {
    // Make two dummy objects (0 = invalid handle, 1 = main thread handle)
    (void)makeDummy();
    (void)makeDummy();

    (void)makeSession(makePort("sm:"));
}

Handle getNextHandle() {
    static Handle handles = 0;

    return handles++;
}

Handle getMainThreadHandle() {
    return 1;
}

KObject *getLastObject() {
    return &kernelObjects[kernelObjects.size() - 1];
}

Handle makeDummy() {
    kernelObjects.emplace_back(KDummy());

    KDummy *dummy = (KDummy *)getLastObject();

    const Handle handle = dummy->getHandle();

    kernelObjectMap.insert(std::pair<Handle, KObject *>{handle, dummy});

    PLOG_DEBUG << "Making KDummy (handle = " << std::hex << handle << ")";

    return handle;
}

Handle makePort(const char *name) {
    kernelObjects.emplace_back(KPort(name));

    KPort *port = (KPort *)getLastObject();

    const Handle handle = port->getHandle();

    kernelObjectMap.insert(std::pair<Handle, KObject *>{handle, port});

    PLOG_DEBUG << "Making KPort (name = " << name << ", handle = " << std::hex << handle << ")";

    return handle;
}

Handle makeSession(Handle portHandle) {
    kernelObjects.emplace_back(KSession(portHandle));

    KSession *session = (KSession *)getLastObject();

    const Handle handle = session->getHandle();

    kernelObjectMap.insert(std::pair<Handle, KObject *>{handle, session});

    PLOG_DEBUG << "Making KSession (port handle = " << std::hex << portHandle << ", handle = " << handle << ")";

    return handle;
}

}

