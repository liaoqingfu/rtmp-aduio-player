
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
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#ifdef __cplusplus
}
#endif 




     

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
    SDL_Event       event;
    YuvDisplayer *SDl2Displayer_ = new YuvDisplayer();

    // Register all formats and codecs
    av_register_all();

	printf("file = %s\n", file);
    // Open video file
    if(avformat_open_input(&pFormatCtx, file, NULL, NULL)!=0)
    {
    	printf("avformat_open_input failed\n");
    	return -1; // Couldn't open file
    }
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    {
    	printf("avformat_find_stream_info failed\n");
    	return -1; // Couldn't open file
    }
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
    {
    	printf("videoStream  = -1 failed\n");
    	return -1; // Couldn't open file
    }
    
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
    {
    	printf("avcodec_open2  = -1 failed\n");
    	return -1; // Couldn't open file
    }
    // Allocate video frame
    pFrame=av_frame_alloc();
    
    AVFrame* pFrameYUV = av_frame_alloc();
    if( pFrameYUV == NULL )
     {
    	printf("pFrameYUV  = -1 failed\n");
    	return -1; // Couldn't open file
    }
    

	printf("width = %d, height = %d\n", pCodecCtx->width,  pCodecCtx->height);

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

	 printf("sws_ctx %d, %d, %d, %d, %d\n",pCodecCtx->width,
	     pCodecCtx->height,
	     pCodecCtx->pix_fmt,
	     pCodecCtx->width,
	     pCodecCtx->height);
    
    int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                  pCodecCtx->height);
	printf("numBytes %d\n", numBytes);
    uint8_t* buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    avpicture_fill((AVPicture *)pFrameYUV, buffer, AV_PIX_FMT_YUV420P,
                   pCodecCtx->width, pCodecCtx->height);
    
    // Read frames and save first five frames to disk
    i=0;
    


    int index = 0;
    while(av_read_frame(pFormatCtx, &packet)>=0) {
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
            // Decode video frame
            YuvFramePtr frame(new YuvFrame());
            avcodec_decode_video2(pCodecCtx, frame->frame.get(), &frameFinished,
                                  &packet);
            SDl2Displayer_->push(frame);
			
            SDL_Delay(30);
            //Sleep(500);
            //if(index++ > 10)
            //break;
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
	RTMP_LogPrintf("SDL_Init\n");
    
	RTMP_LogPrintf("ffmpegSdl2\n");
	//ffmpegSdl2("/mnt/hgfs/ffmpeg/rtmp/rtmp-aduio-player/test/receive.flv");
	//ffmpegSdl2("/mnt/hgfs/ffmpeg/rtmp/rtmp-aduio-player/test/source.200kbps.768x320.flv");
	//return 0;
	av_register_all();
	//if(SDL_Init(SDL_INIT_VIDEO )) 
	{
    	fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    	//exit(1);
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

	
	while(1)
	{
		sleep(1);
	}

	

   // ffmpegSdl2("/mnt/hgfs/qingfu/ffmpeg/qt_project/rtmp-aduio-player/test/source.200kbps.768x320.flv");
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

