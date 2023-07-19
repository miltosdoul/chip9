#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <windows.h>
#include <chrono>
#include <cmath>

#include "../sdl_instance.h"
#include "../lib/types.h"
#include "chip9.h"

double TICK_INTERVAL = 0;
Uint64 cycle_start = 0;

Chip9::Chip9(const bool throttled, 
             const int fps, 
             const bool scaled, 
             const int scaling, 
             const bool drawatonce, 
             const bool userects,
             const bool drawallrectsatonce) {

    this->THROTTLED = throttled;
    this->FPS = fps;
    this->SCALED = scaled;
    this->SCALING = scaled ? scaling : 1;
    this->DRAW_ALL_POINTS_AT_ONCE = drawatonce && !drawallrectsatonce;
    this->USE_RECTS = scaled && userects;
    this->DRAW_ALL_RECTS_AT_ONCE = scaled && userects && drawallrectsatonce;

    if (this->THROTTLED) {
        TICK_INTERVAL = (1000 / (double) FPS);
        std::cout << "DEBUG: Tick interval set to " << TICK_INTERVAL << "\n";
    }

    int sc_width = SCREEN_WIDTH;
    int sc_height = SCREEN_HEIGHT;

    if (this->SCALED) {
        sc_width *= SCALING;
        sc_height *= SCALING;
    } else {
        // padding
        sc_width += 200;
        sc_height += 50;
    }

    this->sdl_instance = new SDL_instance("Chip 9",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          sc_width,
                                          sc_height);

    this->stack = new FIFO();

    memset(this->v, 0, sizeof(this->v));
    memset(this->ram, 0, sizeof(this->ram));
    memset(this->display, 0, sizeof(this->display));

    this->i = 0x0;
    this->pc = 0x0;

    this->delay_timer = 0x0;
    this->sound_timer = 0x0;
}

void Chip9::destroy_window() {
    SDL_DestroyRenderer(this->sdl_instance->get_renderer());
    SDL_DestroyWindow(this->sdl_instance->get_window());
    SDL_Quit();

    return;
}

void Chip9::read_program(u16 program[], int program_size) {
    _swab((char*) program, (char*) (this->ram + RAM_OFFSET), program_size);
}

void draw_all_rects_at_once(u8 d[64][32], SDL_Renderer* renderer, int scaling) {
    SDL_Rect* rects = nullptr;
    int rects_count = 0;
    int index = 0;

    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                rects_count++;
            }
        }
    }

    rects = new SDL_Rect[rects_count];

    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                rects[index].x = w * scaling;
                rects[index].y = h * scaling;
                rects[index].w = scaling;
                rects[index].h = scaling;
                index++;
            }
        }
    }

    SDL_RenderFillRects(renderer, rects, rects_count);
    SDL_RenderDrawRects(renderer, rects, rects_count);
    delete [] rects;
}

void draw_rect(u8 d[64][32], SDL_Renderer* renderer, int scaling) {
    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                SDL_Rect rect = {
                    .x = w * scaling,
                    .y = h * scaling,
                    .w = scaling,
                    .h = scaling
                };

                SDL_RenderFillRect(renderer, &rect);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }
    }
}

void draw_all_points_at_once(u8 d[64][32], SDL_Renderer* renderer, int scaling) {
    SDL_Point* points = nullptr;
    int points_count = 0;
    int index = 0;

    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                points_count++;
            }
        }
    }

    points = new SDL_Point[(scaling * scaling) * points_count];

    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                for (int i = scaling * w; i < ((scaling * w) + scaling); i++) {
                    for (int j = scaling * h; j < ((scaling * h) + scaling); j++) {
                        points[index].x = i;
                        points[index].y = j;
                        index++;
                    }
                }
            }
        }
    }
    
    SDL_RenderDrawPoints(renderer, points, (scaling * scaling) * points_count);
    delete [] points;
}

void draw_points(u8 d[64][32], SDL_Renderer* renderer, int scaling) {
    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                SDL_Point* points = new SDL_Point[scaling * scaling];
                int index = 0;

                for (int i = scaling * w; i < ((scaling * w) + scaling); i++) {
                    for (int j = scaling * h; j < ((scaling * h) + scaling); j++) {
                        points[index].x = i;
                        points[index].y = j;
                        index++;
                    }
                }

                SDL_RenderDrawPoints(renderer, points, scaling * scaling);
                delete [] points;
            }
        }
    }
}

