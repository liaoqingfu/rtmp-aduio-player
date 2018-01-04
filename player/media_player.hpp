#ifndef _MEDIA_PLAYER_H_
#define _MEDIA_PLAYER_H_

#include <iostream>
#include "player.h"
#include "thread.hpp"
#include "audio_dec.hpp"

#include "alsa_pcm.hpp"
//using namespace std;
#include "rtmp_parse.hpp"
class H264Decoder;
class MediaPlayer: public Thread
{
public:
	//typedef std::shared_ptr<MediaPlayer> Ptr;
	MediaPlayer(){rtmpParser_ = nullptr;alsaPcm_ = nullptr;audioDec_ = nullptr; videoDec_ = nullptr;}
	MediaPlayer(RtmpParser *rtmpParser);
	~MediaPlayer();
	void play(const char* strUrl) ;					// 播放接口
	void pause(bool bPause) ;
	void onLoop();									// 维护一个命令队列
	void onAAC(const AdtsFrame &data);
	void onH264(const H264Frame &data);
	//std::shared_ptr<AudioDec> _aacDec;////aac软件解码器
	AudioDec *audioDec_;
	H264Decoder *videoDec_;
	AlsaPcm *alsaPcm_;
	RtmpParser *rtmpParser_;
};

#endif
