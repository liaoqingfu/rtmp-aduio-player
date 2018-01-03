#include "media_player.hpp"
#include "log_util.hpp"
#include "err_code_define.h"
MediaPlayer::MediaPlayer()
{
	audioDec_ = nullptr;
	alsaPcm_ = nullptr;
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
	}
	// 发送给pcm buffer队列
    //FunExit();
}