void draw_point(u8 d[64][32], SDL_Renderer* renderer) {
    for (int h = 0; h < SCREEN_HEIGHT; h++) {
        for (int w = 0; w < SCREEN_WIDTH; w++) {
            if (d[w][h]) {
                SDL_RenderDrawPoint(renderer, w, h);
            }
        }
    }
}

void Chip9::render_frame() {
    SDL_Renderer* r = this->sdl_instance->get_renderer();
    SDL_Texture* t = this->sdl_instance->get_texture();
    
    SDL_SetRenderTarget(r, t);

    if (this->SCALED) {
        if (this->USE_RECTS) {
            if (this->DRAW_ALL_RECTS_AT_ONCE) {
                draw_all_rects_at_once(this->display, r, this->SCALING);
            } else {
                draw_rect(this->display, r, this->SCALING);
            }
        } else if (this->DRAW_ALL_POINTS_AT_ONCE) {
            draw_all_points_at_once(this->display, r, this->SCALING);
        } else {
            draw_points(this->display, r, this->SCALING);
        }
    } else {
        draw_point(this->display, r);
    }

    this->frames_rendered++;

    SDL_SetRenderTarget(r, nullptr);
    SDL_RenderCopy(r, t, nullptr, nullptr);
    SDL_RenderPresent(r);

    Uint64 cycle_end = SDL_GetTicks64();

    if (this->THROTTLED) {
        double delay = TICK_INTERVAL - (cycle_end - cycle_start);

        if (delay > 0) {
            Sleep(delay);
        }
    }
}

void Chip9::clear_screen() {
    SDL_Renderer* renderer = this->sdl_instance->get_renderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    memset(this->display, 0, (SCREEN_WIDTH * SCREEN_HEIGHT) * sizeof(u8));
}

int getLength(u16 arr[]) {
    int i = 0;
    while (arr[i]) i++;
    return i;
}

void Chip9::dump_ram(int program_size) {
    for (int i = 0; i < program_size * 2; i = i + 2) {
        u16 opcode = (this->read_ram((RAM_OFFSET) + i    ) << 8)
                   | (this->read_ram((RAM_OFFSET) + i + 1) << 0);

        printf("%x ", opcode);
        if (i % 16 == 0) {
            printf("\n");
        }
    }
}

int Chip9::poll_and_handle_event() {
    SDL_Event e = this->get_sdl_instance()->get_event();

    if (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                this->destroy_window();
                return 1;
            case SDL_WINDOWEVENT:
                switch (e.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        auto end = std::chrono::high_resolution_clock::now();
                        std::cout << "Quitting...\n";
                        std::cout << "Execution time: " << ((end - this->start) / std::chrono::milliseconds(1)) << "ms\n";
                        std::cout << "Cycles executed: " << this->cycles << "\n";
                        std::cout << "Average FPS: " 
                                  << (this->frames_rendered * 1000) / ((end - this->start) / std::chrono::milliseconds(1))
                                  << " FPS\n";
                        this->destroy_window();
                        return 1;
                    }
                }
        }
    }

    return 0;
}

int Chip9::run(u16* program) {
    int program_length = getLength(program);

    this->read_program(program, 2 * program_length);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("ERROR: SDL Initialization failed\n");
        return 0;
    }

    int k = 0;

    while (1) {
        cycle_start = SDL_GetTicks64();

        if (k == program_length * 2) {
            k = 0;
            this->cycles++;
        }

        if (this->poll_and_handle_event()) {
            return 0;
        }
        
        // k     :  0x12 
        // k + 1 :  0x34
        // opcode:  0x1234
        u16 opcode = (this->read_ram((RAM_OFFSET) + k    ) << 8)
                   | (this->read_ram((RAM_OFFSET) + k + 1) << 0);

        this->decode(opcode);

        k = k + 2;
    }
}

