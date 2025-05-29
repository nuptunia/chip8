# include "chip8/chip8.h"
# include <SDL.h>
# include <iostream>

const int video_scale = 10;
const int video_width = 64;
const int video_height = 32;

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "SDL Display failed ): \n";
		return -1; // FAIL!!! 
	}

	SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		video_width * video_scale, video_height * video_scale, 0); 

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); 

	Chip8 emulator;
	emulator.LoadROM("pong.ch8"); 

	bool quit = false;
	SDL_Event event;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) quit = true;
		}

		emulator.Cycle();

		if (emulator.drawFlag) {
			emulator.drawFlag = false;
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
			SDL_RenderClear(renderer); 

			// and here we go maths
			// i need mathsisfun.com
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			for (int y = 0; y < video_height; ++y) {
				for (int x = 0; x < video_width; ++x) {
					if (emulator.gfx[y * video_width + x]) {
						SDL_Rect pixel = { x * video_scale, y * video_scale, video_scale, video_scale }; 
						SDL_RenderFillRect(renderer, &pixel);
					}
				}
			}

			SDL_RenderPresent(renderer);
		}
		SDL_Delay(2);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit(); 
	return 0;

}