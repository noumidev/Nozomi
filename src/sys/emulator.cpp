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
#include "host1x.hpp"
#include "kernel.hpp"
#include "loader.hpp"
#include "memory.hpp"
#include "nvflinger.hpp"
#include "nvhost_gpu.hpp"
#include "object.hpp"
#include "renderer.hpp"
#include "window.hpp"

namespace sys::emulator {

constexpr u64 CYCLES_PER_FRAME = cpu::CPU_CLOCK / 60;

using hle::Handle;

void init(const char *path) {
    renderer::window::init();
    renderer::init();

    cpu::init();
    hle::kernel::init();
    nvidia::host1x::init();
    nvidia::nvflinger::init();
    nvidia::channel::nvhost_gpu::init();

    // Create main thread
    // NOTE: initial values don't matter; loader sets CPU registers directly
    const Handle mainThreadHandle = hle::kernel::makeThread(memory::MemoryBase::Application, 0, memory::MemoryBase::Stack + memory::STACK_PAGES * memory::PAGE_SIZE, 0, 0);

    hle::kernel::setMainThreadHandle(mainThreadHandle);
    hle::kernel::startThread(mainThreadHandle);

    // Load executable
    loader::load(path);

    // Set up stack
    (void)memory::allocate(memory::MemoryBase::Stack, memory::STACK_PAGES, 0, 0, memory::MemoryPermission::RW);
}

void run() {
    while (!renderer::window::shouldQuit()) {
        cpu::run(CYCLES_PER_FRAME);
        cpu::addTicks(CYCLES_PER_FRAME);

        renderer::window::pollEvents();
        renderer::draw();
    }

    renderer::waitIdle();

    renderer::deinit();
    renderer::window::deinit();
}

void update(u8 *fb) {
    (void)fb;
}

}
