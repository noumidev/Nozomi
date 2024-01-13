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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "types.hpp"

namespace hle {

constexpr u64 IPC_BUFFER_SIZE = 0x100;
constexpr u64 MAX_DATA_ALIGNMENT = 16;

inline constexpr u32 makeMagic(const char *magic) {
    return (u32)magic[0] | ((u32)magic[1] << 8) | ((u32)magic[2] << 16) | ((u32)magic[3] << 24);
}

constexpr u32 INPUT_HEADER_MAGIC = makeMagic("SFCI");
constexpr u32 OUTPUT_HEADER_MAGIC = makeMagic("SFCO");

namespace PointerBuffer {
    enum {
        X, A, B, W, C,
        NumPointerBuffers,
    };
}

namespace CommandType {
    enum : u32 {
        Invalid,
        LegacyRequest,
        Close,
        LegacyControl,
        Request,
        Control,
        RequestWithContext,
        ControlWithContext,
    };
}

namespace DomainCommand {
    enum : u64 {
        SendMessage = 1,
        CloseVirtualHandle,
    };
}

union Header {
    u64 raw;
    struct {
        u64 type : 16;
        u64 numX : 4;
        u64 numA : 4;
        u64 numB : 4;
        u64 numW : 4;
        u64 dataSize : 10;
        u64 flagsC : 4;
        u64 : 17;
        u64 hasHandleDescriptor : 1;
    };
};

static_assert(sizeof(Header) == sizeof(u64));

union HandleDescriptor {
    u32 raw;
    struct {
        u32 sendPID : 1;
        u32 numCopyHandles : 4;
        u32 numMoveHandles : 4;
        u32 : 23;
    };
};

static_assert(sizeof(HandleDescriptor) == sizeof(u32));

union DomainHeader {
    u64 raw;
    struct {
        u64 command : 8;
        u64 numInput : 8;
        u64 dataPayloadLength : 16;
        u64 objectID : 32;
    };
};

static_assert(sizeof(DomainHeader) == sizeof(u64));

struct DataPayloadHeader {
    u32 magic;
    u32 version;
    u32 command;
};

struct BufferDescriptor {
    u64 address, size;
};

struct XBufferDescriptor {
    u64 raw;

    u64 getAddress() {
        return (raw >> 32) | ((raw & 0xF000) << 20) | ((raw & 0x1C0) << 30);
    }

    u64 getSize() {
        return (raw >> 16) & 0xFFFF;
    }
};

struct ABWBufferDescriptor {
    u32 raw[3];

    // This is actually so bad
    u64 getAddress() {
        const u64 mid = ((u64)raw[2] >> 28) & 15;
        const u64 top = ((u64)raw[2] >> 2) & 7;

        return (u64)raw[1] | (mid << 32) | (top << 36);
    }

    u32 getFlags() {
        return raw[2] & 3;
    }

    u64 getSize() {
        const u64 top = ((u64)raw[2] >> 24) & 15;

        return (u64)raw[0] | (top << 32);
    }
};

union CBufferDescriptor {
    u64 raw;
    struct {
        u64 address : 48;
        u64 size : 16;
    };
};

static_assert(sizeof(CBufferDescriptor) == sizeof(u64));

class IPCContext {
    void *ipcPointer;

    // Service object
    KObject *service;

    // Raw IPC buffer
    u8 ipcBuffer[IPC_BUFFER_SIZE];
    u64 offset;

    u64 dataAlignment;

    Header header;
    HandleDescriptor handleDescriptor;
    DomainHeader domainHeader;
    DataPayloadHeader dataPayloadHeader;

    u32 numDomainObjects;

    // Handle descriptor area
    u32 pid;
    std::vector<Handle> copyHandles, moveHandles;

    std::vector<BufferDescriptor> bufferDescriptors[PointerBuffer::NumPointerBuffers];
    std::vector<CBufferDescriptor> receiveDescriptors;

    // Offsets
    u64 pointerDescriptorOffset[PointerBuffer::NumPointerBuffers];
    u64 copyHandlesOffset, moveHandlesOffset, domainHeaderOffset, dataPayloadOffset;

    u64 getOffset() {
        return offset;
    }

    void setOffset(u64 offset) {
        this->offset = offset;
    }

    void advance(u64 offset) {
        this->offset += offset;
    }

    void alignData() {
        if (dataAlignment == 0) {
            return;
        }

        if (dataAlignment != MAX_DATA_ALIGNMENT) {
            advance(dataAlignment);

            dataAlignment = 0;
        } else {
            constexpr u64 alignmentMask = MAX_DATA_ALIGNMENT - 1;
            const u64 oldAlignment = offset & alignmentMask;

            if (oldAlignment != 0) {
                advance(MAX_DATA_ALIGNMENT - oldAlignment);

                dataAlignment = oldAlignment;
            } else { // Move forward by 16 bytes if offset was aligned
                advance(MAX_DATA_ALIGNMENT);

                dataAlignment = 0;
            }
        }
    }

