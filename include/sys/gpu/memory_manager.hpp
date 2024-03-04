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

#include "types.hpp"

namespace sys::gpu::memory_manager {

constexpr u64 GPU_ADDRESS_SPACE = 1LLU << 40;

void *getPage(u64 iova);

u8 read8(u64 iova);
u16 read16(u64 iova);
u32 read32(u64 iova);
u64 read64(u64 iova);

void write8(u64 iova, u8 data);
void write16(u64 iova, u16 data);
void write32(u64 iova, u32 data);
void write64(u64 iova, u64 data);

void map(u64 iova, u64 address, u64 size, u64 align);

}
