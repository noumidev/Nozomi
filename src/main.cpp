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

#include <plog/Init.h>
#include <plog/Log.h>
#include <plog/Formatters/FuncMessageFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include "emulator.hpp"

int main(int argc, char **argv) {
    // Initialize logger
    static plog::ColorConsoleAppender<plog::FuncMessageFormatter> consoleAppender;
    plog::init(plog::verbose, &consoleAppender);

    if (argc < 2) {
        PLOG_FATAL << "Please provide a Switch executable";

        return -1;
    }

    sys::emulator::init(argv[1]);
    sys::emulator::run();

    return 0;
}
