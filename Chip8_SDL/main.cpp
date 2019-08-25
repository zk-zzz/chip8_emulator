#include "chip8.h"
#include "SDL.h"
#include "cstdio"
#include<cstdlib>
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
int ratio = 10;//window窗口大小和实际模拟器像素之间的比例
//windows窗口大小
int display_width = SCREEN_WIDTH * ratio;
int display_height = SCREEN_HEIGHT * ratio;

// Calculated internals. Do not modify.
#define EMULATION_SPEED 800
#define MS_PER_TICK 1000 / EMULATION_SPEED
#define SDL_TICKS_PER_FRAME 1000 / SDL_FPS


unsigned char *buffer;
SDL_Window *screen;
SDL_Renderer* sdlRenderer;
SDL_Texture* sdlTexture;
SDL_Rect sdlRect;;
chip8 myChip8;//作为全局变量
void Circle();
void Update();
Uint32 startTime;
Uint32 endTime;
Uint32 elapsedTime;
int main(int argc, char** argv)
{

	if (argc < 2)
	{
		printf("Usage: myChip8.exe chip8application\n\n");
		return 1;
	}
	//读取rom
	if (!myChip8.loadApplication(argv[1]))
		return 1;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	screen = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		display_width, display_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!screen) {
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);


	bool quit = false;
	SDL_Event event;
	while (!quit)
	{
		Circle();
		while (SDL_PollEvent(&event)) {
			switch (event.type)      //处理取到的消息，判断消息类型  
			{
			case SDL_KEYDOWN:       //如果是键盘按下事件  
				if (event.key.keysym.sym == SDLK_ESCAPE) //如果按的是ESC键  
				{
					quit = true;                       //退出循环  
				}
				if (event.key.keysym.sym == SDLK_1)
				{
					myChip8.key[0x1] = 1;
				}
				if (event.key.keysym.sym == SDLK_2)
				{
					myChip8.key[0x2] = 1;
				}if (event.key.keysym.sym == SDLK_3)
				{
					myChip8.key[0x3] = 1;
				}if (event.key.keysym.sym == SDLK_4)
				{
					myChip8.key[0xC] = 1;
				}
				if (event.key.keysym.sym == SDLK_q)
				{
					myChip8.key[0x4] = 1;
				}if (event.key.keysym.sym == SDLK_w)
				{
					myChip8.key[0x5] = 1;
				}if (event.key.keysym.sym == SDLK_e)
				{
					myChip8.key[0x6] = 1;
				}if (event.key.keysym.sym == SDLK_r)
				{
					myChip8.key[0xD] = 1;
				}if (event.key.keysym.sym == SDLK_a)
				{
					myChip8.key[0x7] = 1;
				}if (event.key.keysym.sym == SDLK_s)
				{
					myChip8.key[0x8] = 1;
				}if (event.key.keysym.sym == SDLK_d)
				{
					myChip8.key[0x9] = 1;
				}if (event.key.keysym.sym == SDLK_f)
				{
					myChip8.key[0xE] = 1;
				}if (event.key.keysym.sym == SDLK_z)
				{
					myChip8.key[0xA] = 1;
				}if (event.key.keysym.sym == SDLK_x)
				{
					myChip8.key[0x0] = 1;
				}if (event.key.keysym.sym == SDLK_c)
				{
					myChip8.key[0xB] = 1;
				}if (event.key.keysym.sym == SDLK_v)
				{
					myChip8.key[0xF] = 1;
				}
				break;
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_1)
				{
					myChip8.key[0x1] = 0;
				}
				if (event.key.keysym.sym == SDLK_2)
				{
					myChip8.key[0x2] = 0;
				}if (event.key.keysym.sym == SDLK_3)
				{
					myChip8.key[0x3] = 0;
				}if (event.key.keysym.sym == SDLK_4)
				{
					myChip8.key[0xC] = 0;
				}
				if (event.key.keysym.sym == SDLK_q)
				{
					myChip8.key[0x4] = 0;
				}if (event.key.keysym.sym == SDLK_w)
				{
					myChip8.key[0x5] = 0;
				}if (event.key.keysym.sym == SDLK_e)
				{
					myChip8.key[0x6] = 0;
				}if (event.key.keysym.sym == SDLK_r)
				{
					myChip8.key[0xD] = 0;
				}if (event.key.keysym.sym == SDLK_a)
				{
					myChip8.key[0x7] = 0;
				}if (event.key.keysym.sym == SDLK_s)
				{
					myChip8.key[0x8] = 0;
				}if (event.key.keysym.sym == SDLK_d)
				{
					myChip8.key[0x9] = 0;
				}if (event.key.keysym.sym == SDLK_f)
				{
					myChip8.key[0xE] = 0;
				}if (event.key.keysym.sym == SDLK_z)
				{
					myChip8.key[0xA] = 0;
				}if (event.key.keysym.sym == SDLK_x)
				{
					myChip8.key[0x0] = 0;
				}if (event.key.keysym.sym == SDLK_c)
				{
					myChip8.key[0xB] = 0;
				}if (event.key.keysym.sym == SDLK_v)
				{
					myChip8.key[0xF] = 0;
				}
				break;
			case SDL_QUIT:          //如果是窗口关闭事件，例如点击窗口的关闭按钮  
				quit = true;                           //退出循环  
				break;
			default:
				break;
			}
		}
		if (myChip8.drawFlag)
		{

			Update();

		}
	}
	return 0;

}
void Circle()
{
	startTime = SDL_GetTicks();
	myChip8.emulateCycle();
	endTime = SDL_GetTicks();
	elapsedTime = endTime - startTime;
	if (elapsedTime < MS_PER_TICK) {
		SDL_Delay(MS_PER_TICK - elapsedTime);
	}
}
int a = 1;
void Update() {
	int i, j;
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
	SDL_RenderClear(sdlRenderer);
	SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
	for (i = 0; i < SCREEN_HEIGHT; i++) {
		for (j = 0; j < SCREEN_WIDTH; j++) {
				if(myChip8.gfx[(i * 64) + j] == 0 ){

				SDL_Rect dstRect;
				dstRect.x = j * ratio;
				dstRect.y = i * ratio;
				dstRect.w = ratio;
				dstRect.h = ratio;
				SDL_RenderFillRect(sdlRenderer, &dstRect);
				}
			}
	}

	SDL_RenderPresent(sdlRenderer);
}

