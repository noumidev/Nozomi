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

#include "emulator.hpp"

#include "cpu.hpp"
#include "kernel.hpp"
#include "loader.hpp"
#include "memory.hpp"

namespace sys::emulator {

constexpr u64 CYCLES_PER_FRAME = cpu::CPU_CLOCK / 60;

void init(const char *path) {
    cpu::init();
    hle::kernel::init();

    // Load executable
    loader::load(path);

    // Set up stack and thread-local storage (this will be replaced later on)
    (void)memory::allocate(memory::MemoryBase::Stack, 1, 0, 0, memory::MemoryPermission::RW);
    (void)memory::allocate(memory::MemoryBase::TLSBase, 1, 0, 0, memory::MemoryPermission::RW);
}

void run() {
    while (true) {
        cpu::run(CYCLES_PER_FRAME);
        cpu::addTicks(CYCLES_PER_FRAME);
    }
}

}