    void alignUp(u64 alignment) {
        const u64 alignmentMask = alignment - 1;
        const u64 oldAlignment = offset & alignmentMask;

        if (oldAlignment != 0) {
            advance(alignment - oldAlignment);
        }
    }

public:
    IPCContext(void *ipcPointer) : ipcPointer(ipcPointer), service(NULL), dataAlignment(MAX_DATA_ALIGNMENT), header(), handleDescriptor(), domainHeader(), dataPayloadHeader(), numDomainObjects(0), pid(0), copyHandlesOffset(0ULL), moveHandlesOffset(0ULL), domainHeaderOffset(0ULL), dataPayloadOffset(0ULL) {
        setOffset(0ULL);

        std::memset(ipcBuffer, 0, IPC_BUFFER_SIZE);
        std::memset(pointerDescriptorOffset, 0, sizeof(pointerDescriptorOffset));
    }

    IPCContext(void *ipcPointer, KObject *service) : ipcPointer(ipcPointer), service(service), dataAlignment(MAX_DATA_ALIGNMENT), header(), handleDescriptor(), domainHeader(), dataPayloadHeader(), numDomainObjects(0), pid(0), copyHandlesOffset(0ULL), moveHandlesOffset(0ULL), domainHeaderOffset(0ULL), dataPayloadOffset(0ULL) {
        setOffset(0ULL);

        std::memset(ipcBuffer, 0, IPC_BUFFER_SIZE);
        std::memset(pointerDescriptorOffset, 0, sizeof(pointerDescriptorOffset));
    }

    ~IPCContext() {}

    bool isDomain() {
        KDomain *domain = dynamic_cast<KDomain *>(service);

        if (domain == NULL) {
            return false;
        }

        return domain->isDomainObject();
    }

    void *getIPCPointer() {
        return ipcPointer;
    }

    void setService(KObject *service) {
        this->service = service;
    }

    KObject *getService() {
        return this->service;
    }

    template<typename T>
    T read() {
        T data;
        std::memcpy(&data, &ipcBuffer[offset], sizeof(T));

        offset += sizeof(T);

        return data;
    }

    template<typename T>
    void write(T data) {
        std::memcpy(&ipcBuffer[offset], &data, sizeof(T));

        offset += sizeof(T);
    }

    void *getData() {
        return &ipcBuffer[dataPayloadOffset + 4 * sizeof(u32)];
    }

    u64 getCommand() {
        return dataPayloadHeader.command;
    }

    u64 getCommandType() {
        return header.type;
    }

    u64 getObjectID() {
        return domainHeader.objectID;
    }

    std::vector<Handle> getCopyHandles() {
        return copyHandles;
    }

    void copyHandle(Handle handle) {
        copyHandles.push_back(handle);
    }

    void moveHandle(Handle handle) {
        moveHandles.push_back(handle);
    }

    void readXBufferDescriptors() {
        pointerDescriptorOffset[PointerBuffer::X] = getOffset();

        for (u64 descriptor = 0; descriptor < header.numX; descriptor++) {
            XBufferDescriptor d;
            d.raw = read<u64>();

            bufferDescriptors[PointerBuffer::X].push_back(BufferDescriptor{.address = d.getAddress(), .size = d.getSize()});

            PLOG_VERBOSE << "Buffer descriptor " << descriptor << " (address = " << std::hex << d.getAddress() << ", size = " << d.getSize() << ")";
        }
    }

    void readBufferDescriptors(int buffer, u64 numDescriptors) {
        pointerDescriptorOffset[buffer] = getOffset();
    
        for (u64 descriptor = 0; descriptor < numDescriptors; descriptor++) {
            ABWBufferDescriptor d;
            d.raw[0] = read<u32>();
            d.raw[1] = read<u32>();
            d.raw[2] = read<u32>();

            bufferDescriptors[buffer].push_back(BufferDescriptor{.address = d.getAddress(), .size = d.getSize()});

            PLOG_VERBOSE << "Buffer descriptor " << descriptor << " (address = " << std::hex << d.getAddress() << ", size = " << d.getSize() << ", flags = " << d.getFlags() << ")";
        }
    }

