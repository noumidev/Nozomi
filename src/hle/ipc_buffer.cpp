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

#include "ipc_buffer.hpp"

#include <cassert>
#include <cstdio>

#include "memory.hpp"

namespace hle {

constexpr u64 ALIGNMENT = 16;
constexpr u64 ALIGNMENT_MASK = ALIGNMENT - 1;

IPCBuffer::IPCBuffer(u64 ipcMessage) : remAlignment(ALIGNMENT) {
    ipcPointer = sys::memory::getPointer(ipcMessage);
    
    setOffset(0);
}

void *IPCBuffer::get() {
    // Cast to u8 * for pointer arithmetic
    return (void *)((u8 *)ipcPointer + offset);
}

u64 IPCBuffer::getOffset() {
    return offset;
}

void IPCBuffer::setOffset(u64 offset) {
    this->offset = offset;
}

void IPCBuffer::advance(u64 offset) {
    this->offset += offset;
}

void IPCBuffer::retire(u64 offset) {
    this->offset -= offset;
}

// Aligns IPC buffer
void IPCBuffer::align(u64 alignment) {
    assert(alignment != 0);

    const u64 alignmentMask = alignment - 1;
    if ((offset & alignmentMask) != 0) {
        offset += alignment - (offset & alignmentMask);
    }
}

// Aligns IPC data payload
void IPCBuffer::alignUp() {
    if (remAlignment == 0) { // The max amount of padding is 16 bytes
        return;
    }
    
    if (remAlignment != ALIGNMENT) { // Add remaining padding
        advance(remAlignment);

        remAlignment = 0;

        return;
    }

    const u64 alignment = offset & ALIGNMENT_MASK;
    if (alignment != 0) {
        advance(ALIGNMENT - alignment);

        remAlignment = alignment;
    } else { // Move forward by 16 bytes if offset was aligned
        advance(ALIGNMENT);

        remAlignment = 0;
    }
}

}
