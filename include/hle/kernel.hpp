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

#include <ios>

#include <plog/Log.h>

#include "handle.hpp"
#include "handle_table.hpp"
#include "object.hpp"

namespace hle::kernel {

void init();

Handle getMainThreadHandle();

Handle makeEvent(bool autoClear);
Handle makePort(const char *name);
Handle makeServiceSession(const char *name);
Handle makeSession(Handle portHandle);

template<typename T>
Handle makeService() {
    const Handle handle = table::add(HandleType::KService, new T());

    KService *service = dynamic_cast<KService *>(table::getLast());

    if (service == NULL) {
        PLOG_FATAL << "Invalid service type";

        exit(0);
    }

    service->setHandle(handle);

    PLOG_DEBUG << "Making KService (name = " << service->getName() << ", handle = " << std::hex << service->getHandle().raw << ")";

    return handle;
}

void destroyServiceSession(Handle handle);
void destroySession(Handle handle);

void closeHandle(Handle handle);
Handle copyHandle(Handle handle);

KObject *getObject(Handle handle);

KPort *getPort(const char *name);

}
