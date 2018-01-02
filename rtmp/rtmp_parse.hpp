#ifndef _RTMP_PARSE_HPP_
#define _RTMP_PARSE_HPP_

#include "player.h"

class RtmpParser{
public:
	// 
	bool init();
	bool inputData(const char *pcData, int iLen);	// 只管数据进来,由内部线程来parse数据
	// 设置环形buffer
	void onGetAAC(const char *pcData, int iLen, uint32_t ui32TimeStamp);
	int onLoop();							//内部线程
private:
	int m_iSampleRate = 44100;
	int m_iSampleBit = 16;
	int m_iChannel = 1;
	AdtsFrame m_adts;
};
#endif