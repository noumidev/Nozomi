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

#include "svc.hpp"

#include <cstdlib>
#include <cstring>
#include <ios>

#include <plog/Log.h>

#include "cpu.hpp"
#include "handle.hpp"
#include "ipc_manager.hpp"
#include "kernel.hpp"
#include "memory.hpp"
#include "object.hpp"
#include "result.hpp"

namespace hle::svc {

namespace SupervisorCall {
    enum : u32 {
        SetHeapSize = 0x01,
        QueryMemory = 0x06,
        MapSharedMemory = 0x13,
        CreateTransferMemory = 0x15,
        CloseHandle,
        WaitSynchronization = 0x18,
        GetSystemTick = 0x1E,
        ConnectToNamedPort = 0x1F,
        SendSyncRequest = 0x21,
        OutputDebugString = 0x27,
        GetInfo = 0x29,
    };
}

// GetInfo
namespace InfoType {
    enum : u32 {
        AliasRegionAddress = 2,
        AliasRegionSize = 3,
        HeapRegionAddress = 4,
        HeapRegionSize = 5,
        TotalMemorySize = 6,
        UsedMemorySize = 7,
        RandomEntropy = 11,
        AslrRegionAddress = 12,
        AslrRegionSize = 13,
        StackRegionAddress = 14,
        StackRegionSize = 15,
        SystemResourceSizeTotal = 16,
        SystemResourceSizeUsed = 17,
        InitialProcessIdRange = 19,
        UserExceptionContextAddress = 20,
    };
}

void handleSVC(u32 svc) {
    switch (svc) {
        case SupervisorCall::SetHeapSize:
            svcSetHeapSize();
            break;
        case SupervisorCall::QueryMemory:
            svcQueryMemory();
            break;
        case SupervisorCall::MapSharedMemory:
            svcMapSharedMemory();
            break;
        case SupervisorCall::CreateTransferMemory:
            svcCreateTransferMemory();
            break;
        case SupervisorCall::CloseHandle:
            svcCloseHandle();
            break;
        case SupervisorCall::WaitSynchronization:
            svcWaitSynchronization();
            break;
        case SupervisorCall::GetSystemTick:
            svcGetSystemTick();
            break;
        case SupervisorCall::ConnectToNamedPort:
            svcConnectToNamedPort();
            break;
        case SupervisorCall::SendSyncRequest:
            svcSendSyncRequest();
            break;
        case SupervisorCall::OutputDebugString:
            svcOutputDebugString();
            break;
        case SupervisorCall::GetInfo:
            svcGetInfo();
            break;
        default:
            PLOG_FATAL << "Unimplemented SVC " << std::hex << svc;

            exit(0);
    }
}

void svcCloseHandle() {
    const Handle handle = hle::makeHandle((u32)sys::cpu::get(0));

    PLOG_INFO << "svcCloseHandle (handle = " << std::hex << handle.raw << ")";

    kernel::closeHandle(handle);

    sys::cpu::set(0, KernelResult::Success);
}

void svcConnectToNamedPort() {
    const u64 portName = sys::cpu::get(1);

    PLOG_INFO << "svcConnectToNamedPort (port name* = " << std::hex << portName << ")";

    const char *name = (char *)sys::memory::getPointer(portName);

    sys::cpu::set(0, KernelResult::Success);
    sys::cpu::set(1, kernel::makeSession(kernel::getPort(name)->getHandle()).raw);
}

void svcCreateTransferMemory() {
    const u64 address = sys::cpu::get(1);
    const u64 size = sys::cpu::get(2);
    const u32 permission = (u32)sys::cpu::get(3);

    PLOG_INFO << "svcCreateTransferMemory (address = " << std::hex << address << ", size = " << size << ", permission = " << permission << ")";

    if (!sys::memory::isAligned(address) || !sys::memory::isAligned(size)) {
        PLOG_FATAL << "Unaligned transfer memory address/size";

        exit(0);
    }

    if (sys::memory::getPointer(address) == NULL) {
        PLOG_FATAL << "Memory doesn't exist";

        exit(0);
    }

    sys::cpu::set(0, KernelResult::Success);
    sys::cpu::set(1, kernel::makeTransferMemory(address, size, permission).raw);
}

void svcGetInfo() {
    const u32 type = sys::cpu::get(1);
    const Handle handle = hle::makeHandle((u32)sys::cpu::get(2));
    const u64 subType = sys::cpu::get(3);

    PLOG_INFO << "svcGetInfo (type = " << type << ", handle = " << std::hex << handle.raw << ", sub type = " << subType << ")";

    sys::cpu::set(0, KernelResult::Success);

    switch (type) {
        case InfoType::AliasRegionAddress:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for AliasRegionAddress";
            }

            sys::cpu::set(1, 0); // What is this?
            break;
        case InfoType::AliasRegionSize:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for AliasRegionSize";
            }

