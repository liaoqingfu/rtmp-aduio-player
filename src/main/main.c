
/**
 * rtmp音频播放器
 *
 * 廖庆富
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序用于接收RTMP流媒体并在本地保存成FLV格式的文件。
 * This program can receive rtmp live stream and save it as local flv file.
 * 第三方库 
 * (1)librtmp
 * (2)flaac
 * (3)x264
 * (4)sdl2
 */
#include <iostream>
#include <cstring>
#include <unistd.h>
#ifdef __cplusplus          // 一定要类似这样添加c头文件
extern "C" {
#endif 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif 


#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

     

#include <librtmp/rtmp.h>
#include <librtmp/log.h>
#include "rtmp_parse.hpp"
#include "audio_dec.hpp"
#include "h264_decoder.hpp"
#include "SDl2Displayer.hpp"
#include "media_player.hpp"
#include "log_util.hpp"
#include "alsa_pcm.hpp"

using namespace std;

int InitSockets()
{
	
	return 1;
}

void CleanupSockets()
{
	
}

int ffmpegSdl2(char *file) {
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
    

    // Register all formats and codecs
    av_register_all();
    
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    
    // Open video file
    if(avformat_open_input(&pFormatCtx, file, NULL, NULL)!=0)
        return -1; // Couldn't open file
    
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information
    
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, file, 0);
    
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
        //if (!SDL_GetRendererInfo(screen, &info))
       //     av_log(NULL, AV_LOG_VERBOSE, "Initialized %s renderer.\n", info.name);
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

    int index = 0;
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
            SDL_Delay(20);
            //Sleep(500);
            if(index++ > 10)
            break;
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


int main(int argc, char* argv[])
{
	RtmpParser *rtmpParser;
	//MediaPlayer mediaPlayer;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
        }
	InitSockets();
	initLog4cpp();
	FunEntry();
	double duration=-1;
	int nRead;
	//is live stream ?
	bool bLiveStream=true;				
	char rtmpUrl[256];
	
	int bufsize=1024*1024*10;			
	char *buf =  new char[bufsize];
	memset(buf,0,bufsize);
	long countbufsize=0;
    

	FILE *fp=fopen("receive.flv","wb");
	if (!fp){
		RTMP_LogPrintf("Open File Error.\n");
		CleanupSockets();
		return -1;
	}

	if(argc < 2)
	{
        RTMP_LogPrintf("please use rtmp-save-flv  rtmp_url\n");
        strcpy(rtmpUrl, "rtmp://live.hkstv.hk.lxdns.com/live/hks");
        RTMP_LogPrintf("used default url = %s\n", rtmpUrl);
	}
	else
	{
        strcpy(rtmpUrl,argv[1]);
        RTMP_LogPrintf("user url = %s\n", rtmpUrl);
	}
	rtmpParser = new RtmpParser(rtmpUrl);

	RTMP_LogPrintf("ffmpegSdl2 into\n");
	ffmpegSdl2("/mnt/hgfs/qingfu/ffmpeg/qt_project/rtmp-aduio-player/test/source.200kbps.768x320.flv");
	RTMP_LogPrintf("ffmpegSdl2 exit\n");
	
	while(1)
	{
		sleep(1);
	}


    ffmpegSdl2("/mnt/hgfs/qingfu/ffmpeg/qt_project/rtmp-aduio-player/test/source.200kbps.768x320.flv");
	return 0;
	/* set log level */
	//RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
	//RTMP_LogSetLevel(loglvl);

	RTMP *rtmp=RTMP_Alloc();                // 为结构体“RTMP”分配内存。
	RTMP_Init(rtmp);                        // 初始化结构体“RTMP”中的成员变量。
	//set connection timeout,default 30s
	rtmp->Link.timeout=10;	                // 设置连接超时
	// HKS's live URL
	if(!RTMP_SetupURL(rtmp, rtmpUrl))       // 设置输入的RTMP连接的URL。
	{
		RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
		RTMP_Free(rtmp);                    // 释放结构体“RTMP”
		CleanupSockets();
		return -1;
	}
	if (bLiveStream)
	{
		rtmp->Link.lFlags|=RTMP_LF_LIVE;
	}
	
	//1hour
	RTMP_SetBufferMS(rtmp, 3600*1000);		
	
	if(!RTMP_Connect(rtmp,NULL)){
		RTMP_Log(RTMP_LOGERROR,"Connect Err\n");
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}

	if(!RTMP_ConnectStream(rtmp,0)){
		RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}

	while(nRead=RTMP_Read(rtmp,buf,bufsize)){
		fwrite(buf,1,nRead,fp);
        
		countbufsize+=nRead;
		RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB\n",nRead,countbufsize*1.0/1024);
		if (!bLiveStream&&duration<0)                   // 点播的时候使用
		{
			 duration = RTMP_GetDuration(rtmp);
			 RTMP_LogPrintf("duration:%f\n",duration);
		}
	}

	if(fp)
		fclose(fp);

	if(buf){
		delete [] buf;
	}

	if(rtmp){
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		rtmp=NULL;
	}	
	return 0;
}

