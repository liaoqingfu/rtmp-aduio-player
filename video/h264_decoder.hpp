#ifndef _H264_DECODER_H_
#define _H264_DECODER_H_
#include <string>
#include <memory>
#include <stdexcept>
#include <thread>
#include "SDl2Displayer.hpp"

#include "log_util.hpp"
#ifdef __cplusplus
extern "C" {
#endif 
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif 



using namespace std;

class YuvFrame{
public:
	YuvFrame() {
		frame.reset(av_frame_alloc(), [](AVFrame *pFrame) {
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
class H264Decoder
{
public:
	H264Decoder(void){
		yuvDisplayer_ = nullptr;
		avcodec_register_all();
        av_log_set_level(AV_LOG_QUIET);
		AVCodec *pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if (!pCodec) {
			throw std::runtime_error("avcodec_find_decoder failed");
		}

        pCodec->capabilities |= AV_CODEC_CAP_SLICE_THREADS | AV_CODEC_CAP_FRAME_THREADS ;
		m_pContext.reset(avcodec_alloc_context3(pCodec), [](AVCodecContext *pCtx) {
			avcodec_close(pCtx);
			avcodec_free_context(&pCtx);
		});
		if (!m_pContext) {
			throw std::runtime_error("avcodec_alloc_context3 failed");
		}
        m_pContext->thread_count = thread::hardware_concurrency();
		m_pContext->refcounted_frames = 1;
		if (pCodec->capabilities & CODEC_CAP_TRUNCATED) {
			/* we do not send complete frames */
			m_pContext->flags |= CODEC_FLAG_TRUNCATED;
		}
		if(avcodec_open2(m_pContext.get(), pCodec, NULL)< 0){
			throw std::runtime_error("avcodec_open2 failed");
		}
	}
	~H264Decoder(void){}
	std::shared_ptr<YuvFrame> inputVideo(unsigned char* data,unsigned int dataSize,uint32_t dts, uint32_t pts)
	{
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = data;
		pkt.size = dataSize;
		pkt.dts = dts;
		pkt.pts = pts;
		int iGotPicture ;
		std::shared_ptr<YuvFrame> frame(new YuvFrame());// = m_pool.obtain();
		//LogDebug("ctx = %x", m_pContext.get());
		//int iLen;
		auto iLen = avcodec_decode_video2(m_pContext.get(), frame->frame.get(), &iGotPicture, &pkt);
		if (!iGotPicture || iLen < 0) {
			//m_pool.quit(frame);
			//LogDebug("iGotPicture");
			return nullptr;
		}
		//LogDebug("return frame");
		if(!yuvDisplayer_)
		{
			yuvDisplayer_ = new YuvDisplayer();
		}
		yuvDisplayer_->displayYUV(frame->frame.get());
		return frame;
	}
private:
	std::shared_ptr<AVCodecContext> m_pContext;
	YuvDisplayer *yuvDisplayer_;
	//ResourcePool<YuvFrame> m_pool;
};

#endif /* _H264_DECODER_H_ */

