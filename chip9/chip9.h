#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <chrono>

#include "../sdl_instance.h"
#include "../lib/fifo/fifo.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

#define RAM_CAPACITY 4096
#define RAM_OFFSET 512

#define NIBBLE 4

#define MSB 0xFF00
#define LSB 0x00FF
#define GET_MSB(a) ( (a & MSB) >> 2*NIBBLE )
#define GET_LSB(a) (  a & LSB  )

// 1-indexed from least to most significant bit
// EXAMPLE: 
//  - if data: 0x1234
//      (i.e.: 0001 0010 0011 0100)
//  -   and i: 4
//  - then res: 0b0001, or 0x1
#define GET_NIBBLE(data, i) ( (data >> ((i - 1) * NIBBLE)) & 0xF )

// gets the 'offset' bit from 's' byte
#define GET_SPRITE_DATA(s, offset) ( (s >> (7 - offset % 8)) & 0x1 )

class Chip9 {
private:
    u8 v[16];
    u8 display[SCREEN_WIDTH][SCREEN_HEIGHT];
    u8 ram[RAM_CAPACITY];
    u8 delay_timer;
    u8 sound_timer;
    u8 font[80] = {
        0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xf0, 0x10, 0xF0, 0x80, 0xf0, // 2
        0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
        0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
        0xf0, 0x10, 0x20, 0x40, 0x40, // 7
        0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
        0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
        0xf0, 0x90, 0xf0, 0x90, 0x90, // A
        0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
        0xf0, 0x80, 0x80, 0x80, 0xf0, // C
        0xe0, 0x90, 0x90, 0x90, 0xe0, // D
        0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
        0xf0, 0x80, 0xf0, 0x80, 0x80  // F
    };

    FIFO* stack;
    u16 i;
    u16 pc;

    SDL_instance* sdl_instance;

    int SCALING;
    int FPS;
    bool SCALED;
    bool THROTTLED;
    bool DRAW_ALL_POINTS_AT_ONCE;
    bool USE_RECTS;
    bool DRAW_ALL_RECTS_AT_ONCE;
    
    void xdxyn(u16 ins);
    void x6xnn(u16 ins);
    void x7xnn(u16 ins);

public:
    Chip9(const bool throttled, 
          const int fps, 
          const bool scaled, 
          const int scaling, 
          const bool drawatonce, 
          const bool userects,
          const bool drawallrectsatonce);
          
    ~Chip9() { delete this; }

    int draw_calls = 0;
    int cycles = 0;
    int frames_rendered = 0;
    std::chrono::high_resolution_clock::time_point start = std::chrono::time_point<std::chrono::high_resolution_clock>::max();

    u8* getRam() {
        return ram;
    }

    void clear_display() {
        memset(this->display, 0, (SCREEN_WIDTH * SCREEN_HEIGHT) * sizeof(u8));
    }

    SDL_instance* get_sdl_instance() {
        return sdl_instance;
    }

    u16 get_pc() {
        return this->pc;
    }

    void set_pc(u16 pc) {
        this->pc = pc;
    }

    u8 get_v(u8 index) {
        return this->v[index];
    }

    void set_v(u8 value, u8 index) {
        this->v[index] = value;
    }

    u16 get_i() {
        return this->i;
    }

    void set_i(u16 i) {
        this->i = i;
    }

    u8 read_ram(u16 idx) {
        return this->ram[idx];
    }

    void set_ram_at(u8 val, u16 idx) {
        this->ram[idx] = val;
    }

    u8 get_pixel(u8 x, u8 y) {
        return this->display[x][y];
    }

    void set_pixel(u8 x, u8 y, u8 val) {
        this->display[x][y] = val;
    }

    int poll_and_handle_event();

    void read_program(u16 program[], int program_size);

    void clear_screen();

    void dump_ram(int program_size);
    int run(u16 program[]);
    void decode(u16 ins);
    void destroy_window();
    void render_frame();
};
