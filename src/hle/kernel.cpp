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

#include "kernel.hpp"

#include <map>
#include <type_traits>
#include <utility>

#include "cpu.hpp"
#include "memory.hpp"

namespace hle::kernel {

Handle mainThreadHandle;

KThread *activeThread;

void init() {
    table::init();

    (void)makePort("sm:");
}

Handle getMainThreadHandle() {
    return mainThreadHandle;
}

void setMainThreadHandle(Handle handle) {
    mainThreadHandle = handle;
}

Handle makeEvent(bool autoClear) {
    const Handle handle = table::add(HandleType::KEvent, new KEvent(autoClear));

    ((KEvent *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KEvent (auto clear = " << autoClear << ", handle = " << std::hex << handle.raw << ")";

    return handle;
}

Handle makePort(const char *name) {
    const Handle handle = table::add(HandleType::KPort, new KPort(name));

    ((KPort *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KPort (name = " << name << ", handle = " << std::hex << handle.raw << ")";

    return handle;
}

Handle makeServiceSession(const char *name) {
    const Handle handle = table::add(HandleType::KServiceSession, new KServiceSession(name));

    ((KServiceSession *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KServiceSession (name = " << name << ", handle = " << std::hex << handle.raw << ")";

    return handle;
}

Handle makeSession(Handle portHandle) {
    const Handle handle = table::add(HandleType::KSession, new KSession(portHandle));

    ((KSession *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KSession (port handle = " << std::hex << portHandle.raw << ", handle = " << handle.raw << ")";

    return handle;
}

Handle makeSharedMemory(u64 size) {
    const Handle handle = table::add(HandleType::KSharedMemory, new KSharedMemory(size));

    ((KSharedMemory *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KSharedMemory (size = " << std::hex << size << ", handle = " << handle.raw << ")";

    return handle;
}

Handle makeThread(u64 entry, u64 args, u64 stackTop, i32 priority, i32 processorID) {
    const Handle handle = table::add(HandleType::KThread, new KThread());

    PLOG_DEBUG << "Making KThread (entry = " << std::hex << entry << ", args* = " << args << ", stack top = " << stackTop << ", priority = " << std::dec << priority << ", processor ID = " << processorID << ")";

    KThread *thread = (KThread *)table::getLast();
    thread->setHandle(handle);

    thread->setPriority(priority);
    thread->setProcessorID(processorID);

    // Set up thread context
    ThreadContext *ctx = thread->getCtx();
    ctx->pc = entry;
    ctx->sp = stackTop;
    ctx->regs[0] = args;

    // Allocate TLS
    thread->setTLSBase(sys::memory::allocateTLS());

    return handle;
}

Handle makeTransferMemory(u64 address, u64 size, u32 permission) {
    const Handle handle = table::add(HandleType::KTransferMemory, new KTransferMemory(address, size));

    ((KTransferMemory *)table::getLast())->setHandle(handle);

    PLOG_DEBUG << "Making KTransferMemory (address = " << std::hex << address << ", size = " << size << ", permission = " << sys::memory::getPermissionString(permission) << ", handle = " << handle.raw << ")";

    // TODO: actually change permissions?

    return handle;
}

void startThread(Handle handle) {
    if (handle.type != HandleType::KThread) {
        PLOG_FATAL << "Invalid thread handle";

        exit(0);
    }

    KThread *thread = (KThread *)table::get(handle);
    thread->start();

    setActiveThread(thread);
}

void setActiveThread(KThread *thread) {
    if (activeThread != NULL) {
        sys::cpu::getContext(activeThread);
    }

    activeThread = thread;

    sys::cpu::setContext(activeThread);
}

void destroyServiceSession(Handle handle) {
    PLOG_DEBUG << "Destroying KServiceSession (handle = " << std::hex << handle.raw << ")";

    KServiceSession *serviceSession = (KServiceSession *)table::remove(handle);

    if (serviceSession->close()) {
        delete serviceSession;
    }
}

void destroySession(Handle handle) {
    PLOG_DEBUG << "Destroying KSession (handle = " << std::hex << handle.raw << ")";

    KSession *session = (KSession *)table::remove(handle);

    if (session->close()) {
        delete session;
    }
}

void closeHandle(Handle handle) {
    PLOG_DEBUG << "Closing handle " << std::hex << handle.raw;

    KObject *object = table::remove(handle);

    if (object->close()) {
        delete object;
    }
}

Handle copyHandle(Handle handle) {
    PLOG_DEBUG << "Copying handle " << std::hex << handle.raw;

    KObject *object = getObject(handle);

    // Increment ref count
    object->open();

    return table::add(handle.type, object);
}

KObject *getObject(Handle handle) {
    return table::get(handle);
}

KPort *getPort(const char *name) {
    PLOG_DEBUG << "Searching port " << name;

    KPort *port = table::getPort(name);

    if (port != NULL) {
        return port;
    }

    PLOG_FATAL << "Unable to find port " << name;

    exit(0);
}

}

