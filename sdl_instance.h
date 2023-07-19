#pragma once

#include <SDL2/SDL.h>

class SDL_instance {
private:
    SDL_Renderer* renderer;
    SDL_Event event;
    SDL_Window* window;
    SDL_Texture* texture;

public:
    SDL_instance(const char* title, int init_x, int init_y, int w, int h) {
        SDL_instance::window = SDL_CreateWindow(title, init_x, init_y, w, h, 0);
        SDL_instance::renderer = SDL_CreateRenderer(this->window, -1, 0);
        SDL_instance::texture = SDL_CreateTexture(
            this->renderer, 
            SDL_PIXELFORMAT_RGBA8888, 
            SDL_TEXTUREACCESS_TARGET,
            w,
            h
        );

        SDL_SetRenderTarget(SDL_instance::renderer, SDL_instance::texture);
        SDL_SetRenderDrawColor(SDL_instance::renderer, 255, 255, 255, 255);
    }

    ~SDL_instance();

    SDL_Renderer* get_renderer() {
        return this->renderer;
    }

    SDL_Event get_event() {
        return this->event;
    }

    SDL_Window* get_window() {
        return this->window;
    }

    SDL_Texture* get_texture() {
        return this->texture;
    }
};