            sys::cpu::set(1, 0); // What is this?
            break;
        case InfoType::HeapRegionAddress:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for HeapRegionAddress";
            }

            sys::cpu::set(1, sys::memory::MemoryBase::Heap);
            break;
        case InfoType::HeapRegionSize:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for HeapRegionSize";
            }

            sys::cpu::set(1, sys::memory::getHeapSize());
            break;
        case InfoType::TotalMemorySize:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for TotalMemorySize";
            }

            sys::cpu::set(1, sys::memory::TOTAL_MEMORY_SIZE);
            break;
        case InfoType::UsedMemorySize:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for UsedMemorySize";
            }

            sys::cpu::set(1, sys::memory::getUsedMemorySize());
            break;
        case InfoType::RandomEntropy:
            if ((handle.raw != 0) || (subType > 3)) {
                PLOG_WARNING << "Unexpected handle/sub type for UsedMemorySize";
            }

            sys::cpu::set(1, 0); // TODO: add proper entropy generation
            break;
        case InfoType::AslrRegionAddress:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for AslrRegionAddress";
            }

            sys::cpu::set(1, sys::memory::MemoryBase::Application); // Probably
            break;
        case InfoType::AslrRegionSize:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for AslrRegionSize";
            }

            sys::cpu::set(1, sys::memory::getAppSize()); // Probably
            break;
        case InfoType::StackRegionAddress:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for StackRegionAddress";
            }

            sys::cpu::set(1, sys::memory::MemoryBase::Stack);
            break;
        case InfoType::StackRegionSize:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for StackRegionSize";
            }

            sys::cpu::set(1, sys::memory::PAGE_SIZE);
            break;
        case InfoType::SystemResourceSizeTotal:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for SystemResourceSizeTotal";
            }

            sys::cpu::set(1, sys::memory::TOTAL_MEMORY_SIZE); // Maybe? (probably not)
            break;
        case InfoType::SystemResourceSizeUsed:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for SystemResourceSizeUsed";
            }

            sys::cpu::set(1, sys::memory::getUsedMemorySize()); // Maybe? (probably not)
            break;
        case InfoType::InitialProcessIdRange:
            if (handle.raw != 0) {
                PLOG_WARNING << "Unexpected handle type for InitialProcessIdRange";
            }

            switch (subType) {
                case 0: // Lower bound
                    sys::cpu::set(1, 0);
                    break;
                case 1: // Upper bound
                    sys::cpu::set(1, 0);
                    break;
                default:
                    PLOG_FATAL << "Invalid sub type InitialProcessIdRange";

                    exit(0);
            }
            break;
        case InfoType::UserExceptionContextAddress:
            if ((handle.raw != KernelHandles::CurrentProcess) || (subType != 0)) {
                PLOG_WARNING << "Unexpected handle/sub type for UserExceptionContextAddress";
            }

            sys::cpu::set(1, 0); // TODO: figure out which values are normally returned on a real Switch
            break;
        default:
            PLOG_FATAL << "Unknown type " << type;

            exit(0);
    }
}

void svcGetSystemTick() {
    PLOG_INFO << "svcGetSystemTick";

    sys::cpu::set(0, sys::cpu::getSystemTicks());
}

