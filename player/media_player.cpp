#include "media_player.hpp"
#include "log_util.hpp"
#include "err_code_define.h"

MediaPlayer::MediaPlayer(RtmpParser *rtmpParser)
{
	audioDec_ = nullptr;
	alsaPcm_ = nullptr;
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


