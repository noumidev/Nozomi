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

#include "handle_table.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

namespace hle::kernel::table {

constexpr u32 FIRST_HANDLE = 2;
constexpr u32 MAX_HANDLES = 1 << 20;

struct TableEntry {
    u32 type;

    KObject *object;
};

TableEntry handleTable[MAX_HANDLES];

u32 nextIndex = FIRST_HANDLE;
u32 handleCount = 0;

u32 getNextIndex() {
    if (nextIndex == MAX_HANDLES) {
        PLOG_FATAL << "Handle table overflowed";

        exit(0);
    }

    return nextIndex++;
}

u32 getPrevIndex() {
    return nextIndex - 1;
}

void init() {
    for (auto &entry : handleTable) {
        entry.type = HandleType::None;

        entry.object = NULL;
    }
}

Handle add(u32 type, KObject *object) {
    if ((type == HandleType::None) || (type >= HandleType::NumHandleTypes)) {
        PLOG_FATAL << "Invalid type";

        exit(0);
    }

    if (object == NULL) {
        PLOG_FATAL << "Trying to add invalid object";
    }

    Handle handle{.index = getNextIndex(), .type = type};

    handleTable[handle.index].type = type;
    handleTable[handle.index].object = object;

    handleCount++;

    return handle;
}

KObject *get(Handle handle) {
    if (handle.index >= nextIndex) {
        PLOG_FATAL << "Invalid index";

        exit(0);
    }

    const TableEntry *entry = &handleTable[handle.index];

    if (entry->type != handle.type) {
        PLOG_FATAL << "Object type mismatch";

        exit(0);
    }

    KObject *object = entry->object;

    if (object == NULL) {
        PLOG_FATAL << "Invalid object";

        exit(0);
    }
    
    return object;
}

KObject *getLast() {
    KObject *object = handleTable[getPrevIndex()].object;

    if (object == NULL) {
        PLOG_FATAL << "Invalid object";

        exit(0);
    }

    return object;
}

KPort *getPort(const char *name) {
    for (u32 i = 0; i < handleCount; i++) {
        TableEntry *entry = &handleTable[FIRST_HANDLE + i];

        if (entry->type != HandleType::KPort) {
            continue;
        }

        KPort *port = (KPort *)entry->object;

        if (std::strcmp(name, port->getName()) == 0) {
            return port;
        }
    }

    return NULL;
}

}
