#ifndef _RTMP_PARSE_HPP_
#define _RTMP_PARSE_HPP_

#include "player.h"
#include "thread.hpp"
#include "audio_dec.hpp"
#include "media_player.hpp"

class RtmpParser:public Thread
{
public:
	// 
	RtmpParser(const char *url);
	bool init();
	bool inputData(const char *pcData, int iLen);	// 只管数据进来,由内部线程来parse数据
	// 设置环形buffer
	void onGetAAC(const uint8_t *pcData, int iLen, uint32_t ui32TimeStamp);
	void onLoop();							//内部线程
private:
	int m_iSampleRate = 44100;
	int m_iSampleBit = 16;
	int m_iChannel = 1;
	AdtsFrame m_adts;
	char rtmpUrl_[256];
	MediaPlayer *mediaPlayer_;
};
#endif