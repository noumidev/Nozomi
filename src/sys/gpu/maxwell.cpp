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

#include "maxwell.hpp"

#include <array>
#include <cstdlib>
#include <ios>

#include <plog/Log.h>

#include "maxwell_registers.hpp"

namespace sys::gpu::maxwell {

constexpr bool ENABLE_WRITE_LOG = true;

constexpr u32 NUM_REGS = 0x1000;
constexpr u32 NUM_DMA_REGS = 0x800;

std::array<u32, NUM_REGS> regs;
std::array<u32, NUM_DMA_REGS> dmaRegs;

const char *getRenderEnableCMsg() {
    switch (regs[Register::SetRenderEnableC]) {
        case 1:
            return "true";
        default:
            PLOG_FATAL << "Invalid enum";

            exit(0);
    }
}

void write(u32 addr, u32 data) {
    if (addr > NUM_REGS) {
        PLOG_FATAL << "Invalid register address " << std::hex << addr;

        exit(0);
    }

    regs[addr] = data;

    if constexpr (ENABLE_WRITE_LOG) {
        switch (addr) {
            case Register::SetObject:
                PLOG_INFO << "SetObject (data = " << std::hex << data << ")";
                break;
            case Register::SetSmTimeoutInterval:
                PLOG_INFO << "SetSmTimeoutInterval (data = " << std::hex << data << ")";
                break;
            case Register::SetCtSelect:
                PLOG_INFO << "SetCtSelect (data = " << std::hex << data << ")";
                break;
            case Register::SetAntiAliasEnable:
                PLOG_INFO << "SetAntiAliasEnable (enable = " << std::hex << data << ")";
                break;
            case Register::SetRenderEnableC:
                PLOG_INFO << "SetRenderEnableC (render enable = " << getRenderEnableCMsg() << ")";
                break;
            case Register::SetCsaa:
                PLOG_INFO << "SetCsaa (data = " << std::hex << data << ")";
                break;
            case Register::SetZCompression:
                PLOG_INFO << "SetZCompression (enable = " << std::hex << data << ")";
                break;
            default:
                if ((addr >= Register::SetColorCompression) && (addr < Register::SetCtWrite)) {
                    PLOG_INFO << "SetColorCompression" << addr - Register::SetColorCompression << " (enable = " << std::hex << data << ")";
                } else {
                    PLOG_FATAL << "Unrecognized write (register = " << std::hex << addr << ", data = " << data << ")";

                    exit(0);
                }
        }
    }
}

void writeDMA(u32 addr, u32 data) {
    if (addr > NUM_DMA_REGS) {
        PLOG_FATAL << "Invalid DMA register address " << std::hex << addr;

        exit(0);
    }

    dmaRegs[addr] = data;

    if constexpr (ENABLE_WRITE_LOG) {
        switch (addr) {
            default:
                PLOG_WARNING << "Unrecognized DMA write (register = " << std::hex << addr << ", data = " << data << ")";

                break;
        }
    }
}

}
