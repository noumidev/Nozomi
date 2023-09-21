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

#include "cpu.hpp"

#include <cstdlib>
#include <ios>

#include <dynarmic/interface/A64/a64.h>
#include <dynarmic/interface/A64/config.h>
#include <dynarmic/interface/exclusive_monitor.h>

#include <plog/Log.h>

#include "memory.hpp"
#include "svc.hpp"

namespace sys::cpu {

class MyEnvironment final : public Dynarmic::A64::UserCallbacks {
public:
    u64 ticksLeft = 0;
    u64 totalTicks = 0;

    u64 getCyclesForInstruction(bool isThumb, u32 instruction) {
        (void)isThumb;
        (void)instruction;

        return 1;
    }

    std::optional<std::uint32_t> MemoryReadCode(Dynarmic::VAddr vaddr) override {
        return MemoryRead32(vaddr);
    }

    u8 MemoryRead8(Dynarmic::VAddr vaddr) override {
        return memory::read8(vaddr);
    }

    u16 MemoryRead16(Dynarmic::VAddr vaddr) override {
        return memory::read16(vaddr);
    }

    u32 MemoryRead32(Dynarmic::VAddr vaddr) override {
        return memory::read32(vaddr);
    }

    u64 MemoryRead64(Dynarmic::VAddr vaddr) override {
        return memory::read64(vaddr);
    }
    
    Dynarmic::A64::Vector MemoryRead128(Dynarmic::VAddr vaddr) override {
        Dynarmic::A64::Vector data;

        data[0] = memory::read64(vaddr);
        data[1] = memory::read64(vaddr + sizeof(u64));

        return data;
    }

    void MemoryWrite8(Dynarmic::VAddr vaddr, u8 value) override {
        memory::write8(vaddr, value);
    }

    void MemoryWrite16(Dynarmic::VAddr vaddr, u16 value) override {
        memory::write16(vaddr, value);
    }

    void MemoryWrite32(Dynarmic::VAddr vaddr, u32 value) override {
        memory::write32(vaddr, value);
    }

    void MemoryWrite64(Dynarmic::VAddr vaddr, u64 value) override {
        memory::write64(vaddr, value);
    }

    void MemoryWrite128(Dynarmic::VAddr vaddr, Dynarmic::A64::Vector value) override {
        memory::write64(vaddr              , value[0]);
        memory::write64(vaddr + sizeof(u64), value[1]);
    }

    #define makeExclusiveWriteHandler(size) \
    bool MemoryWriteExclusive##size(u64 vaddr, u##size value, u##size expected) override { \
        u##size current = sys::memory::read##size(vaddr); /* Get current value */          \
        if (current == expected) {   /* Perform the write if current == expected */        \
            sys::memory::write##size(vaddr, value);                                        \
            return true; /* Exclusive write succeeded */                                   \
        }                                                                                  \
                                                                                           \
        return false; /* Exclusive write failed */                                         \
    }

    makeExclusiveWriteHandler(8)
    makeExclusiveWriteHandler(16)
    makeExclusiveWriteHandler(32)
    makeExclusiveWriteHandler(64)

    #undef makeExclusiveWriteHandler

    void InterpreterFallback(u64 pc, size_t num_instructions) override {
        (void)pc;
        (void)num_instructions;

        PLOG_FATAL << "Falling back to interpreter";

        exit(0);
    }

    void CallSVC(u32 swi) override {
        hle::svc::handleSVC(swi);
    }

    void ExceptionRaised(u64 pc, Dynarmic::A64::Exception exception) override {
        switch (exception) {
            default:
                PLOG_FATAL << "Unhandled exception (addr = " << std::hex << pc << ")";
                break;
        }
        
        exit(0);
    }

    void AddTicks(u64 ticks) override {
        totalTicks += ticks;

        if (ticks > ticksLeft) {
            ticksLeft = 0;
            return;
        }

        ticksLeft -= ticks;
    }

    u64 GetTicksRemaining() override {
        return ticksLeft;
    }
    
    u64 GetCNTPCT() override {
        PLOG_WARNING << "Unhandled CNTPCT read";

        return 0;
    }
};

u64 tpidr_el0 = 0;

Dynarmic::A64::Jit *jit;
Dynarmic::ExclusiveMonitor *exclusiveMonitor;

MyEnvironment env;

void init() {
    exclusiveMonitor = new Dynarmic::ExclusiveMonitor(1);

    Dynarmic::A64::UserConfig config;

    config.callbacks = &env;
    config.define_unpredictable_behaviour = true;
    config.global_monitor = exclusiveMonitor;
    config.processor_id = 0;

    config.tpidr_el0 = &tpidr_el0;
    config.tpidrro_el0 = &tpidr_el0;

    jit = new Dynarmic::A64::Jit(config);

    env.totalTicks = 0;

    jit->Reset();
    jit->ClearCache();
    jit->GetRegisters().fill(0);
    jit->GetVectors().fill(Dynarmic::A64::Vector{});

    jit->SetPC(memory::MemoryBase::Application);
    jit->SetSP(memory::MemoryBase::Stack + memory::PAGE_SIZE);
    
    setTLSAddr(memory::MemoryBase::TLSBase);
}

void run(u64 ticks) {
    env.ticksLeft = ticks;

    const auto exitReason = jit->Run();

    if ((u32)exitReason != 0) {
        PLOG_FATAL << "Unhandled JIT exit";

        exit(0);
    }
}

void addTicks(u64 ticks) {
    env.totalTicks += ticks;
}

u64 get(int idx) {
    return jit->GetRegister(idx);
}

u64 getTLSAddr() {
    return tpidr_el0;
}

void set(int idx, u64 data) {
    jit->SetRegister(idx, data);
}

void setTLSAddr(u64 addr) {
    tpidr_el0 = addr;
}

}