    void marshal() {
        setOffset(0);

        PLOG_VERBOSE << "New command packet header = " << std::hex << header.raw;

        write(header.raw);

        if (header.hasHandleDescriptor) {
            PLOG_VERBOSE << "New handle descriptor = " << std::hex << handleDescriptor.raw;

            write(HandleDescriptor{.sendPID = 0, .numCopyHandles = 0, .numMoveHandles = 1}.raw);

            if (handleDescriptor.sendPID) { // Shouldn't happen
                PLOG_FATAL << "Unexpected PID bit setting";

                exit(0);
            }

            if (handleDescriptor.numCopyHandles > 0) {
                for (u32 copyHandle = 0; copyHandle < handleDescriptor.numCopyHandles; copyHandle++) {
                    PLOG_VERBOSE << "Copy handle " << copyHandle << " = " << std::hex << copyHandles[copyHandle].raw;

                    write(kernel::copyHandle(copyHandles[copyHandle]));
                }
            }

            if (handleDescriptor.numMoveHandles > 0) {
                for (u32 moveHandle = 0; moveHandle < handleDescriptor.numMoveHandles; moveHandle++) {
                    PLOG_VERBOSE << "Move handle " << moveHandle << " = " << std::hex << moveHandles[moveHandle].raw;

                    write(moveHandles[moveHandle].raw);
                }
            }
        }

        setOffset(dataPayloadOffset);

        PLOG_VERBOSE << "New data payload header (magic = " << std::hex << dataPayloadHeader.magic << ", version = " << dataPayloadHeader.version << ")";

        write(dataPayloadHeader.magic);
        write(dataPayloadHeader.version);

        setOffset(sizeof(u32) * header.dataSize);

        if (isDomain() && (numDomainObjects != 0)) {
            for (u32 domainObject = 0; domainObject < numDomainObjects; domainObject++) {
                PLOG_VERBOSE << "New domain object " << domainObject << " = " << std::hex << moveHandles[domainObject].raw;

                write(((KServiceSession *)service)->add(moveHandles[domainObject]));
            }
        }

        // Write back IPC buffer
        std::memcpy(ipcPointer, ipcBuffer, IPC_BUFFER_SIZE);
    }

    void unmarshal() {
        // Copy raw IPC message into buffer
        std::memcpy(ipcBuffer, ipcPointer, IPC_BUFFER_SIZE);

        header.raw = read<u64>();

        PLOG_VERBOSE << "Command packet header = " << std::hex << header.raw;

        if (header.type == CommandType::Close) { // Return early if this packet closes the session
            return;
        }

        if (header.hasHandleDescriptor) {
            handleDescriptor.raw = read<u32>();

            PLOG_VERBOSE << "Handle descriptor = " << std::hex << handleDescriptor.raw;

            if (handleDescriptor.sendPID) {
                pid = read<u64>();

                PLOG_VERBOSE << "PID = " << std::hex << pid;
            }

            if (handleDescriptor.numCopyHandles > 0) {
                copyHandlesOffset = getOffset();

                PLOG_VERBOSE << "Copy handles = " << handleDescriptor.numCopyHandles << " (offset = " << std::hex << copyHandlesOffset << ")";

                for (u32 copyHandle = 0; copyHandle < handleDescriptor.numCopyHandles; copyHandle++) {
                    copyHandles.emplace_back(Handle{.raw = read<u32>()});
                }
            }

            if (handleDescriptor.numMoveHandles > 0) {
                moveHandlesOffset = getOffset();

                PLOG_VERBOSE << "Move handles = " << handleDescriptor.numMoveHandles << " (offset = " << std::hex << moveHandlesOffset << ")";

                for (u32 moveHandle = 0; moveHandle < handleDescriptor.numMoveHandles; moveHandle++) {
                    moveHandles.emplace_back(Handle{.raw = read<u32>()});
                }
            }
        }

        if (header.numX > 0) {
            PLOG_VERBOSE << "Reading X buffer descriptors";

            readXBufferDescriptors();
        }

        if (header.numA > 0) {
            PLOG_VERBOSE << "Reading A buffer descriptors";

            readBufferDescriptors(PointerBuffer::A, header.numA);
        }

        if (header.numB > 0) {
            PLOG_VERBOSE << "Reading B buffer descriptors";

            readBufferDescriptors(PointerBuffer::B, header.numB);
        }

        if (header.numW > 0) {
            PLOG_FATAL << "Unimplemented W buffer descriptors";

            exit(0);
        }

        alignData();

        if (isDomain() && (header.type == CommandType::Request)) {
            domainHeaderOffset = getOffset();

            domainHeader.raw = read<u64>();

            PLOG_VERBOSE << "Domain header (command = " << domainHeader.command << ", input objects = " << std::hex << domainHeader.numInput << ", data payload length = " << domainHeader.dataPayloadLength << ", object ID = " << domainHeader.objectID << ")";

            if ((domainHeader.command != DomainCommand::SendMessage) && (domainHeader.command != DomainCommand::CloseVirtualHandle)) {
                PLOG_FATAL << "Invalid domain command";

                exit(0);
            }

            if (domainHeader.numInput != 0) {
                PLOG_FATAL << "Unimplemented input objects";

                exit(0);
            }

            if (domainHeader.command == DomainCommand::CloseVirtualHandle) {
                PLOG_FATAL << "Unimplemented CloseVirtualHandle";

                exit(0);
            }

            advance(sizeof(u64));
        }

        dataPayloadOffset = getOffset();

        dataPayloadHeader.magic = read<u32>();
        dataPayloadHeader.version = read<u32>();
        dataPayloadHeader.command = read<u32>();

        PLOG_VERBOSE << "Data payload header (magic = " << std::hex << dataPayloadHeader.magic << ", version = " << dataPayloadHeader.version << ", command = " << dataPayloadHeader.command << ", offset = " << dataPayloadOffset << ")";

        if (dataPayloadHeader.magic != INPUT_HEADER_MAGIC) {
            PLOG_FATAL << "Invalid data payload magic";

            exit(0);
        }

        // Skip data payload
        setOffset(sizeof(u32) * (header.dataSize + 2 * header.numX + 3 * (header.numA + header.numB)));
        alignData();

        if (header.flagsC != 0) {
            if (header.flagsC == 1) {
                PLOG_FATAL << "Unimplemented inlined C buffer";
                
                exit(0);
            }

            u64 numC = header.flagsC - 2;
            if (header.flagsC == 2) {
                numC = 1;
            }

            // Fetch C buffer descriptors
            for (u64 descriptor = 0; descriptor < numC; descriptor++) {
                receiveDescriptors.emplace_back(CBufferDescriptor{.raw = read<u64>()});

                PLOG_VERBOSE << "Receive buffer descriptor " << descriptor << " (addr = " << std::hex << receiveDescriptors[descriptor].address << ", size = " << receiveDescriptors[descriptor].size << ")";
            }
        }
    }

