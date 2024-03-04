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

#include "pfifo.hpp"

#include <cstdlib>
#include <ios>

#include <plog/Log.h>

#include "memory_manager.hpp"

namespace sys::gpu::pfifo {

namespace Opcode {
    enum : u32 {
        IncrementAddress = 1,
        NoIncrement = 3,
        Immediate,
        IncrementOnce,
    };
}

union Command {
    u32 raw;
    struct {
        u32 address : 12;
        u32 : 1;
        u32 subchannel : 3;
        u32 data : 13;
        u32 opcode : 3;
    };
};

static_assert(sizeof(Command) == sizeof(u32));

void submit(CommandListHeader header) {
    PLOG_INFO << "Submitting new command list (IOVA = " << std::hex << header.iova << ", size = " << std::dec << header.size << ", allow flush = " << header.allowFlush << ", is push buffer = " << header.isPushBuf << ", sync = " << header.sync << ")";

    for (u64 i = 0; i < header.size;) {
        Command command;
        command.raw = memory_manager::read32(header.iova + sizeof(u32) * i++);

        PLOG_VERBOSE << "Command word = " << std::hex << command.raw << " (opcode = " << std::dec << command.opcode << ", subchannel = " << command.subchannel << ", address = " << std::hex << command.address << ")";

        switch (command.opcode) {
            case Opcode::IncrementAddress:
                {
                    PLOG_VERBOSE << "INC_METHOD";

                    u32 address = command.address;
                    for (u32 j = 0; j < command.data; j++) {
                        PLOG_VERBOSE << "Data = " << std::hex << memory_manager::read32(header.iova + sizeof(u32) * i++) << ", register = " << address++;
                    }
                }
                break;
            case Opcode::NoIncrement:
                {
                    PLOG_VERBOSE << "NON_INC_METHOD";

                    const u32 address = command.address;
                    for (u32 j = 0; j < command.data; j++) {
                        PLOG_VERBOSE << "Data = " << std::hex << memory_manager::read32(header.iova + sizeof(u32) * i++) << ", register = " << address;
                    }
                }
                break;
            case Opcode::Immediate:
                PLOG_VERBOSE << "IMMD_DATA_METHOD (data = " << std::hex << command.data << ", register = " << command.address << ")";
                break;
            case Opcode::IncrementOnce:
                {
                    PLOG_VERBOSE << "ONE_INC";

                    u32 increment = 1;

                    u32 address = command.address;
                    for (u32 j = 0; j < command.data; j++) {
                        PLOG_VERBOSE << "Data = " << std::hex << memory_manager::read32(header.iova + sizeof(u32) * i++) << ", register = " << address;

                        address += increment;

                        increment = 0;
                    }
                }
                break;
            default:
                PLOG_FATAL << "Unimplemented opcode " << command.opcode;

                exit(0);
        }
    }
}

}
