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

#include "host1x.hpp"

#include <array>
#include <cstdlib>

#include <plog/Log.h>

namespace nvidia::host1x {

// Value taken from Yuzu
constexpr u32 MAX_SYNCPOINTS = 192;

constexpr u32 ID_OFFSET = 1024;

std::array<NVFence, MAX_SYNCPOINTS> syncpoints;

void init() {
    for (NVFence &syncpoint : syncpoints) {
        syncpoint.id = NO_SYNCPOINT;
        syncpoint.value = 0;
    }
}

u32 findFreeFence() {
    for (u32 i = 0; i < MAX_SYNCPOINTS; i++) {
        const NVFence &syncpoint = syncpoints[i];
        
        if (syncpoint.id == NO_SYNCPOINT) {
            return i;
        }
    }

    PLOG_FATAL << "Unable to find free fence";
    
    exit(0);
}

NVFence makeFence() {
    const u32 id = findFreeFence();

    PLOG_INFO << "Creating fence with ID " << id;

    NVFence *fence = &syncpoints[id];

    // "Allocate" fence
    fence->id = id + ID_OFFSET;

    return *fence;
}

}
