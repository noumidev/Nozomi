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

#include "handle.hpp"
#include "types.hpp"

namespace hle {

class IPCContext;

constexpr int KPORT_NAME_LENGTH = 16;

enum class ThreadStatus {
    Dormant,
    Started,
    Waiting,
};

struct ThreadContext {
    u64 regs[31];
    u64 vregs[64];

    u64 pc, sp;
    u32 pstate;
    u32 fpcr, fpsr;
    u64 tpidr;
};

// Base class for domain objects
class KDomain {
protected:
    bool isDomain;

    std::vector<Handle> domainHandles;

public:
    KDomain();
    virtual ~KDomain();

    bool isDomainObject();

    void makeDomain();

    int add(Handle handle);
    void remove(int objectID);

    Handle getDomainObjectHandle(int objectID);

    void handleRequest(int objectID, IPCContext &ctx, IPCContext &reply);
};

// Kernel object base class
class KObject {
protected:
    Handle handle;

    int refCount;

public:
    KObject();
    virtual ~KObject();

    Handle getHandle();

    int getRefCount();

    void setHandle(Handle handle);

    void open();
    bool close();
};

class KEvent : public KObject {
public:
    KEvent(bool autoClear);
    ~KEvent();
};

class KPort : public KObject {
    char name[KPORT_NAME_LENGTH];

public:
    KPort(const char *name);
    ~KPort();

    const char *getName();
};

// Service base class (system services don't use this)
class KService : public KObject {
public:
    KService();
    virtual ~KService();

    virtual const char *getName();

    virtual void handleRequest(IPCContext &ctx, IPCContext &reply);
};

class KServiceSession : public KObject, public KDomain {
    const char *name; // Making this a pointer should be fine, service name pointers are constant

public:
    KServiceSession(const char *name);
    ~KServiceSession();

    const char *getName();
};

class KSession : public KObject {
    Handle portHandle;

public:
    KSession(Handle portHandle);
    ~KSession();

    Handle getPortHandle();
};

class KSharedMemory : public KObject {
    u64 size;

    void *mem;

public:
    KSharedMemory(u64 size);
    ~KSharedMemory();

    u64 getSize();

    void *getMem();

    void map(u64 address, u64 size, u32 permission);
    void unmap(u64 address, u64 size);
};

class KThread : public KObject {
    ThreadContext ctx;

    ThreadStatus status;

    i32 priority, processorID;

public:
    KThread();
    ~KThread();

    ThreadContext *getCtx();

    u64 getTLSBase();

    void setTLSBase(u64 tlsBase);

    void setPriority(i32 priority);
    void setProcessorID(i32 processorID);

    void start();
};

class KTransferMemory : public KObject {
    u64 address, size;

public:
    KTransferMemory(u64 address, u64 size);
    ~KTransferMemory();

    u64 getAddress();
    u64 getSize();
};

}
