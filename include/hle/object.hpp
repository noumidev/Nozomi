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

namespace hle {

constexpr int KPORT_NAME_LENGTH = 16;

// Kernel object base class
class KObject {
protected:
    Handle handle;

    int refCount;

public:
    KObject();
    virtual ~KObject();

    Handle getHandle();

    void setHandle(Handle handle);

    void open();
    bool close();
};

class KPort : public KObject {
    char name[KPORT_NAME_LENGTH];

public:
    KPort(const char *name);
    ~KPort();

    const char *getName();
};

class KSession : public KObject {
    Handle portHandle;

public:
    KSession(Handle portHandle);
    ~KSession();

    Handle getPortHandle();
};

}