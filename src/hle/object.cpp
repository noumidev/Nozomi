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

#include "object.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

namespace hle {

KObject::KObject() : handle(Handle{.raw = 0}), refCount(0) {
    open();
}

KObject::~KObject() {}

Handle KObject::getHandle() {
    return this->handle;
}

void KObject::setHandle(Handle handle) {
    this->handle = handle;
}

void KObject::open() {
    refCount++;
}

bool KObject::close() {
    if (refCount == 0) {
        PLOG_FATAL << "Attempted to close KObject with refCount = 0";

        exit(0);
    }

    refCount--;

    return refCount <= 0;
}

KPort::KPort(const char *name) {
    std::strncpy(this->name, name, KPORT_NAME_LENGTH);

    if (this->name[KPORT_NAME_LENGTH - 1] != 0) {
        PLOG_FATAL << "Name too long";

        exit(0);
    }
}

KPort::~KPort() {}

const char *KPort::getName() {
    return name;
}

KSession::KSession(Handle portHandle) : portHandle(portHandle) {}

KSession::~KSession() {}

Handle KSession::getPortHandle() {
    return this->portHandle;
}

}
