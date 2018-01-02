#include "media_player.hpp"
MediaPlayer::MediaPlayer()
{

}
MediaPlayer::~MediaPlayer()
{}
void MediaPlayer::play(const char* strUrl)                  // 播放接口
{}
void MediaPlayer::pause(bool bPause) 
{}
void MediaPlayer::onLoop()
{}

void MediaPlayer::onAAC(const AdtsFrame &data) 
{

/*
    if (!_aacDec) {
        _aacDec.reset(new AudioDec());
        _aacDec->Init(data.data);
    }
    uint8_t *pcm;
    int pcmLen = _aacDec->InputData(data.data, data.aac_frame_length, &pcm);
    if(pcmLen){
           // _pcmBuf.append((char *)pcm,pcmLen);
    }
    // 发送给pcm buffer队列
    */
}


