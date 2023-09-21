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

#include "common/types.hpp"

namespace sys::cpu {

constexpr u64 CPU_CLOCK = 1020000000;

void init();

void run(u64 ticks);

void addTicks(u64 ticks);

u64 get(int idx);
u64 getTLSAddr();

void set(int idx, u64 data);
void setTLSAddr(u64 addr);

}
