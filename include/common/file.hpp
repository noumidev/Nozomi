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

#include <cstdio>
#include <vector>

inline std::vector<char> readFile(const char *path) {
    FILE *file = std::fopen(path, "rb");

    if (file == NULL) {
        return std::vector<char>(0);
    }

    std::fseek(file, 0, SEEK_END);
    const size_t size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    std::vector<char> data(size);
    std::fread(data.data(), sizeof(char), size, file);

    std::fclose(file);

    return data;
}
