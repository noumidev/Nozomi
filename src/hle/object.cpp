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

#include "object.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

#include "ipc.hpp"
#include "kernel.hpp"

namespace hle {

KDomain::KDomain() : isDomain(false) {}

KDomain::~KDomain() {}

bool KDomain::isDomainObject() {
    return isDomain;
}

void KDomain::makeDomain() {
    isDomain = true;
}

int KDomain::add(Handle handle) {
    domainHandles.push_back(handle);

    return domainHandles.size();
}

void KDomain::remove(int objectID) {
    if ((objectID < 1) || (objectID > (int)domainHandles.size())) {
        PLOG_FATAL << "Object ID out of bounds";

        exit(0);
    }

    domainHandles[objectID - 1] = Handle{.raw = 0};
}

Handle KDomain::getDomainObjectHandle(int objectID) {
    if (objectID > (int)domainHandles.size()) {
        PLOG_FATAL << "Object ID out of bounds";

        exit(0);
    }

    return domainHandles[objectID - 1];
}

void KDomain::handleRequest(int objectID, IPCContext &ctx, IPCContext &reply) {
    if (objectID > (int)domainHandles.size()) {
        PLOG_FATAL << "Object ID out of bounds";

        exit(0);
    }

    assert(objectID != 1); // This is a special case

    KService *service = (KService *)kernel::getObject(domainHandles[objectID - 1]);
    if (service == NULL) {
        PLOG_FATAL << "Invalid virtual handle";

        exit(0);
    }

    service->handleRequest(ctx, reply);
}

KObject::KObject() : handle(Handle{.raw = 0}), refCount(0) {
    open();
}

KObject::~KObject() {}

Handle KObject::getHandle() {
    return handle;
}

int KObject::getRefCount() {
    return refCount;
}

void KObject::setHandle(Handle handle) {
    this->handle = handle;
}

void KObject::open() {
    refCount++;
}

bool KObject::close() {
    if (refCount == 0) {
        PLOG_FATAL << "Attempted to close KObject with refCount = 0";

        exit(0);
    }

    refCount--;

    return refCount <= 0;
}

KEvent::KEvent(bool autoClear) {
    (void)autoClear;
}

KEvent::~KEvent() {}

KService::KService() {}

KService::~KService() {}

const char *KService::getName() {
    return "Invalid service";
}

void KService::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;
    (void)reply;

    PLOG_FATAL << "handleRequest not overriden";

    exit(0);
}

KPort::KPort(const char *name) {
    std::strncpy(this->name, name, KPORT_NAME_LENGTH);

    if (this->name[KPORT_NAME_LENGTH - 1] != 0) {
        PLOG_FATAL << "Name too long";

        exit(0);
    }
}

KPort::~KPort() {}

const char *KPort::getName() {
    return name;
}

KServiceSession::KServiceSession(const char *name) : name(name) {}

KServiceSession::~KServiceSession() {}

const char *KServiceSession::getName() {
    return name;
}

KSession::KSession(Handle portHandle) : portHandle(portHandle) {}

KSession::~KSession() {}

Handle KSession::getPortHandle() {
    return portHandle;
}

KSharedMemory::KSharedMemory(u64 size) : size(size) {
    mem = std::malloc(size);

    std::memset(mem, 0, size);
}

KSharedMemory::~KSharedMemory() {}

u64 KSharedMemory::getSize() {
    return size;
}

void *KSharedMemory::getMem() {
    return mem;
}

void KSharedMemory::map(u64 address, u64 size, u32 permission) {
    open();

    if (size != this->size) {
        PLOG_FATAL << "Size mismatch";

        exit(0);
    }

    sys::memory::map(mem, address, size >> sys::memory::PAGE_SHIFT, 0, 0, permission);
}

void KSharedMemory::unmap(u64 address, u64 size) {
    if (size != this->size) {
        PLOG_FATAL << "Size mismatch";

        exit(0);
    }

    if (getRefCount() == 1) { // UnmapSharedMemory deletes this object
        std::free(mem);
    }

    sys::memory::unmap(address, size >> sys::memory::PAGE_SHIFT);
}

KThread::KThread() : status(ThreadStatus::Dormant) {}

KThread::~KThread() {}

ThreadContext *KThread::getCtx() {
    return &ctx;
}

u64 KThread::getTLSBase() {
    return ctx.tpidr;
}

void KThread::setTLSBase(u64 tlsBase) {
    ctx.tpidr = tlsBase;
}

void KThread::setPriority(i32 priority) {
    this->priority = priority;
}

void KThread::setProcessorID(i32 processorID) {
    this->processorID = processorID;
}

void KThread::start() {
    status = ThreadStatus::Started;
}

KTransferMemory::KTransferMemory(u64 address, u64 size) : address(address), size(size) {}

KTransferMemory::~KTransferMemory() {}

u64 KTransferMemory::getAddress() {
    return address;
}

u64 KTransferMemory::getSize() {
    return size;
}

}
