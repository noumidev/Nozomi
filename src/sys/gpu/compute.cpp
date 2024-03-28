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

#include "compute.hpp"

#include <array>
#include <cstdlib>
#include <ios>

#include <plog/Log.h>

namespace sys::gpu::compute {

constexpr bool ENABLE_WRITE_LOG = true;

constexpr u32 NUM_REGS = 0x1000;

std::array<u32, NUM_REGS> regs;

void write(u32 addr, u32 data) {
    if (addr > NUM_REGS) {
        PLOG_FATAL << "Invalid register address " << std::hex << addr;

        exit(0);
    }

    regs[addr] = data;

    if constexpr (ENABLE_WRITE_LOG) {
        switch (addr) {
            default:
                PLOG_WARNING << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                break;
        }
    }
}

}
