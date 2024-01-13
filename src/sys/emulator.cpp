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

#include "emulator.hpp"

#include "cpu.hpp"
#include "host1x.hpp"
#include "kernel.hpp"
#include "loader.hpp"
#include "memory.hpp"
#include "nvflinger.hpp"
#include "nvhost_gpu.hpp"

#include <SDL2/SDL.h>

namespace sys::emulator {

constexpr u64 CYCLES_PER_FRAME = cpu::CPU_CLOCK / 60;

struct Screen {
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *texture;
};

struct State {
    Screen screen;

    SDL_Event event;

    bool isRunning;
};

State state;

void sdlInit() {
    Screen &screen = state.screen;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, 0, &screen.window, &screen.renderer);
    SDL_SetWindowSize(screen.window, SCR_WIDTH, SCR_HEIGHT);
    SDL_RenderSetLogicalSize(screen.renderer, SCR_WIDTH, SCR_HEIGHT);
    SDL_SetWindowResizable(screen.window, SDL_FALSE);
    SDL_SetWindowTitle(screen.window, "Nozomi");

    screen.texture = SDL_CreateTexture(screen.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCR_WIDTH, SCR_HEIGHT);
}

void init(const char *path) {
    sdlInit();

    cpu::init();
    hle::kernel::init();
    nvidia::host1x::init();
    nvidia::nvflinger::init();
    nvidia::channel::nvhost_gpu::init();

    // Load executable
    loader::load(path);

    // Set up stack and thread-local storage (this will be replaced later on)
    (void)memory::allocate(memory::MemoryBase::Stack, memory::STACK_PAGES, 0, 0, memory::MemoryPermission::RW);
    (void)memory::allocate(memory::MemoryBase::TLSBase, 1, 0, 0, memory::MemoryPermission::RW);

    state.isRunning = true;
}

void run() {
    while (state.isRunning) {
        cpu::run(CYCLES_PER_FRAME);
        cpu::addTicks(CYCLES_PER_FRAME);

        SDL_Event &event = state.event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state.isRunning = false;
                    break;
                default:
                    break;
            }
        }
    }

    SDL_Quit();
}

void update(u8 *fb) {
    Screen &screen = state.screen;

    SDL_UpdateTexture(screen.texture, nullptr, fb, BPP * STRIDE);
    SDL_RenderCopy(screen.renderer, screen.texture, nullptr, nullptr);
    SDL_RenderPresent(screen.renderer);
}

}
