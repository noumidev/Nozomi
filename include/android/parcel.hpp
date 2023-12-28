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

#include "types.hpp"

namespace android::parcel {

class Parcel {
    std::vector<u8> payload, objects;

public:
    Parcel() {}
    ~Parcel() {}

    void writeObject(const std::vector<u8> &data) {
        payload.insert(payload.end(), data.begin(), data.end());

        for (int i = 0; i < 4; i++) {
            objects.push_back(0);
        }
    }

    std::vector<u8> serialize() {
        std::vector<u8> data;

        // Write parcel header
        u32 header[4];

        header[0] = (u32)payload.size();               // Payload size
        header[1] = (u32)sizeof(header);               // Payload offset (always 0x10)
        header[2] = (u32)objects.size();               // Objects size
        header[3] = (u32)(sizeof(header) + header[0]); // Objects offset

        data.insert(data.end(), (u8 *)header, (u8 *)&header[4]);

        // Write payload & objects
        data.insert(data.end(), payload.begin(), payload.end());
        data.insert(data.end(), objects.begin(), objects.end());

        return data;
    }
};

}
