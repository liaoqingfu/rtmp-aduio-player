#ifndef _ALSA_PCM_H_
#define _ALSA_PCM_H_
#include <alsa/asoundlib.h>
#include "tb_buffer.hpp"
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include "thread.hpp"

class AlsaPcm
        :public Thread
{
public:
    AlsaPcm();
    virtual ~AlsaPcm();

    virtual int Push(void * data, int len);
    int Push(TBBuffer *buf);
    virtual int IsCanPush();
    virtual int GetVolume();
    virtual int SetVolume(int volume );
    void setAudioParams(int &fmt, unsigned int &samplerate, unsigned short &channels);
    void onLoop();
    int GetAlsaQueueSize();
    int GetPcmWriteCount();
    
protected:
    int Init();
    int Release();
    int InitAlsa();
    int ReleaseAlsa();
    int PlayFrame();
    int InitMixer();
    
private:
    snd_pcm_t* pcm_handle_;
    snd_pcm_hw_params_t* pcm_params_;
    snd_pcm_sw_params_t* pcm_params_sw_;
    snd_mixer_t* mixer_handle_;
    int fmt_;
    int sample_rate_;
    int channels_;
    int volume_;

    typedef boost::shared_ptr<TBBuffer> TBBufferPtr;
    typedef std::queue<TBBufferPtr> BufQueue;
    BufQueue buf_queue_;
    boost::mutex buf_queue_mutex_;

    bool running_;
    bool cacheing_;
    int cache_min_size_;
    int cache_max_size_;
	int mad_push_count_;
    int pcm_write_count_;
	FILE* ifile = NULL;
};
#endif