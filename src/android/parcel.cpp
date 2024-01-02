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

#include "parcel.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

namespace android::parcel {

struct ParcelHeader {
    u32 payloadSize, payloadOffset;
    u32 objectsSize, objectsOffset;
} __attribute__((packed));

static_assert(sizeof(ParcelHeader) == (4 * sizeof(u32)));

Parcel::Parcel() : payloadReadPointer(0) {}

Parcel::~Parcel() {}

void Parcel::alignUp(u32 alignment) {
    if ((payloadReadPointer & (alignment - 1)) != 0) {
        payloadReadPointer |= (alignment - 1);
        payloadReadPointer += 1;
    }
}

void Parcel::writeObject(const std::vector<u8> &data) {
    payload.insert(payload.end(), data.begin(), data.end());

    for (int i = 0; i < 4; i++) {
        objects.push_back(0);
    }
}

std::vector<u8> Parcel::serialize() {
    std::vector<u8> data;

    // Write parcel header
    ParcelHeader header;
    header.payloadSize = payload.size();
    header.payloadOffset = sizeof(ParcelHeader);
    header.objectsSize = objects.size();
    header.objectsOffset = sizeof(ParcelHeader) + header.payloadSize;

    data.insert(data.end(), (u8 *)&header, (u8 *)&header + sizeof(ParcelHeader));

    // Write payload & objects
    data.insert(data.end(), payload.begin(), payload.end());
    data.insert(data.end(), objects.begin(), objects.end());

    return data;
}

void Parcel::deserialize(const std::vector<u8> &data) {
    // Read parcel header
    ParcelHeader header;

    if (data.size() < sizeof(ParcelHeader)) {
        PLOG_FATAL << "Invalid parcel header";

        exit(0);
    }

    std::memcpy(&header, data.data(), sizeof(ParcelHeader));

    if (data.size() != (sizeof(ParcelHeader) + header.payloadSize + header.objectsSize)) {
        PLOG_FATAL << "Invalid parcel size";

        exit(0);
    }

    PLOG_VERBOSE << "Payload (size = " << header.payloadSize << ", offset = " << header.payloadOffset << "), Objects (size = " << header.objectsSize << ", offset = " << header.objectsOffset << ")";

    payload.resize(header.payloadSize);
    objects.resize(header.objectsSize);

    std::memcpy(payload.data(), &data[header.payloadOffset], header.payloadSize);
    std::memcpy(objects.data(), &data[header.objectsOffset], header.objectsSize);

    // ??
    (void)read<u32>();

    // Read interface token
    const u32 length = read<u32>();

    char interfaceToken[length + 1];
    std::memset(interfaceToken, 0, sizeof(interfaceToken));

    for (u32 i = 0; i < length; i++) {
        interfaceToken[i] = (char)read<u16>();
    }

    PLOG_VERBOSE << "Interface token = " << (const char *)interfaceToken;

    alignUp(sizeof(u32));

    bufferIndex = read<u32>();

    PLOG_VERBOSE << "Buffer index = " << bufferIndex;
}

}
