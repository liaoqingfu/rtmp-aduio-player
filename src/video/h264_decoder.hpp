#ifndef _H264_DECODER_H_
#define _H264_DECODER_H_
#include <string>
#include <memory>
#include <stdexcept>
#include <thread>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "log_util.hpp"
#ifdef __cplusplus
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



using namespace std;

class YuvFrame{
public:
	YuvFrame() {
		frame.reset(av_frame_alloc(), [](AVFrame *pFrame) {
			//printf("av_frame_free\n");
			av_frame_unref(pFrame);
			av_frame_free(&pFrame);
		});
	}
	~YuvFrame() {
	}
	std::shared_ptr<AVFrame> frame;
	
	uint32_t dts;
	uint32_t pts;
};

typedef std::shared_ptr<YuvFrame> YuvFramePtr;

class H264Decoder
{
public:
	H264Decoder(void){
		
		avcodec_register_all();
        av_log_set_level(AV_LOG_FATAL);
		AVCodec *pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if (!pCodec) {
			throw std::runtime_error("avcodec_find_decoder failed");
		}
		

        //pCodec->capabilities |= AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS ;
		m_pContext.reset(avcodec_alloc_context3(pCodec), [](AVCodecContext *pCtx) {
			avcodec_close(pCtx);
			avcodec_free_context(&pCtx);
		});
		if (!m_pContext) {
			throw std::runtime_error("avcodec_alloc_context3 failed");
		}
        //m_pContext->thread_count = thread::hardware_concurrency();
        //表示该frame的引用计数，当这个值为1时，表示有另外一帧将该帧用作参考帧，而且参考帧返回给调用者；当参考完成时，调用者需要调用av_frame_unref()方法解除对该帧的参考；
		 m_pContext->refcounted_frames = 1;
		if (pCodec->capabilities & CODEC_CAP_TRUNCATED) {
			/* we do not send complete frames */
			m_pContext->flags |= CODEC_FLAG_TRUNCATED;
		}
		if(avcodec_open2(m_pContext.get(), pCodec, NULL)< 0){
			throw std::runtime_error("avcodec_open2 failed");
		}
	}
	~H264Decoder(void){
		if(bmp)
			SDL_DestroyTexture(bmp);
	}

	int ffmpegSdl2(AVCodecContext  *pCodecCtx,   AVFrame         *pFrame) {
	   
	    SDL_Rect        rect;

		if(!pFrameYUV)
	   		pFrameYUV = av_frame_alloc();
	    if( pFrameYUV == NULL )
	        return -1;
	    

		printf("width = %d, height = %d\n", pCodecCtx->width,  pCodecCtx->height);
		if(!screen)
	    {
			screen = SDL_CreateWindow("Video Window",
	                              SDL_WINDOWPOS_UNDEFINED,
	                              SDL_WINDOWPOS_UNDEFINED,
	                              800,  480,
	                              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
		}
	    
	   	if(!renderer)
	    	renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	    if (!renderer) {
	        av_log(NULL, AV_LOG_WARNING, "Failed to initialize a hardware accelerated renderer: %s\n", SDL_GetError());
	        renderer = SDL_CreateRenderer(screen, -1, 0);
	    }

		if(!bmp)
	    	bmp = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);
	    //SDL_SetTextureBlendMode(bmp,SDL_BLENDMODE_BLEND );
	    if(!sws_ctx)
	    {
		    sws_ctx = sws_getContext
		    (
		     pCodecCtx->width,
		     pCodecCtx->height,
		     pCodecCtx->pix_fmt,
		     pCodecCtx->width,
		     pCodecCtx->height,
		     AV_PIX_FMT_YUV420P,
		     SWS_FAST_BILINEAR,
		     NULL,
		     NULL,
		     NULL
		     );
		}
	    
	    
	    if(!buffer_)
		{
			int numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
	                                  pCodecCtx->height);
			printf("numBytes %d\n", numBytes);
			buffer_ = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
			
	    }
	    avpicture_fill((AVPicture *)pFrameYUV, buffer_, AV_PIX_FMT_YUV420P,
	                   pCodecCtx->width, pCodecCtx->height);
	    
	    // Read frames and save first five frames to disk
	    
	    rect.x = 0;
	    rect.y = 0;
	    rect.w = pCodecCtx->width;
	    rect.h = pCodecCtx->height;
		printf("sws_scale %d\n", rect.w);
        sws_scale(
			sws_ctx,
			(uint8_t const * const *)pFrame->data,
			pFrame->linesize,
			0,
			pCodecCtx->height,
			pFrameYUV->data,
			pFrameYUV->linesize
			);
		printf("frame %d %d %d,  pFrameYUV %d %d %5d\n",
			pFrame->linesize[0] , pFrame->linesize[1] , pFrame->linesize[2],
			pFrameYUV->linesize[0] , pFrameYUV->linesize[1] , pFrameYUV->linesize[2]);


		SDL_Surface * tempSurface = SDL_LoadBMP("/mnt/hgfs/ffmpeg/rtmp/rtmp-aduio-player/test/car.bmp");
		if(tempSurface==NULL)
		{
			fprintf(stderr, "SDL_LoadBMP() failed: %s", SDL_GetError());
			exit(-1);
		}

		SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
		SDL_FreeSurface(tempSurface);
	
        ////iPitch 计算yuv一行数据占的字节数
        SDL_RenderClear( renderer );
        //SDL_UpdateTexture( bmp, &rect, pFrameYUV->data[0], pFrameYUV->linesize[0] );
        //
        //SDL_RenderCopy( renderer, bmp, &rect, &rect );
		SDL_RenderCopy( renderer, texture, &rect, &rect );
        SDL_RenderPresent( renderer );
	    SDL_DestroyTexture(texture);      
    
    return 0;
}
	YuvFramePtr inputVideo(unsigned char* data,unsigned int dataSize,uint32_t dts, uint32_t pts)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = data;
		pkt.size = dataSize;
		pkt.dts = dts;
		pkt.pts = pts;
		int iGotPicture ;
		YuvFramePtr frame(new YuvFrame());// = m_pool.obtain();
		// LogDebug("pix_fmt = 0x%x", (m_pContext.get())->pix_fmt);
		//int iLen;
		auto iLen = avcodec_decode_video2(m_pContext.get(), frame->frame.get(), &iGotPicture, &pkt);
		if (!iGotPicture || iLen < 0) {
			//m_pool.quit(frame);
			//LogDebug("iGotPicture");
			return nullptr;
		}
		//LogDebug("return frame");
		frame->pts = pts;
		
		//ffmpegSdl2(m_pContext.get(),  frame->frame.get());
		//yuvDisplayer_->displayYUV(frame->frame.get());
		return frame;
	}
private:
	std::shared_ptr<AVCodecContext> m_pContext;
	
	struct SwsContext *sws_ctx = nullptr;
	 AVDictionary    *optionsDict = nullptr;
	    
    //SDL_CreateTexture();
    SDL_Texture    *bmp = nullptr;
    SDL_Window     *screen = nullptr;
	SDL_Renderer *renderer = nullptr;
	uint8_t* buffer_ = nullptr;
	AVFrame* pFrameYUV= nullptr;
};

#endif /* _H264_DECODER_H_ */

