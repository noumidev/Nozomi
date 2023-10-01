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

#include <cstring>

#include "types.hpp"

namespace hle {

struct IPCBuffer {
    IPCBuffer(u64 ipcMessage);

    void *get();

    u64 getOffset();

    void setOffset(u64 offset);

    void advance(u64 offset);
    void retire(u64 offset);

    void alignUp();

    template<typename T>
    T read() {
        T data;
        std::memcpy(&data, get(), sizeof(T));

        advance(sizeof(T));

        return data;
    }

    template<typename T>
    void write(T data) {
        std::memcpy(get(), &data, sizeof(T));

        advance(sizeof(T));
    }

private:
    void *ipcPointer;

    u64 offset, remAlignment;
};

}
