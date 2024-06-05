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

#include "error_applet.hpp"

#include <cstdlib>
#include <cstring>

#include <plog/Log.h>

namespace hle::service::applet::error_applet {

struct ErrorViewerStartupParamForApplicationError {
    u32 common;
    u8 version;
    u8 reserved[3];
    u32 errorCode;
    u64 languageCode;
    char dialogMsg[0x800];
    char fullScreenMsg[0x800];
} __attribute__((packed));

static_assert(sizeof(ErrorViewerStartupParamForApplicationError) == 0x1014);

void pushInData(const std::vector<u8> &data) {
    switch (data.size()) {
        case sizeof(ErrorViewerStartupParamForApplicationError):
            {
                ErrorViewerStartupParamForApplicationError error;
                std::memcpy(&error, data.data(), sizeof(ErrorViewerStartupParamForApplicationError));

                PLOG_FATAL << error.dialogMsg << ": " << error.fullScreenMsg;

                exit(0);
            }
            break;
        default:
            PLOG_FATAL << "Unimplemented PushInData";

            exit(0);
    }
}

}