    void makeReply(u64 numParams, u32 numCopyHandles = 0, u32 numMoveHandles = 0, bool forceMove = false) {
        header.type = CommandType::Invalid; // Shouldn't matter

        advance(sizeof(header.raw));

        u32 moveHandles = 0;
        if (!isDomain() || forceMove) {
            moveHandles = numMoveHandles;
        } else {
            numDomainObjects = numMoveHandles;
        }
        
        // Build handle descriptor (if any)
        if ((numCopyHandles | moveHandles) != 0) {
            header.hasHandleDescriptor = 1;

            handleDescriptor.numCopyHandles = numCopyHandles;
            handleDescriptor.numMoveHandles = moveHandles;

            advance(sizeof(handleDescriptor.raw) + 4 * (numCopyHandles + moveHandles));
        }

        alignUp(16);

        if (isDomain() && !forceMove) {
            domainHeaderOffset = getOffset();

            // Write output header
            write(numDomainObjects);

            advance(sizeof(u32) * 3);
        }

        dataPayloadOffset = getOffset();

        // Make output header
        dataPayloadHeader.magic = OUTPUT_HEADER_MAGIC;
        dataPayloadHeader.version = 0;

        advance(sizeof(u64) + sizeof(u32) * numParams);

        header.dataSize = getOffset() / 4;

        // Set write offset to right after the data payload header (magic, version)
        setOffset(dataPayloadOffset + sizeof(u64));

        PLOG_DEBUG << "Data reply offset = " << std::hex << getOffset();
    }

    std::vector<u8> readSend(int idx = 0) {
        if ((idx > header.numX) && (idx > header.numA)) {
            PLOG_FATAL << "Invalid descriptor index";

            exit(0);
        }

        std::vector<u8> data;

        const bool useX = (header.numX > 0) && (bufferDescriptors[PointerBuffer::X][0].size > 0);

        BufferDescriptor *d;
        if (useX) {
            d = &bufferDescriptors[PointerBuffer::X][idx];
        } else {
            d = &bufferDescriptors[PointerBuffer::A][idx];
        }

        if (d->size == 0) {
            PLOG_FATAL << "Send buffer is empty";

            exit(0);
        }

        data.resize(d->size);
        std::memcpy(data.data(), sys::memory::getPointer(d->address), d->size);

        return data;
    }

    // Writes data to output buffers B/C
    u64 writeReceive(const std::vector<u8> &output) {
        const bool useB = (header.numB > 0) && (bufferDescriptors[PointerBuffer::B][0].size > 0);

        u64 address, size;
        if (useB) {
            // Get address from B buffer descriptors
            BufferDescriptor &d = bufferDescriptors[PointerBuffer::B][0];

            address = d.address;
            size = d.size;
        } else {
            // TODO: use X buffer descriptor receive index
            CBufferDescriptor &d = receiveDescriptors[0];

            address = d.address;
            size = d.size;
        }

        if (output.size() > size) {
            PLOG_WARNING << "Output size larger than buffer";
        }
        
        size = std::min((u64)output.size(), size);

        std::memcpy(sys::memory::getPointer(address), output.data(), size);

        return size;
    }
};

}
