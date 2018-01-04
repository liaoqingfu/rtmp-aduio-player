#include "media_player.hpp"
#include "log_util.hpp"
#include "err_code_define.h"
#include "h264_decoder.hpp"

MediaPlayer::MediaPlayer(RtmpParser *rtmpParser)
{
	audioDec_ = nullptr;
	alsaPcm_ = nullptr;
	videoDec_ = nullptr;
	rtmpParser_ = rtmpParser;
}

MediaPlayer::~MediaPlayer()
{
	if(audioDec_)
	{
		delete audioDec_;
	}
}
void MediaPlayer::play(const char* strUrl)                  // 播放接口
{}
void MediaPlayer::pause(bool bPause) 
{}
void MediaPlayer::onLoop()
{}

void MediaPlayer::onAAC(const AdtsFrame &data) 
{
    //FunEntry();
	if (!audioDec_) 
	{
		audioDec_ = new AudioDec();
		audioDec_->Init(data.data);
	}
	uint8_t *pcm;
	//LogDebug("aac_frame_length = %d", data.aac_frame_length);
	int pcmLen = audioDec_->InputData(data.data, data.aac_frame_length, &pcm);
	if(pcmLen)
	{
	    //LogDebug("alsa alsaPcm_ = 0x%x", alsaPcm_);
		if(!alsaPcm_)
		{
			 alsaPcm_ = new AlsaPcm();
		}
		
		//LogDebug("Push pcmLen = %d", pcmLen);
	    if(alsaPcm_->Push(pcm, pcmLen) != E_OK)
		{
			LogError("alsa buffer full");
		}
		int fmt = SND_PCM_FORMAT_S16_LE;
		unsigned int  samplerate = audioDec_->getSamplerate();
		unsigned short channels = audioDec_->getChannels();

		if(rtmpParser_)
		{
            samplerate = rtmpParser_->m_iSampleRate;
            channels  = rtmpParser_->m_iChannel;
		}
		
		alsaPcm_->setAudioParams(fmt, samplerate, channels);
	}
	// 发送给pcm buffer队列
    //FunExit();
}



void MediaPlayer::onH264(const H264Frame &data) {

    if (!videoDec_) 
    {
        LogDebug("new H264Decoder()");
        videoDec_ = new H264Decoder();
    }
    if (!videoDec_) {
        LogDebug("videoDec_ = NULL");
 
        return;
    }
    //解码器已经OK
    //_h264Parser->inputH264(data.data, data.timeStamp);
    // pts 先不处理
    LogDebug("videoDec_ = %x", videoDec_);
    auto frame = videoDec_->inputVideo((uint8_t *)data.data.data(), data.data.size(), data.timeStamp, data.timeStamp);
    if (!frame) {
        LogDebug("get frame failed");
        return;
    }
    frame->dts = frame->frame->pkt_dts;
    frame->pts = frame->frame->pts;
    FunExit();
    //onDecoded(frame);
    // 将解码后的帧发给队列
}