void Chip9::decode(u16 ins) {
    if (this->start == std::chrono::time_point<std::chrono::high_resolution_clock>::max()) {
        this->start = std::chrono::high_resolution_clock::now();
    }

    switch (GET_NIBBLE(ins, 4)) {
        case 0x0:
            switch (GET_LSB(ins)) {
                case 0xe0:
                    switch (GET_LSB(ins)) {
                        case 0xe0:
                            clear_screen();
                            break;
                    }
                    break;
                case 0xee:
                    set_pc(this->stack->dequeue());
                    break;
            }
            break;
        case 0x1:
            set_pc(ins & 0x0FFF);
            break;
        case 0x2:
            {
                stack->enqueue(this->get_pc());
                set_pc(ins & 0x0FFF);
            }
            break;
        case 0x3:
            // TODO
            break;
        case 0x4:
            // TODO
            break;
        case 0x5:
            // TODO
            break;
        case 0x6:
            x6xnn(ins);
            break;
        case 0x7:
            x7xnn(ins);
            break;
        case 0x8:
            {
            u8 x = GET_NIBBLE(ins, 3);
            u8 y = GET_NIBBLE(ins, 2);
            u8 vx = get_v(x);
            u8 vy = get_v(y);

            switch(GET_NIBBLE(ins, 1)) {
                case 0:
                    {
                        set_v(vy, x);
                    }
                    break;
                case 1:
                    {
                        u16 x_or_y = vx | vy;
                        set_v(x_or_y, x);
                    }
                    break;
                case 2:
                    {
                        u16 x_and_y = vx & vy;
                        set_v(x_and_y, x);
                    }
                    break;
                case 3:
                    {
                        u16 x_xor_y = vx ^ vy;
                        set_v(x_xor_y, x);
                    }
                    break;
                case 4:
                    {
                        u16 x_plus_y = vx + vy;

                        // if no overflow, carry flag is set to 0.
                        set_v(0, 0x0F);

                        if (x_plus_y > 255) {
                            x_plus_y = 255;
                            set_v(1, 0x0F);
                        }

                        set_v(x_plus_y, x);
                    }
                    break;
                case 5:
                    {
                        set_v(0, 0x0F);

                        u16 x_sub_y = vx - vy;
                        set_v(x_sub_y, x);

                        if (vx > vy) {
                            set_v(1, 0x0F);
                        }
                    }
                    break;
                case 6:
                    {
                        u8 shift_bit = vx & 0x01;
                        vx = vx >> 1;
                        set_v(vx, x);
                        set_v(shift_bit, 0x0F);
                    }
                    break;
                case 7:
                    {
                        set_v(0, 0x0F);

                        u16 y_sub_x = vy - vx;
                        set_v(y_sub_x, x);

                        if (vy > vx) {
                            set_v(1, 0x0F);
                        }
                    }
                    break;
                case 0xE:
                    {
                        // mask is 0x80, or 0b1000 0000, to get most significant bit
                        u8 shift_bit = vx & 0x80;
                        vx = vx << 1;
                        set_v(vx, x);
                        set_v(shift_bit, 0x0F);
                    }
                    break;
                }
            }
        case 0x9:
            // TODO
            break;
        case 0xa:
            set_i(ins & 0x0FFF);
            break;
        case 0xb:
            // TODO
            break;
        case 0xc:
            // TODO
            break;
        case 0xd:
            xdxyn(ins);
            break;
        default:
            break;
    };
}

void Chip9::x6xnn(u16 ins) {
    u8 idx = GET_NIBBLE(ins, 3);
    set_v(GET_LSB(ins), idx);
}

void Chip9::x7xnn(u16 ins) {
    u8 idx = GET_NIBBLE(ins, 3);
    u8 val = get_v(idx) + GET_LSB(ins);
    set_v(val, idx);
}

void Chip9::xdxyn(u16 ins) {
    this->draw_calls++;

    int x = get_v(GET_NIBBLE(ins, 3)) % SCREEN_WIDTH;
    int y = get_v(GET_NIBBLE(ins, 2)) % SCREEN_HEIGHT;
    u8 max = GET_NIBBLE(ins, 1);

    set_v(0x00, 0x0F);

    for (int row = 0; row < max; row++) {
        u8 sprite_data = this->read_ram(this->get_i() + row);

        for (int bit = 0; bit < 8; bit++) {
            u8 data = GET_SPRITE_DATA(sprite_data, bit);
            u8 p = this->get_pixel(x, y);

            u8 val = p ^ data;
            this->set_pixel(x, y, val);

            // If pixel val=0 and data=1, set overflow flag (VF)
            this->set_v(~val & data, 0x0F);

            ++x;
        }

        // reset x for rendering of new row
        x = (int) this->get_v(GET_NIBBLE(ins, 3)) % SCREEN_WIDTH;

        ++y;
    }

    this->render_frame();
}
