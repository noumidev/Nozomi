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

#include "handle.hpp"
#include "ipc.hpp"
#include "object.hpp"
#include "types.hpp"

namespace hle::service::audren {

// https://github.com/switchbrew/libnx/blob/c769852631bc38d49a2576587bef26bc834a42e4/nx/source/services/audren.c#L15
struct AudioRendererParams {
    i32 sampleRate;
    i32 sampleCount;
    i32 mixBufferCount;
    i32 submixCount;
    i32 voiceCount;
    i32 sinkCount;
    i32 effectCount;
    u64 unknown0;
    i32 splitterCount;
    u64 unknown1;
    u32 revision;
} __attribute__((packed));

class AudioRenderer : public KService {
    AudioRendererParams params;
    Handle event;

    void cmdQuerySystemEvent(IPCContext &ctx, IPCContext &reply);
    void cmdStart(IPCContext &ctx, IPCContext &reply);

public:
    AudioRenderer();
    ~AudioRenderer();

    const char *getName() override {
        return "IAudioRenderer";
    }

    void setParams(AudioRendererParams params) {
        this->params = params;
    }

    void handleRequest(IPCContext &ctx, IPCContext &reply) override;
};

void handleRequest(IPCContext &ctx, IPCContext &reply);

void cmdGetWorkBufferSize(IPCContext &ctx, IPCContext &reply);
void cmdOpenAudioRenderer(IPCContext &ctx, IPCContext &reply);

}
