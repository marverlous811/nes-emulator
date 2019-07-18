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

     int window_w = 256 * 2;
     int window_h = 240 * 2;

     SDL_Init(SDL_INIT_EVERYTHING);
     window = SDL_CreateWindow(
        "nes_emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        window_w, window_h,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //nes screen texture
    const unsigned int textWidth = 256;
    const unsigned int textHeight = 240;
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        textWidth, textHeight
    );
    SDL_Rect screen;
    uint8* pixelBuff = new uint8[textWidth * textHeight * 4];

    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&event) != 0){
            if(event.type == SDL_QUIT){
                quit = true;
            }
        }

        // TODO: ADD Frame Limiting
        nes->step();
        if(nes->isRunning() == false){
            quit = true;
            break;
        }

        //output
        SDL_GetWindowSize(window, &window_w, &window_h);

        float skew_w = window_w / 256.0;
        float skew_h = window_h / 240.0;

        if(skew_w > skew_h){
            // i.e: fat window
            screen.h = window_h;
            screen.w = window_h * (256.0 / 240.0);
            screen.x = -(screen.w - window_w) / 2;
            screen.y = 0;
        }
        else{
            // i.e: tall window
            screen.h = window_w * (240.0 / 256.0);
            screen.w = window_w;
            screen.x = 0;
            screen.y = -(screen.h - window_h) / 2;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        for(int x = 0; x < textWidth; x++){
            for(int y = 0; y < textHeight; y++){
                const unsigned int offset = (textWidth * 4 * y) + x * 4;
                pixelBuff[offset + 0] = (sin((SDL_GetTicks()/ 1000.0)) + 1) * 126; //b
                pixelBuff[offset + 1] = (float(y) / textHeight) * 256; // g
                pixelBuff[offset + 2] = (float(x) / textWidth)  * 256; // r
                pixelBuff[offset + 3] = SDL_ALPHA_OPAQUE;             // a
            }
        }

        SDL_UpdateTexture(
            texture,
            nullptr,
            &pixelBuff[0],
            textWidth * 4
        );

        SDL_RenderCopy(renderer, texture, nullptr, &screen);
        SDL_RenderPresent(renderer);
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

void Nes::step(){
    uint8 cpu_cycles = this->cpu->step();
    if(this->cpu->getState() == CPU::State::Halted){
        this->is_running = false;
    }
}
