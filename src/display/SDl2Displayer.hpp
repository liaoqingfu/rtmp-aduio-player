/*
 * MIT License
 *
 * Copyright (c) 2017 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef YUVDISPLAYER_H_
#define YUVDISPLAYER_H_
#include <stdexcept>

#ifdef __cplusplus
extern "C" {
#endif
#include "SDL2/SDL.h"
#include "libavcodec/avcodec.h"
#ifdef __cplusplus
}
#endif

#include "h264_decoder.hpp"
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "thread.hpp"

using namespace std;


class YuvDisplayer :public Thread{
public:
	YuvDisplayer(void *hwnd = nullptr,const char *title = "video"){
		_title = title;
		_hwnd = hwnd;
		
		run();
	}
	virtual ~YuvDisplayer(){
		if (_texture) {
			SDL_DestroyTexture(_texture);
			_texture = nullptr;
		}
		if (_render) {
			SDL_DestroyRenderer(_render);
			_render = nullptr;
		}
		if (_win) {
			SDL_DestroyWindow(_win);
			_win = nullptr;
		}
	}
	void onLoop()						//内部线程
	{
		int i = 0;
		YuvFramePtr  frame;
		while (!isShutdown())
    	{
        	usleep(20000);
			// 从线程中去队列
			if (!buf_queue_.empty())
            {
                boost::mutex::scoped_lock lock(buf_queue_mutex_);
                frame = buf_queue_.front();
                buf_queue_.pop();
            }
			else
			{
				continue;
			}
			displayYUV(frame->frame.get());		// 是SDL2的问题，导致播放一段时间后就变为灰色
			//MyWriteJPEG(frame->frame.get(), (frame->frame.get())->width, 
			//		(frame->frame.get())->height, i++);
			frame.reset();
			//printf("displayYUV\n");
		}
	}

	void push(YuvFramePtr frame)
	{
		boost::mutex::scoped_lock lock(buf_queue_mutex_);
		buf_queue_.push(frame);
	}
	int MyWriteJPEG(AVFrame* pFrame, int width, int height, int iIndex)  
	{  
	    // 输出文件路径  
	    char out_file[256] = {0};  
	    sprintf(out_file, "%05d.jpg",  iIndex);  
	    //LogDebug("out_file = %s", out_file);  
	    // 分配AVFormatContext对象  
	    AVFormatContext* pFormatCtx = avformat_alloc_context();  
	    //LogDebug("L:%d", __LINE__); 
	    // 设置输出文件格式  
	    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);  
	    // 创建并初始化一个和该url相关的AVIOContext  
	    if( avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {  
	        printf("Couldn't open output file.");  
	        return -1;  
	    }  
	     //LogDebug("L:%d", __LINE__);  
	    // 构建一个新stream  
	    AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);  
	    if( pAVStream == NULL ) {  
			LogError("L:%d", __LINE__);
	        return -1;  
	    }  
	      
	    // 设置该stream的信息  
	    AVCodecContext* pCodecCtx = pAVStream->codec;  
	    //LogDebug("L:%d  %x %x", __LINE__, pCodecCtx, pFormatCtx->oformat);     
	    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;  
	    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;  
	    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;  
	    pCodecCtx->width = width;  
	    pCodecCtx->height = height;  
	    pCodecCtx->time_base.num = 1;  
	    pCodecCtx->time_base.den = 25;  
	    //LogDebug("L:%d", __LINE__);   
	    // Begin Output some information  
	    av_dump_format(pFormatCtx, 0, out_file, 1);  
	    // End Output some information  
	    //LogDebug("L:%d", __LINE__);   
	    // 查找解码器  
	    AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);  
	    if( !pCodec ) {  
	        printf("Codec not found.");  
	        return -1;  
	    }  
	    // 设置pCodecCtx的解码器为pCodec  
	    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ) {  
	        printf("Could not open codec.");  
	        return -1;  
	    }  
	    //LogDebug("L:%d", __LINE__);   
	    //Write Header  
	    avformat_write_header(pFormatCtx, NULL);  
	     //LogDebug("L:%d", __LINE__);  
	    int y_size = pCodecCtx->width * pCodecCtx->height;  
	      
	    //Encode  
	    // 给AVPacket分配足够大的空间  
	    AVPacket pkt;  
	    av_new_packet(&pkt, y_size * 3);  
	      //LogDebug("L:%d", __LINE__); 
	    //   
	    int got_picture = 0;  
	    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);  
	    if( ret < 0 ) {  
	        printf("Encode Error.\n");  
	        return -1;  
	    }  
	    if( got_picture == 1 ) {  
	        //pkt.stream_index = pAVStream->index;  
	        ret = av_write_frame(pFormatCtx, &pkt);  
	    }  
	  	//LogDebug("L:%d", __LINE__);  
	    av_free_packet(&pkt);  
	  
	    //Write Trailer  
	    av_write_trailer(pFormatCtx);  
	  
	    //printf("Encode Successful.\n");  
	  
	    if( pAVStream ) {  
	        avcodec_close(pAVStream->codec);  
	    }  
	    avio_close(pFormatCtx->pb);  
	    avformat_free_context(pFormatCtx);  
	      
	    return 0;  
	}  
	bool displayYUV(AVFrame *pFrame){
		static int frameCount = 0;
		if (!_win) {
			if (_hwnd) {
				_win = SDL_CreateWindowFrom(_hwnd);
			}else {
				_win = SDL_CreateWindow(_title.data(),
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					pFrame->width,	pFrame->height,
					SDL_WINDOW_SHOWN);//SDL_WINDOW_OPENGL);
			}
		}
		if (_win && ! _render){
                        #if 0
            SDL_RENDERER_SOFTWARE = 0x00000001,         /**< The renderer is a software fallback */
            SDL_RENDERER_ACCELERATED = 0x00000002,      /**< The renderer uses hardware
                                                             acceleration */
            SDL_RENDERER_PRESENTVSYNC = 0x00000004,     /**< Present is synchronized
                                                             with the refresh rate */
            SDL_RENDERER_TARGETTEXTURE = 0x00000008     /**< The renderer supports
                                                             rendering to texture */
                #endif
                #if defined(__linux__)
             		_render = SDL_CreateRenderer(_win, -1, SDL_RENDERER_ACCELERATED);
                #else
                    _render = SDL_CreateRenderer(_win, -1, SDL_RENDERER_ACCELERATED);
                #endif
		}
		if (_render && !_texture) {
			_texture = SDL_CreateTexture(_render, SDL_PIXELFORMAT_YV12,
				SDL_TEXTUREACCESS_STREAMING,
				pFrame->width,
				pFrame->height);
		}
        if (_texture) {
			
            SDL_UpdateYUVTexture(_texture, nullptr,
                pFrame->data[0], pFrame->linesize[0],
                pFrame->data[1], pFrame->linesize[1],
                pFrame->data[2], pFrame->linesize[2]);
			//printf("w = %d, h = %d %d %d %d\n", pFrame->width,	pFrame->height,
			//	pFrame->linesize[0],pFrame->linesize[1],pFrame->linesize[2]);
            //SDL_UpdateTexture(_texture, nullptr, pFrame->data[0], pFrame->linesize[0]);
            SDL_RenderClear(_render);
            SDL_RenderCopy(_render, _texture, nullptr, nullptr);
            SDL_RenderPresent(_render);

			if(frameCount++ > 10000)
			{
			frameCount = 0;
			SDL_DestroyTexture(_texture);  
			_texture = nullptr;
			SDL_DestroyRenderer(_render);
			_render = nullptr;
			SDL_DestroyWindow(_win);  
			_win = nullptr;
			}
			//SDL_DestroyRenderer(_render);  
    		
    		
			
            return true;
        }
		return false;
	}
private:
	//typedef std::shared_ptr<YuvFrame> YuvFramePtr;
	typedef std::queue<YuvFramePtr> BufQueue;
    BufQueue buf_queue_;
    boost::mutex buf_queue_mutex_;
	
	string _title;
	SDL_Window *_win = nullptr;
	SDL_Renderer *_render = nullptr;
	SDL_Texture *_texture = nullptr;
	void *_hwnd = nullptr;
};

#endif /* YUVDISPLAYER_H_ */
