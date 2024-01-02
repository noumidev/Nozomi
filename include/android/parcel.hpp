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

#include <vector>

#include <plog/Log.h>

#include "types.hpp"

namespace android::parcel {

class Parcel {
    std::vector<u8> payload, objects;
    u32 payloadPointer;

    u32 bufferIndex;

public:
    Parcel();
    ~Parcel();

    void alignUp(u32 alignment);

    template<typename T>
    T read() {
        T data;

        if (payload.size() < (payloadPointer + sizeof(T))) {
            PLOG_FATAL << "Out of bounds payload read";

            exit(0);
        }

        std::memcpy(&data, &payload[payloadPointer], sizeof(T));

        payloadPointer += sizeof(T);

        return data;
    }

    // Intended to be used for output parcels
    template<typename T>
    void write(T data) {
        payload.resize(payload.size() + sizeof(T));

        std::memcpy(&payload[payloadPointer], &data, sizeof(T));

        payloadPointer += sizeof(T);
    }

    void writeObject(const std::vector<u8> &data);

    std::vector<u8> serialize();

    void deserialize(const std::vector<u8> &data);
};

}
