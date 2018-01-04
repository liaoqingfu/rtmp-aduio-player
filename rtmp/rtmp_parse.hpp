#ifndef _RTMP_PARSE_HPP_
#define _RTMP_PARSE_HPP_

#include "player.h"
#include "thread.hpp"
#include "audio_dec.hpp"
#include <iostream>
using namespace std;
class MediaPlayer;
class RtmpParser:public Thread
{
public:
	// 
	RtmpParser(const char *url);
	bool init();
	bool inputData(const char *pcData, int iLen);	// 只管数据进来,由内部线程来parse数据
	// 设置环形buffer
	void onGetAAC(const uint8_t *pcData, int iLen, uint32_t ui32TimeStamp);
	void _onGetH264(const char *pcData, int iLen, uint32_t ui32TimeStamp);
	void onGetH264(const char *pcData, int iLen, uint32_t ui32TimeStamp);
	void onLoop();							//内部线程
	bool containAudio() const {
		return m_bHaveAudio;
	}
	bool containVideo () const {
		return m_bHaveVideo;
	}
	float getDuration() const {
		return m_fDuration;
	}
	int m_iSampleRate = 44100;
	int m_iSampleBit = 16;
	int m_iChannel = 2;
private:
	// video
	H264Frame m_h264frame;
	// audio
	AdtsFrame m_adts;
	char rtmpUrl_[256];
	MediaPlayer *mediaPlayer_;

	string m_strSPS;
	string m_strPPS;
	int m_iVideoWidth = 0;
	int m_iVideoHeight = 0;
	float m_fVideoFps = 0;
	bool m_bHaveAudio = false;
	bool m_bHaveVideo = false;
	float m_fDuration = 0;
};
#endif