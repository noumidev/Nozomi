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

#include "nvflinger.hpp"

#include <cstdlib>

#include <plog/Log.h>

namespace nvidia::nvflinger {

std::vector<Display> displays;

void init() {
    makeDisplay(makeDisplayName("Default"));
}

void makeDisplay(DisplayName name) {
    static u64 displayID = 0;

    PLOG_DEBUG << "Making display (name = " << name.data() << ", ID = " << displayID << ")";

    displays.emplace_back(Display(name, displayID++));
}

Display *getDisplay(u64 id) {
    if (id > displays.size()) {
        PLOG_FATAL << "Invalid display ID";

        exit(0);
    }

    return &displays[id];
}

u64 openDisplay(DisplayName name) {
    PLOG_DEBUG << "Opening display (name = " << name.data() << ")";

    for (auto &display : displays) {
        if (std::strcmp(display.getName().data(), name.data()) == 0) {
            return display.getID();
        }
    }

    PLOG_FATAL << "Display does not exist";

    exit(0);
}

u64 makeLayer(u64 displayID) {
    static u64 layerID = 0;

    PLOG_DEBUG << "Making layer (display ID = " << displayID << ")";

    getDisplay(displayID)->makeLayer(layerID);

    return layerID++;
}

Layer::Layer(u64 id) : id(id) {}

Layer::~Layer() {}

u64 Layer::getID() {
    return id;
}

Display::Display(DisplayName name, u64 id) : name(name), id(id) {}

Display::~Display() {}

DisplayName Display::getName() {
    return name;
}

u64 Display::getID() {
    return id;
}

void Display::makeLayer(u64 id) {
    layers.emplace_back(Layer(id));
}

HOSDriverBinder::HOSDriverBinder() {}

HOSDriverBinder::~HOSDriverBinder() {}

void HOSDriverBinder::handleRequest(IPCContext &ctx, IPCContext &reply) {
    (void)reply;

    const u32 command = ctx.getCommand();
    switch (command) {
        default:
            PLOG_FATAL << "Unimplemented command " << command;

            exit(0);
    }
}

}
