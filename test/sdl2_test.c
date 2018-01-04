
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

SDL_Rect rectSrc;
SDL_Rect rectDest;

void test()
{
	SDL_RenderClear(renderer);//清空窗口
	SDL_RenderPresent(renderer);	///*更新屏幕*/
	//SDL_UpdateYUVTexture();
}

int main(int argc, char * argv[])
{

	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)
	{
		fprintf(stderr, "SDL_Init() %s\n" , SDL_GetError());
		exit(-1);
	}
	atexit(SDL_Quit);
	
	/*能让我们绘制的窗口前两个参数控制窗口位置，然后是窗口大小 再然后是位标（falg）*/	
	window = SDL_CreateWindow("Window Title",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1000, 480, SDL_WINDOW_OPENGL);			// 需要设置合适的长宽，否则图像会被拉伸

	if(NULL==window){
		fprintf(stderr, "SDL_CreateWindow() %s\n" , SDL_GetError());
		exit(-1);
	}
	/*渲染器 第一个位标表示显卡 -1表示从显卡中挑一个 第二个表示以显示器的刷新率来刷新画面*/
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(NULL==renderer){
		fprintf(stderr, "SDL_CreateRenderer() %s\n" , SDL_GetError());
		exit(-1);
	}


	SDL_Surface * tempSurface = SDL_LoadBMP("./car.bmp");
	if(tempSurface==NULL)
	{
		fprintf(stderr, "SDL_LoadBMP() failed: %s", SDL_GetError());
		exit(-1);
	}

	texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
	SDL_FreeSurface(tempSurface);

	SDL_RenderCopy(renderer, texture, 0, 0);	//拷贝新的图像 实际显示的是 texture
	SDL_RenderPresent(renderer);				//刷新显示


	//press ESC for exit
	Uint32 exit = 0;
	SDL_Event event;
	while( !exit && SDL_WaitEvent(&event) )
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			if(event.key.keysym.sym == SDLK_ESCAPE)
			{
				exit = 1;
			}
			break;

		default:
			break;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	SDL_DestroyWindow(window);
	window = NULL;


	return 0;
}
