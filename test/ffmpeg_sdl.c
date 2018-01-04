
// tutorial02.c
// A pedagogical video player that will stream through every video frame as fast as it can.
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard,
// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
//
// Use the Makefile to build all examples.
//
// Run using
// tutorial02 myvideofile.mpg
//
// to play the video stream on your screen.


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#include <stdio.h>

int
randomInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL;
    AVPacket        packet;
    int             frameFinished;
    //float           aspect_ratio;
    
    AVDictionary    *optionsDict = NULL;
    struct SwsContext *sws_ctx = NULL;
    //SDL_CreateTexture();
    SDL_Texture    *bmp = NULL;
    SDL_Window     *screen = NULL;
    SDL_Rect        rect;
    SDL_Event       event;
    
    if(argc < 2) {
        fprintf(stderr, "Usage: test <file>\n");
        exit(1);
    }
    // Register all formats and codecs
    av_register_all();
    
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    
    // Open video file
    if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
        return -1; // Couldn't open file
    
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information
    
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);
    
    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    if(videoStream==-1)
        return -1; // Didn't find a video stream
    
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
    
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
        return -1; // Could not open codec
    
    // Allocate video frame
    pFrame=av_frame_alloc();
    
    AVFrame* pFrameYUV = av_frame_alloc();
    if( pFrameYUV == NULL )
        return -1;
    
    // Make a screen to put our videe
//#ifndef __DARWIN__
//    screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
//#else
//    screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
//#endif
//    SDL_WM_SetCaption("My Game Window", "game");
//    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 0, SDL_FULLSCREEN | SDL_OPENGL);
	printf("width = %d, height = %d\n", pCodecCtx->width,  pCodecCtx->height);
    screen = SDL_CreateWindow("My Game Window",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              800,  480,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	
    
    
    
    if(!screen) {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
    }
	SDL_Renderer *renderer;// = SDL_CreateRenderer(screen, -1, 0);
	SDL_RendererInfo info;
    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        av_log(NULL, AV_LOG_WARNING, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
        renderer = SDL_CreateRenderer(screen, -1, 0);
    }
    if (renderer) {
        if (!SDL_GetRendererInfo(screen, &info))
            av_log(NULL, AV_LOG_VERBOSE, "Initialized %s renderer.\n", info.name);
    }
    // Allocate a place to put our YUV image on that screen
//    bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
//                               pCodecCtx->height,
//                               SDL_YV12_OVERLAY,
//                               screen);
    bmp = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_YV12,SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);
    //SDL_SetTextureBlendMode(bmp,SDL_BLENDMODE_BLEND );
    
    sws_ctx =
    sws_getContext
    (
     pCodecCtx->width,
     pCodecCtx->height,
     pCodecCtx->pix_fmt,
     pCodecCtx->width,
     pCodecCtx->height,
     AV_PIX_FMT_YUV420P,
     SWS_BILINEAR,
     NULL,
     NULL,
     NULL
     );
    
    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                  pCodecCtx->height);
    uint8_t* buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    avpicture_fill((AVPicture *)pFrameYUV, buffer, AV_PIX_FMT_YUV420P,
                   pCodecCtx->width, pCodecCtx->height);
    
    // Read frames and save first five frames to disk
    i=0;
    
    rect.x = 0;
    rect.y = 0;
    rect.w = pCodecCtx->width;
    rect.h = pCodecCtx->height;
    
    while(av_read_frame(pFormatCtx, &packet)>=0) {
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                                  &packet);
            
            // Did we get a video frame?
            if(frameFinished) {

                sws_scale
                (
                 sws_ctx,
                 (uint8_t const * const *)pFrame->data,
                 pFrame->linesize,
                 0,
                 pCodecCtx->height,
                 pFrameYUV->data,
                 pFrameYUV->linesize
                 );
                ////iPitch 计算yuv一行数据占的字节数
                SDL_UpdateTexture( bmp, &rect, pFrameYUV->data[0], pFrameYUV->linesize[0] );
                SDL_RenderClear( renderer );
                SDL_RenderCopy( renderer, bmp, &rect, &rect );
                SDL_RenderPresent( renderer );
            }
            SDL_Delay(50);
            //Sleep(500);
        }
        
        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
        SDL_PollEvent(&event);
        switch(event.type) {
            case SDL_QUIT:
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }
        
    }
    
    SDL_DestroyTexture(bmp);
    
    // Free the YUV frame
    av_free(pFrame);
    av_free(pFrameYUV);
    // Close the codec
    avcodec_close(pCodecCtx);
    
    // Close the video file
    avformat_close_input(&pFormatCtx);
    
    return 0;
}



