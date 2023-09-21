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

#include "loader.hpp"

#include <cstdio>
#include <cstdlib>

#include <plog/Log.h>

#include "nro.hpp"
#include "sys/cpu.hpp"
#include "sys/memory.hpp"

namespace loader {

void load(const char *path) {
    PLOG_VERBOSE << "Input file: " << path;

    FILE *file = std::fopen(path, "rb");

    // Check if file exists
    if (file == NULL) {
        PLOG_FATAL << "Unable to open file " << path;

        exit(0);
    }

    // Go through all supported file formats to find which loader to choose
    if (nro::isNRO(file)) {
        PLOG_VERBOSE << "Input file is NRO";

        nro::load(file);
        nro::makeHomebrewEnv();

        sys::cpu::set(0, sys::memory::MemoryBase::HomebrewEnv);
        sys::cpu::set(1, -1LL);
    } else {
        PLOG_FATAL << "Unrecognized executable format";

        exit(0);
    }

    std::fclose(file);
}

}
