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

#include "buffer_queue.hpp"

#include <cstdlib>
#include <ios>
#include <vector>

#include <plog/Log.h>

#include "parcel.hpp"

#include "nvflinger.hpp"

namespace android::buffer_queue {

using parcel::Parcel;

namespace Code {
    enum : u32 {
        Connect = 10,
    };
}

void connect(Parcel &in, Parcel &out) {
    const bool enableListener = in.read<u32>() == 1;
    const u32 api = in.read<u32>();
    const bool producerControlledByApp = in.read<u32>() == 1;

    PLOG_VERBOSE << "CONNECT (Enable listener = " << enableListener << ", API = " << api << ", producer controlled by app = " << producerControlledByApp << ")";

    nvidia::nvflinger::connect(enableListener, api, producerControlledByApp, out);
}

// https://android.googlesource.com/platform/frameworks/native/+/29a3e90879fd96404c971e7187cd0e05927bbce0/libs/gui/IGraphicBufferProducer.cpp
void transact(IPCContext &ctx, u32 code, u32 flags) {
    Parcel in, out;
    in.deserialize(ctx.readSend());

    switch (code) {
        case Code::Connect:
            connect(in, out);
            break;
        default:
            PLOG_FATAL << "Unimplemented transaction (code = " << code << ", flags = " << std::hex << flags << ")";

            exit(0);
    }

    ctx.writeReceive(out.serialize());
}

}