void svcMapSharedMemory() {
    Handle handle = hle::makeHandle((u32)sys::cpu::get(0));
    const u64 address = sys::cpu::get(1);
    const u64 size = sys::cpu::get(2);
    const u32 permission = (u32)sys::cpu::get(3);

    PLOG_INFO << "svcMapSharedMemory (handle = " << std::hex << handle.raw << ", address = " << address << ", size = " << size << ", permission = " << permission << ")";

    if (!sys::memory::isAligned(address) || !sys::memory::isAligned(size)) {
        PLOG_FATAL << "Unaligned shared memory address/size";

        exit(0);
    }

    if (handle.type != HandleType::KSharedMemory) {
        PLOG_FATAL << "Invalid handle type";

        exit(0);
    }

    ((KSharedMemory *)kernel::getObject(handle))->map(address, size, permission);

    sys::cpu::set(0, KernelResult::Success);
}

void svcOutputDebugString() {
    const u64 string = sys::cpu::get(0);
    const u64 size = sys::cpu::get(1);

    PLOG_INFO << "svcOutputDebugString (string* = " << std::hex << string << ", size = " << size << ")";

    char msg[size + 1];
    std::memset(msg, 0, sizeof(msg));
    std::memcpy(msg, sys::memory::getPointer(string), size);

    PLOG_DEBUG << (const char *)msg;

    sys::cpu::set(0, KernelResult::Success);
}

void svcQueryMemory() {
    const u64 memoryInfo = sys::cpu::get(0);
    const u64 address = sys::cpu::get(2);

    PLOG_INFO << "svcQueryMemory (MemoryInfo* = " << std::hex << memoryInfo << ", address = " << address << ")";

    const sys::memory::MemoryBlock memoryBlock = sys::memory::queryMemory(address);

    sys::memory::write64(memoryInfo, memoryBlock.baseAddress);
    sys::memory::write64(memoryInfo + 8, sys::memory::PAGE_SIZE * memoryBlock.size);
    sys::memory::write32(memoryInfo + 16, memoryBlock.type);
    sys::memory::write32(memoryInfo + 20, memoryBlock.attribute);
    sys::memory::write32(memoryInfo + 24, memoryBlock.permission);
    sys::memory::write32(memoryInfo + 28, 0); // IpcRefCount?
    sys::memory::write32(memoryInfo + 32, 0); // DeviceRefCount?
    sys::memory::write32(memoryInfo + 36, 0); // Padding

    sys::cpu::set(0, KernelResult::Success);
    sys::cpu::set(1, 0); // Page info?
}

void svcSendSyncRequest() {
    const Handle handle = hle::makeHandle((u32)sys::cpu::get(0));

    PLOG_INFO << "svcSendSyncRequest (session handle = " << std::hex << handle.raw << ")";

    ipc::sendSyncRequest(handle, sys::cpu::getTLSAddr());

    sys::cpu::set(0, KernelResult::Success);
}

void svcSetHeapSize() {
    const u64 size = sys::cpu::get(1);

    PLOG_INFO << "svcSetHeapSize (size = " << std::hex << size << ")";

    if (!sys::memory::isAlignedHeap(size)) {
        PLOG_FATAL << "Unaligned heap size";

        exit(0);
    }

    sys::memory::setHeapSize(size);

    sys::cpu::set(0, KernelResult::Success);
    sys::cpu::set(1, sys::memory::MemoryBase::Heap);
}

void svcWaitSynchronization() {
    const u64 handles = sys::cpu::get(1);
    const i32 handlesNum = (i32)sys::cpu::get(2);
    const i64 timeout = sys::cpu::get(3);

    PLOG_INFO << "svcWaitSynchronization (Handle* = " << std::hex << handles << ", handles num = " << std::dec << handlesNum << ", timeout = " << timeout << ")";

    if (handlesNum > 0x40) {
        PLOG_FATAL << "Too many handles";

        exit(0);
    }

    for (i32 i = 0; i < handlesNum; i++) {
        PLOG_DEBUG << "Waiting on object with handle " << std::hex << sys::memory::read32(handles + 4 * i);
    }

    PLOG_WARNING << "Unimplemented WaitSynchronization";

    sys::cpu::set(0, KernelResult::Success);
    sys::cpu::set(1, 0);
}

}
