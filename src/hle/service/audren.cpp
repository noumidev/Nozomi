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

#include "audren.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

#include "handle.hpp"
#include "kernel.hpp"
#include "result.hpp"

namespace hle::service::audren {

namespace AudioRendererCommand {
    enum : u32 {
        Start = 5,
        QuerySystemEvent = 7,
    };
}

namespace Command {
    enum : u32 {
        OpenAudioRenderer,
        GetWorkBufferSize,
    };
}

AudioRenderer::AudioRenderer() {}

AudioRenderer::~AudioRenderer() {}

void AudioRenderer::cmdQuerySystemEvent(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "QuerySystemEvent";

    if (event.type == HandleType::None) {
        event = kernel::makeEvent(true);
    }

    reply.makeReply(2, 1);
    reply.write(KernelResult::Success);
    reply.copyHandle(event);
}

void AudioRenderer::cmdStart(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "Start";

    reply.makeReply(2);
    reply.write(KernelResult::Success);
}

void AudioRenderer::handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case AudioRendererCommand::Start:
            cmdStart(ctx, reply);
            break;
        case AudioRendererCommand::QuerySystemEvent:
            cmdQuerySystemEvent(ctx, reply);
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void handleRequest(IPCContext &ctx, IPCContext &reply) {
    const u32 command = ctx.getCommand();
    switch (command) {
        case Command::OpenAudioRenderer:
            cmdOpenAudioRenderer(ctx, reply);
            break;
        case Command::GetWorkBufferSize:
            cmdGetWorkBufferSize(ctx, reply);
            break;
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

void cmdGetWorkBufferSize(IPCContext &ctx, IPCContext &reply) {
    (void)ctx;

    PLOG_INFO << "GetWorkBufferSize (stubbed)";

    reply.makeReply(4);
    reply.write(KernelResult::Success);
    reply.write(0x10000ULL);
}

void cmdOpenAudioRenderer(IPCContext &ctx, IPCContext &reply) {
    const u64 *data = (u64 *)ctx.getData();

    AudioRendererParams params;
    std::memcpy(&params, data, sizeof(AudioRendererParams));

    const u64 transferMemorySize = data[7];
    const u64 aruid = data[8];

    const std::vector<Handle> handles = ctx.getCopyHandles();

    PLOG_INFO << "OpenAudioRenderer (sample rate = " << params.sampleRate << ", transfer memory size = " << std::hex << transferMemorySize << ", ARUID = " << std::dec << aruid << ")";

    const Handle handle = kernel::makeService<AudioRenderer>();
    AudioRenderer *renderer = (AudioRenderer *)kernel::getObject(handle);

    renderer->setParams(params);

    reply.makeReply(2, 0, 1);
    reply.write(KernelResult::Success);
    reply.moveHandle(handle);
}

}
