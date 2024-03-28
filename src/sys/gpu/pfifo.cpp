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

#include "compute.hpp"
#include "fermi.hpp"
#include "kepler.hpp"
#include "maxwell.hpp"
#include "memory_manager.hpp"

namespace sys::gpu::pfifo {

constexpr u32 MAX_SUBCHANNELS = 8;

namespace Opcode {
    enum : u32 {
        UseTertiaryGRP0,
        IncrementAddress,
        NoIncrement = 3,
        Immediate,
        IncrementOnce,
    };
}

namespace GRP0Opcode {
    enum : u32 {
        IncrementAddress,
    };
}

namespace Engine {
    enum : u32 {
        Fermi = 0x902D,
        Kepler = 0xA140,
        GPFIFO = 0xB06F,
        MaxwellDMA = 0xB0B5,
        Maxwell = 0xB197,
        Compute = 0xB1C0,
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

void (*subchannels[MAX_SUBCHANNELS])(u32, u32) = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

void bindSubchannel(u32 subchannel, u32 data) {
    if (subchannel >= MAX_SUBCHANNELS) {
        PLOG_FATAL << "Invalid subchannel " << subchannel;

        exit(0);
    }

    if (subchannels[subchannel] != NULL) {
        PLOG_WARNING << "Subchannel " << subchannel << " already bound";

        return;
    }

    const u32 classID = (u16)data;
    
    switch (data) {
        case Engine::Fermi:
            PLOG_INFO << "Binding subchannel " << subchannel << " to Fermi";

            subchannels[subchannel] = &fermi::write;
            break;
        case Engine::Kepler:
            PLOG_INFO << "Binding subchannel " << subchannel << " to Kepler";

            subchannels[subchannel] = &kepler::write;
            break;
        case Engine::MaxwellDMA:
            PLOG_INFO << "Binding subchannel " << subchannel << " to Maxwell DMA";

            subchannels[subchannel] = &maxwell::writeDMA;
            break;
        case Engine::Maxwell:
            PLOG_INFO << "Binding subchannel " << subchannel << " to Maxwell";

            subchannels[subchannel] = &maxwell::write;
            break;
        case Engine::Compute:
            PLOG_INFO << "Binding subchannel " << subchannel << " to Compute";

            subchannels[subchannel] = &compute::write;
            break;
        default:
            PLOG_FATAL << "Unrecognized class ID " << std::hex << classID;

            exit(0);
    }
}

void submit(CommandListHeader header) {
    PLOG_INFO << "Submitting new command list (IOVA = " << std::hex << header.iova << ", size = " << std::dec << header.size << ", allow flush = " << header.allowFlush << ", is push buffer = " << header.isPushBuf << ", sync = " << header.sync << ")";

    for (u64 i = 0; i < header.size;) {
        Command command;
        command.raw = memory_manager::read32(header.iova + sizeof(u32) * i++);

        PLOG_VERBOSE << "Command word = " << std::hex << command.raw << " (opcode = " << std::dec << command.opcode << ", subchannel = " << command.subchannel << ", address = " << std::hex << command.address << ")";

        switch (command.opcode) {
            case Opcode::UseTertiaryGRP0:
                {
                    PLOG_VERBOSE << "GRP0_USE_TERT";

                    const u64 tertiaryOpcode = (command.raw >> 16) & 3;

                    switch (tertiaryOpcode) {
                        case GRP0Opcode::IncrementAddress:
                            {
                                PLOG_VERBOSE << "GRP0_INC_METHOD";

                                u32 address = command.address;
                                for (u32 j = 0; j < (command.data >> 2); j++) {
                                    const u32 data = memory_manager::read32(header.iova + sizeof(u32) * i++);

                                    if (subchannels[command.subchannel] == NULL) {
                                        if (address != 0) {
                                            PLOG_WARNING << "Subchannel " << command.subchannel << " is unbound";
                                        } else {
                                            bindSubchannel(command.subchannel, data);
                                        }
                                    } else {
                                        subchannels[command.subchannel](address, data);
                                    }

                                    address++;
                                }
                            }
                            break;
                        default:
                            PLOG_FATAL << "Unrecognized tertiary opcode " << tertiaryOpcode;

                            exit(0);
                    }
                }
                break;
            case Opcode::IncrementAddress:
                {
                    PLOG_VERBOSE << "INC_METHOD";

                    u32 address = command.address;
                    for (u32 j = 0; j < command.data; j++) {
                        const u32 data = memory_manager::read32(header.iova + sizeof(u32) * i++);

                        if (subchannels[command.subchannel] == NULL) {
                            if (address != 0) {
                                PLOG_WARNING << "Subchannel " << command.subchannel << " is unbound";
                            } else {
                                bindSubchannel(command.subchannel, data);
                            }
                        } else {
                            subchannels[command.subchannel](address, data);
                        }

                        address++;
                    }
                }
                break;
            case Opcode::NoIncrement:
                {
                    PLOG_VERBOSE << "NON_INC_METHOD";

                    const u32 address = command.address;
                    for (u32 j = 0; j < command.data; j++) {
                        const u32 data = memory_manager::read32(header.iova + sizeof(u32) * i++);

                        if (subchannels[command.subchannel] == NULL) {
                            if (address != 0) {
                                PLOG_WARNING << "Subchannel " << command.subchannel << " is unbound";
                            } else {
                                bindSubchannel(command.subchannel, data);
                            }
                        } else {
                            subchannels[command.subchannel](address, data);
                        }
                    }
                }
                break;
            case Opcode::Immediate:
                PLOG_VERBOSE << "IMMD_DATA_METHOD (data = " << std::hex << command.data << ", register = " << command.address << ")";

                if (subchannels[command.subchannel] == NULL) {
                    PLOG_FATAL << "Subchannel " << command.subchannel << " is unbound";

                    exit(0);
                }

                subchannels[command.subchannel](command.address, command.data);
                break;
            case Opcode::IncrementOnce:
                {
                    PLOG_VERBOSE << "ONE_INC";

                    u32 increment = 1;

                    u32 address = command.address;
                    for (u32 j = 0; j < command.data; j++) {
                        const u32 data = memory_manager::read32(header.iova + sizeof(u32) * i++);

                        if (subchannels[command.subchannel] == NULL) {
                            if (address != 0) {
                                PLOG_WARNING << "Subchannel " << command.subchannel << " is unbound";
                            } else {
                                bindSubchannel(command.subchannel, data);
                            }
                        } else {
                            subchannels[command.subchannel](address, data);
                        }

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
