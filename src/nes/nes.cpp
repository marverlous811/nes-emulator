//
//  nes.cpp
//  MyNes
//
//  Created by Ows on 7/6/19.
//  Copyright © 2019 hieunq. All rights reserved.
//
#include <iostream>
#include <fstream>
#include "nes.hpp"
#include "../util/fakeMemory.h"
#include <SDL2/SDL.h>

typedef uint32 time_ms;

int startNes(char* path){
    std::ifstream rom_file(path, std::ios::binary);
    if(!rom_file.is_open()){
        std::cerr << "could not open '" << path << "'\n";
        return -1;
    }

    rom_file.seekg(0, rom_file.end);
    uint32 data_len = rom_file.tellg();
    rom_file.seekg(0, rom_file.beg);

    uint8* data = new uint8 [data_len];
    rom_file.read((char*) data, data_len);

    // Generate cartridge from data
    Cartridge* rom_cart = new Cartridge(data, data_len);

    if(rom_cart->is_valid()){
        std::cout<< "iNES file loaded successfully!\n";
    }
    else{
        std::cerr << "Given file was not an iNES file!\n";
        return -1;
    }

    Nes* nes = new Nes();
    bool res = nes->loadCartridge(rom_cart);
    std::cout<<"ines load cartidge: "<<res<<std::endl;

    nes->power_cycle();

    /*
     * Define SDL need to be moved out of main
     */
     SDL_Event event;
     SDL_Renderer *renderer;
     SDL_Window *window;

     constexpr uint32 RES_X = 256;
     constexpr uint32 RES_Y = 240;
     constexpr  float RATIO_XY = float(RES_X) / float(RES_Y);
     constexpr  float RATIO_YX = float(RES_Y) / float(RES_X);

     int window_w = RES_X * 2;
     int window_h = RES_Y * 2;
     char window_title [64] = "nes_emulator";



     SDL_Init(SDL_INIT_EVERYTHING);
     window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_w, window_h,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        RES_X, RES_Y
    );
    SDL_Rect screen;

    // Main NES pixelbuffer
    uint8* pixelBuff = new uint8[RES_X * RES_Y * 4];

    //Frame Counting
    const time_ms start_time = SDL_GetTicks();
    time_ms frame_start_time;
    time_ms frame_end_time;
    uint32 total_frames = 0;

    bool quit = false;
    while (!quit){
        time_ms frame_start_time = SDL_GetTicks();

        while (SDL_PollEvent(&event) != 0){
            if(event.type == SDL_QUIT){
                quit = true;
            }
        }

        // TODO: ADD Frame Limiting
        nes->step_frame();
        if(nes->isRunning() == false){
            quit = true;
            break;
        }

        //output
        SDL_GetWindowSize(window, &window_w, &window_h);

        if (window_w / float(RES_X) > window_h / float(RES_Y)) {
            // i.e: fat window
            screen.h = window_h;
            screen.w = window_h * RATIO_XY;
            screen.x = -(screen.w - window_w) / 2;
            screen.y = 0;
        }
        else{
            // i.e: tall window
            screen.h = window_w * RATIO_YX;
            screen.w = window_w;
            screen.x = 0;
            screen.y = -(screen.h - window_h) / 2;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for(int x = 0; x < RES_X; x++){
            for(int y = 0; y < RES_Y; y++){
                const unsigned int offset = (RES_X * 4 * y) + x * 4;
                pixelBuff[offset + 0] = (sin((SDL_GetTicks()/ 1000.0)) + 1) * 126; //b
                pixelBuff[offset + 1] = (float(y) / float(RES_Y)) * RES_X; // g
                pixelBuff[offset + 2] = (float(x) / float(RES_X)) * RES_X; // r
                pixelBuff[offset + 3] = SDL_ALPHA_OPAQUE;             // a
            }
        }

        SDL_UpdateTexture(
            texture,
            nullptr,
            &pixelBuff[0],
            RES_X * 4
        );

        SDL_RenderCopy(renderer, texture, nullptr, &screen);
        SDL_RenderPresent(renderer);

        /**
         * Count framerate
         */
         total_frames++;
         float total_dt = frame_end_time - start_time;
         float fps = total_frames / (total_dt / 1000.0);
         sprintf(window_title, "nes - %d fps", int(fps));
         SDL_SetWindowTitle(window, window_title);

         /**
          * Limit framerate
          */
          constexpr time_ms TARGET_FPS = 1000.0 / 60.0;
          time_ms  frame_dt = frame_end_time - frame_start_time;
          if(frame_dt < TARGET_FPS){
            SDL_Delay(TARGET_FPS - frame_dt);
          }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    delete rom_cart;
    return 0;
}

Nes::Nes(){
    this->cart = nullptr;
    // Init RAM modules
    this->cpu_ram = new Ram(0x800);
    
    // Init MMUs
    this->cpu_mmu = new CPU_MMU(
        *this->cpu_ram,
        *VoidMemory::Get(),
        *VoidMemory::Get(),
        *VoidMemory::Get(),
        *VoidMemory::Get(),
        this->cart
    );
    
    //create processor
    this->cpu = new CPU(*this->cpu_mmu);
    
    this->is_running = false;
    this->clock_cycles = 0;
}

Nes::~Nes(){
    delete this->cpu;
    delete this->cpu_mmu;
    delete this->cpu_ram;
}

bool Nes::loadCartridge(Cartridge *cart){
    if(cart == nullptr || !cart->is_valid()){
        return false;
    }
    
    this->cart = cart;
    this->cpu_mmu->addCartridge(this->cart);
    
    return true;
}

void Nes::start(){
    this->is_running = true;
}

void Nes::stop(){
    this->is_running = false;
}

bool Nes::isRunning() const{
    return this->is_running;
}

// Power Cycling initializes all the components to their "power on" state
void Nes::power_cycle(){
    this->is_running = true;
    this->cpu->power_cycle();
    this->cpu_ram->clear();
}

void Nes::reset(){
    this->is_running = true;
    this->cpu->reset();
}

void Nes::step_frame(){
    // We need to run this thing until the PPU has a full frame ready to spit out
    // Once the PPU is implemented, I will add a boolean to the return value of
    // the PPU step method, and I will use that to determine when to break out of
    // the loop.

    // Right now though, i'm going to be a bum and just run the CPU for the
    // equivalent ammount of time :P
    // - PPU renders 262 scanlines per frame
    // - Each scanline lasts for 341 PPU clock cycles
    // - 1 CPU cycle = 3 PPU cycles

    constexpr uint32 CPU_CYCLES_PER_FRAME = 262 * 341 / 3;
    uint8 cpu_cycles = this->cpu->step();
    if(this->cpu->getState() == CPU::State::Halted){
        this->is_running = false;
    }

    this->clock_cycles += cpu_cycles * 3;
}
