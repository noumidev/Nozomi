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

#include "nro.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

namespace loader::nro {

constexpr const char *NRO_MAGIC = "NRO0";

namespace HeaderOffset {
    enum {
        Magic = 0x10,
    };
}

namespace HeaderFieldSize {
    enum {
        Magic = 4,
    };
}

void load(FILE *file) {
    (void)file;

    PLOG_FATAL << "NRO loading not implemented";

    exit(0);
}

bool isNRO(FILE *file) {
    // Load magic from file, compare to NRO_MAGIC
    std::fseek(file, HeaderOffset::Magic, SEEK_SET);

    char magic[HeaderFieldSize::Magic];
    std::fread(magic, sizeof(char), sizeof(magic), file);

    return std::strncmp(magic, NRO_MAGIC, sizeof(magic)) == 0;
}

}
