#ifndef SRC_PLAYER_MEDIAPLAYER_H_
#define SRC_PLAYER_MEDIAPLAYER_H_

class MediaPlayer 
{
	typedef std::shared_ptr<MediaPlayer> Ptr;

	MediaPlayer();
	virtual ~MediaPlayer();
	void play(const char* strUrl) ;					// 播放接口
	void pause(bool bPause) ;
	void onLoop();									// 维护一个命令队列

	std::shared_ptr<AudioDec> _aacDec;////aac软件解码器
};

#endif
