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

#include "memory.hpp"

namespace hle {

IPCBuffer::IPCBuffer(u64 ipcMessage) {
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

}
