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
    uint32 total_frames = 0;

    bool quit = false;
    while (!quit){
        time_ms frame_start_time = SDL_GetTicks();
        total_frames++;

        while (SDL_PollEvent(&event) != 0){
            if(event.type == SDL_QUIT){
                quit = true;
            }
        }

        // TODO: ADD Frame Limiting
        nes->step_frame();
        if(nes->isRunning() == false){
//            quit = true;
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
        SDL_UpdateTexture(
            texture,
            nullptr,
            nes->getFrame(),
            RES_X * 4
        );

        SDL_RenderCopy(renderer, texture, nullptr, &screen);
        SDL_RenderPresent(renderer);

        /**
        * Limit framerate
        */
        constexpr time_ms TARGET_FPS = 1000.0 / 60.0;
        time_ms  frame_dt = SDL_GetTicks() - frame_start_time;
        if(frame_dt < TARGET_FPS){
            SDL_Delay(TARGET_FPS - frame_dt);
        }
        time_ms frame_end_time = SDL_GetTicks();

        /**
         * Count framerate
         */
         static float past_fps[20] = {60.0};

         //Get current FPS
         past_fps[total_frames % 20] = 1000.0 / (frame_end_time - frame_start_time);

         float avg_fps = 0;
         for (unsigned i = 0; i < 20; i++)
            avg_fps += past_fps[i];
         avg_fps /= 20;

         sprintf(window_title, "nes - %d fups", int(avg_fps));
         SDL_SetWindowTitle(window, window_title);
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
    this->cpu_wram = new Ram(0x800);
    this->ppu_pram = new Ram(32);
    this->ppu_vram = new Ram(0x800);
    this->ppu_oam = new Ram(256);

    // Init MMUs
    this->dma = new DMA(*this->cpu_wram, *this->ppu_oam);
    this->cpu_mmu = new CPU_MMU(
        /* ram */ *this->cpu_wram,
        /* ppu */ *this->ppu,
        /* apu */ *VoidMemory::Get(),
        /* joy */ *VoidMemory::Get(),
        /* rom */ this->cart
    );

    this->ppu_mmu = new PPU_MMU(
            *this->ppu_vram,
            *this->ppu_pram,
            this->cart
    );
    
    //create processor
    this->cpu = new CPU(*this->cpu_mmu);
    this->ppu = new PPU(*this->ppu_mmu, *this->ppu_oam,  *this->dma);
    
    this->is_running = false;
    this->clock_cycles = 0;
}

Nes::~Nes(){
    delete this->dma;
    delete this->cpu;
    delete this->cpu_mmu;
    delete this->ppu;
    delete this->ppu_mmu;
    delete this->cpu_wram;
    delete this->ppu_pram;
    delete this->ppu_vram;
}

bool Nes::loadCartridge(Cartridge *cart){
    if(cart == nullptr || !cart->is_valid()){
        return false;
    }
    
    this->cart = cart;
    this->cpu_mmu->addCartridge(this->cart);
    this->ppu_mmu->addCartridge(this->cart);
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
    this->ppu->power_cycle();

    this->cpu_wram->clear();
    this->ppu_pram->clear();
    this->ppu_vram->clear();
}

void Nes::reset(){
    this->is_running = true;
    this->cpu->reset();
    this->ppu->reset();
}

void Nes::cycle(){
    if (this->is_running == false) return;

    //execute a CPU instruction
    uint8 cpu_cycles = this->cpu->step();

    //Run the ppu 3x for every cpu_cycle it took
    for(uint i = 0; i < cpu_cycles * 3; i++)
        this->ppu->cycle();

    if(this->cpu->getState() == CPU::State::Halted){
        this->is_running = false;
    }
}

void Nes::step_frame() {
    if(this->is_running == false) return;

    for(uint i = 0; i < 2000; i++)
        this->cycle();
}

const uint8* Nes::getFrame() const {
    return this->ppu->getFrame();
}