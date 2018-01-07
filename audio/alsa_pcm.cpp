#include "alsa_pcm.hpp"
#include "log_util.hpp"

#include "err_code_define.h"

#include <boost/bind.hpp>
#include <alsa/asoundlib.h>

using namespace std;

#define BufQuqueScopeLock boost::mutex::scoped_lock lock(buf_queue_mutex_)

AlsaPcm::AlsaPcm()
    :pcm_handle_(0)
    ,pcm_params_(0)
    ,pcm_params_sw_(0)
    ,mixer_handle_(0)
    ,volume_(40)
    ,cacheing_(true)
    ,cache_min_size_(20)
    ,cache_max_size_(100)
    ,mad_push_count_(0)
{
    InitAlsa();
    Init();
    //SetVolume(100);
    ifile = fopen("temp.pcm", "wb");
    if(!ifile)
        LogError("fopen failed"); 
}

AlsaPcm::~AlsaPcm()
{
    FunEntry();
    Release();
    ReleaseAlsa();
    if(ifile)
    fclose(ifile);
    FunExit();
}

int AlsaPcm::IsCanPush()
{
    return cache_max_size_ - GetAlsaQueueSize();
}

int AlsaPcm::Push(void * data, int len)
{
    int ret = E_OK;
    int writeLen = 0;
    static int err_count = 0;
    // FunEntry();
    if((writeLen=fwrite(data, 1, len, ifile)) != len)
    {
        LogError("writeLen = %d, len = %d", writeLen, len);    
    }
    if(GetAlsaQueueSize() < cache_max_size_ )
    {
        TBBufferPtr buff(TBBuffer::CreateInstance(len));
        if(!buff->Add((uint8_t *)data,len))
		{
			LogError("new buffer failed");
			return E_ERR;
        }
        BufQuqueScopeLock;
        buf_queue_.push(buff);
        err_count = 0;
        mad_push_count_++;
        //LogInfo("buf queue size:%d, len = %d\n", buf_queue_.size(), len);
    }
    else
    {
        ret = E_ERR;
        LogInfo("queueSize = %d/%d overflow", err_count, GetAlsaQueueSize());
        if(++err_count > cache_max_size_ )
        {
            LogError("overflow too match,err:%d/%d, mad_push_count = %d, pcm_write_count = %d", 
                    err_count , cache_max_size_, mad_push_count_, pcm_write_count_);
        }
    }
    //FunExit();
    return ret;
}

int AlsaPcm::Push(TBBuffer *buf)
{
    int ret = E_OK;
    if(GetAlsaQueueSize() <= cache_max_size_ )
    {
        BufQuqueScopeLock;
        buf_queue_.push(TBBufferPtr(buf->Clone()));
        //LogInfo("buf queue size:%d\n", buf_queue_.size());
    }
    else
    {
        ret = E_ERR;
        //LogInfo("queueSize = %d overflow", GetAlsaQueueSize());
    }
    return ret;
}

int AlsaPcm::GetVolume()
{
    if (mixer_handle_ == NULL)
    return 0;

    long ll = 0, lr = 0;
    //处理事件
    snd_mixer_handle_events(mixer_handle_);

    snd_mixer_elem_t *elem = NULL;
    for(elem=snd_mixer_first_elem(mixer_handle_); elem; elem=snd_mixer_elem_next(elem))
    {
        if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE && snd_mixer_selem_is_active(elem))
        {
#if defined(_X86)
            if(std::string(snd_mixer_selem_get_name(elem)) == "Master")
#else
            if(std::string(snd_mixer_selem_get_name(elem)) == "PCM")
#endif
            {
                //左声道
                snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &ll);
                //右声道
                snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &lr);

                break;
            }
        }
    }

    volume_ = (ll + lr) >> 1;
        return volume_;
}

int AlsaPcm::SetVolume(int volume )
{
        int ret = E_OK;
    if (mixer_handle_ == NULL )
    {
            return 0;
    }

    snd_mixer_elem_t *elem = NULL;
    long vol_min = 0, vol_max = 0;
    volume_ = volume;
    for(elem=snd_mixer_first_elem(mixer_handle_); elem; elem=snd_mixer_elem_next(elem))
    {
        if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE && snd_mixer_selem_is_active(elem))
        {
#if defined(_X86)
            if(std::string(snd_mixer_selem_get_name(elem)) == "Master")
#else
            if(std::string(snd_mixer_selem_get_name(elem)) == "PCM")
#endif
            {
#if 0
                LogError ("setvolume =  %d", volume);
                snd_mixer_selem_set_playback_volume_range(elem, 0, 100);
                snd_mixer_selem_set_playback_volume_all(elem, (long)volume);
                volume_ = volume;
#else
                snd_mixer_selem_get_playback_volume_range(elem, &vol_min, &vol_max);
                volume = (volume_ * vol_max) / 100;
                LogDebug("vol range: [%d - %d] , cur vol : %d", vol_min, vol_max, volume);

                snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, (long)volume);
                snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT,(long)volume);
#endif

            }
        }
    }
    return ret;
}
int AlsaPcm::Init()
{
    pcm_write_count_ = 0;
    run();
}

int AlsaPcm::Release()
{
    FunEntry();
    running_  = false;
    cacheing_ = true;
    shutdown();
    join();
    FunExit();
}

void AlsaPcm::onLoop()
{
    PlayFrame();
}

int AlsaPcm::InitAlsa()
{
    int ret = 0, openCnt = 0;
    ret = snd_pcm_open(&pcm_handle_, "default", SND_PCM_STREAM_PLAYBACK, 0);
  
    if(ret < 0)
    {
        LogError ("open PCM device failed  %s !", snd_strerror(ret));
        pcm_handle_ = NULL;
        return -1;
    }

    int fmt = SND_PCM_FORMAT_S16_LE;
    unsigned int rate = 16000;
    unsigned short channel = 2;
    cache_min_size_ = rate / 1000;
    setAudioParams(fmt, rate, channel);

    return ret;
}


int AlsaPcm::InitMixer()
{
    int ret = 0;
    ret = snd_mixer_open(&mixer_handle_, 0);
    if (ret < 0)
    {
        LogError("open Mixer device failed %s !", snd_strerror(ret));
        mixer_handle_ = NULL;
        return ret;
    }

    // Attach an HCTL to an opened mixer
    if ((ret = snd_mixer_attach(mixer_handle_, "default" )) < 0)
    {
        LogError("snd_mixer_attach error %d !", ret);
            snd_mixer_close(mixer_handle_);
            mixer_handle_ = NULL;
        return ret;
    }
    // 注册混音器
    if ((ret = snd_mixer_selem_register(mixer_handle_, NULL, NULL )) < 0)
    {
            LogError("snd_mixer_selem_register error %d !", ret);
            snd_mixer_close(mixer_handle_);
            mixer_handle_ = NULL;
        return ret;
    }
    // 加载混音器
    if ((ret = snd_mixer_load(mixer_handle_)) < 0)
    {
        LogError("snd_mixer_load error %d !", ret);
            snd_mixer_close(mixer_handle_);
            mixer_handle_ = NULL;
        return ret;
    }
}


int AlsaPcm::ReleaseAlsa()
{
    int ret = 0;

    if (pcm_params_sw_ != NULL)
    {
        snd_pcm_sw_params_free (pcm_params_sw_);
        pcm_params_sw_ = NULL;
    }

    if (pcm_params_ != NULL)
    {
        snd_pcm_hw_params_free (pcm_params_);
        pcm_params_ = NULL;
    }

    if (pcm_handle_ != NULL)
    {
        snd_pcm_drop(pcm_handle_);
        snd_pcm_close(pcm_handle_);
        pcm_handle_ = NULL;
    }

    if (mixer_handle_ != NULL)
    {
        snd_mixer_close(mixer_handle_);
        mixer_handle_ = NULL;
    }

    return ret;
}

int AlsaPcm::GetAlsaQueueSize()
{
    return buf_queue_.size();
}

/**
 * for recovery alsa
 * @param handle
 * @param err
 * @param caller    ,only for debug
 * @param pcm_write_count  ,only for debug
 * @return 
 */
static int xrun_recovery(snd_pcm_t *handle, int err, int caller, int pcm_write_count)
{
    static int s_snd_pcm_resume_try_count = 0;
    //if (verbose)
    //	printf("stream recovery\n");
    LogInfo("xrun_recovery error:%d, caller = %d, pcm_write_count = %d", err, caller, pcm_write_count);
    if (err == -EPIPE) 
    {	/* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
    } 
    else if (err == -ESTRPIPE) 
    {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
        {
            if(s_snd_pcm_resume_try_count++ > 5)
                break;
            sleep(1);	/* wait until the suspend flag is released */
        }    
        s_snd_pcm_resume_try_count = 0;
        if (err < 0) {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    }
	return err;
}

static int wait_for_poll(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
{
	unsigned short revents;

	while (1) {
		poll(ufds, count, -1);
		snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);
		if (revents & POLLERR)
			return -EIO;
		if (revents & POLLOUT)
			return 0;
	}
}
int AlsaPcm::GetPollDescriptors(snd_pcm_t *handle, int &count, struct pollfd **ufds)
{
    int err;
    count = snd_pcm_poll_descriptors_count(handle);
    if (count <= 0)
    {
        LogError("Invalid poll descriptors count\n");
        return count;
    }

    *ufds = (struct pollfd*) malloc(sizeof (struct pollfd) * count);
    //LogInfo("sizeof (struct pollfd) * count = %dbytes", sizeof (struct pollfd) * count);
    if (ufds == NULL)
    {
        LogError("No enough memory\n");
        return -ENOMEM;
    }

    if ((err = snd_pcm_poll_descriptors(handle, *ufds, count)) < 0)
    {
        LogError("Unable to obtain poll descriptors for playback: %s\n", snd_strerror(err));
        if (*ufds)
            free(*ufds);
        return err;
    }

    return 0;
}

int AlsaPcm::PlayFrame()
{
    struct pollfd *ufds;
    double phase = 0;
    signed short *ptr;
    int err, count, cptr, init;
    unsigned int channels = channels_;
    snd_pcm_sframes_t period_size;
    snd_pcm_t *handle = pcm_handle_;
    static int err_count = 0;
    TBBufferPtr buf;
    bool is_first_pcm_write = false;
    int cacheing_wait_count = 0;

    FunEntry();
    running_ = true;

    err = GetPollDescriptors(handle, count, &ufds);
    if (err != 0)
    {
        LogError("GetPollDescriptors failed");
        if (ufds)
            free(ufds);
        return err;
    }
    init = 1;
    cacheing_ = true;

    while (!isShutdown())
    {
        usleep(5000);

        if (!init)
        {
            err = wait_for_poll(handle, ufds, count);
            if (err < 0)
            {
                if (snd_pcm_state(handle) == SND_PCM_STATE_XRUN
                        || snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED)
                {
                    err = snd_pcm_state(handle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
                    if (xrun_recovery(handle, err, 0, pcm_write_count_) < 0)
                    {
                        LogError("Write error: %s\n", snd_strerror(err));
                    }

                    init = 1;
                }
                else
                {
                    LogError("Wait for poll failed\n");
                    return err;
                }
            }
        }

        if (cacheing_)
        {
            cacheing_wait_count++;

        }
        else
        {
            cacheing_wait_count = 0;
        }

        if (!buf)
        {
            if (!cacheing_)
            {
                if (!buf_queue_.empty())
                {
                    BufQuqueScopeLock;
                    buf = buf_queue_.front();
                    buf_queue_.pop();
                }
                else
                {
                    cacheing_ = true;
                    LogDebug("buf:%d/%d, mad_push_count = %d", buf_queue_.size(), cache_min_size_, mad_push_count_);
                    usleep(50000);
                    continue;
                }
            }
            else
            {
                LogDebug("Cacheing:%d/%d, mad_push_count = %d", buf_queue_.size(), cache_min_size_, mad_push_count_);
                if (buf_queue_.size() > cache_min_size_)
                {
                    cacheing_ = false;
                    //cache_min_size_ = 5;
                }
                else
                {
                    usleep(100000);
                }
                continue;
            }
        }
        if (!is_first_pcm_write) //only for debug
        {
            LogDebug("Cacheing ready:%d/%d, mad_push_count = %d", buf_queue_.size(), cache_min_size_, mad_push_count_);
        }

        if (buf)
        {
            ptr = (short int*) buf->Data();
            cptr = buf->Size() / (2 * channels);
        }
        else
        {
            LogError("underrun,queue size:%d....", buf_queue_.size());
        }

        while (cptr > 0)
        {
            if (!is_first_pcm_write) //only for debug
            {
                is_first_pcm_write = true;
                LogDebug("first pcm write, alsa_size = %d, mad_push_count_ = %d", buf_queue_.size(), mad_push_count_);
            }
            err = snd_pcm_writei(handle, ptr, cptr);
            pcm_write_count_++; //for debug
            if (err < 0)
            {
                LogError("Write error(%d): %s\n", err_count, snd_strerror(err));
                if (xrun_recovery(handle, err, 1, pcm_write_count_) < 0)
                {
                    LogError("xrun_recovery error: %s\n", snd_strerror(err));
                }
                init = 1;

                ++err_count;
                LogError("overflow too match,error count:%d.", err_count);
 
                break; /* skip one period */
            }
            else
            {
                err_count = 0;
            }

            buf.reset();
            if (snd_pcm_state(handle) == SND_PCM_STATE_RUNNING)
                init = 0;
            ptr += err * channels;
            cptr -= err;
            if (cptr == 0)
                break;
            /* it is possible, that the initial buffer cannot store */
            /* all data from the last period, so wait awhile */
            err = wait_for_poll(handle, ufds, count);
            if (err < 0)
            {
                if (snd_pcm_state(handle) == SND_PCM_STATE_XRUN
                        || snd_pcm_state(handle) == SND_PCM_STATE_SUSPENDED)
                {
                    err = snd_pcm_state(handle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
                    if (xrun_recovery(handle, err, 2, pcm_write_count_) < 0)
                    {
                        LogError("Write error: %s\n", snd_strerror(err));
                    }
                    init = 1;
                }
                else
                {
                    LogError("Wait for poll failed\n");
                    return err;
                }
            }
        }
    }

    if (ufds)
        free(ufds);
    while (!buf_queue_.empty())
    {
        buf_queue_.pop();
    }
    FunExit();
    return err;
}



void AlsaPcm::setAudioParams(int &fmt, unsigned int &samplerate, unsigned short &channels)
{
    int ret =0;
    unsigned int buffer_time;

    if (pcm_handle_ == NULL)
    {
        return;
    }

    if (channels == 1)
    {
        channels = 2; //将单声道转为双声道
    }

    if ( fmt_ == fmt && sample_rate_ == samplerate && channels_ == channels)
    {
        return;
    }
    cache_min_size_ = samplerate / 1000;

    LogInfo("sample rate: %d , channels: %d\n", samplerate, channels);

    if (pcm_params_ != NULL)
    {
        snd_pcm_hw_params_free (pcm_params_);
        pcm_params_ = NULL;
    }

    snd_pcm_hw_params_malloc(&pcm_params_);                     //分配params结构体
    ret = snd_pcm_hw_params_any(pcm_handle_, pcm_params_);      //初始化params
    if (ret < 0)
    {
        LogError("snd_pcm_hw_params_any error: %s !\n", snd_strerror(ret));
        return;
    }

    ret = snd_pcm_hw_params_set_access(pcm_handle_, pcm_params_, SND_PCM_ACCESS_RW_INTERLEAVED);    //初始化访问权限
    if (ret < 0)
    {
        LogError("snd_pcm_params_set_access error: %s !\n", snd_strerror(ret));
        return;
    }

    ret = snd_pcm_hw_params_set_format(pcm_handle_, pcm_params_, (snd_pcm_format_t)fmt);        //设置16位采样精度 
    if (ret < 0)
    {
        LogError("snd_pcm_params_set_format error: %s !\n", snd_strerror(ret));
        return;
    }

    fmt_ = fmt;

    ret = snd_pcm_hw_params_set_channels(pcm_handle_, pcm_params_, channels);                   //设置声道,1表示单声>道，2表示立体声        
    if (ret < 0)
    {
        LogError("snd_pcm_params_set_channels error: %s !\n", snd_strerror(ret));
        return;
    }

    channels_ = channels;

    int dir=0;

    ret = snd_pcm_hw_params_set_rate_near(pcm_handle_, pcm_params_, &samplerate, &dir);     //设置>频率
    if (ret < 0)
    {
        LogError("snd_pcm_hw_params_set_rate_near error: %s !\n", snd_strerror(ret));
        return;
    }
    sample_rate_ = samplerate;

   
	ret = snd_pcm_hw_params_get_buffer_time_max(pcm_params_, &buffer_time, NULL);
    if (ret < 0) {
        LogError("Unable to retrieve buffer time: %s\n", snd_strerror(ret));
        //return ret;
    }
    if (buffer_time > 500000)
    buffer_time = 500000;
	// set buffer time 
	ret = snd_pcm_hw_params_set_buffer_time_near(pcm_handle_, pcm_params_, &buffer_time, 0);
	if (ret < 0) {
		LogError("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(ret));
		//return err;
	}
	

    ret = snd_pcm_hw_params(pcm_handle_, pcm_params_);
    if (ret < 0)
    {
        LogError("snd_pcm_hw_params error: %s !\n", snd_strerror(ret));
        return;
    }


    if (pcm_params_sw_ != NULL)
    {
        snd_pcm_sw_params_free (pcm_params_sw_);
        pcm_params_sw_ = NULL;
    }

    snd_pcm_sw_params_malloc(&pcm_params_sw_);
   
    // set software parameters 
    ret = snd_pcm_sw_params_current(pcm_handle_, pcm_params_sw_);
    if (ret < 0)
    {
        LogError("unable to determine current software params: %s !\n", snd_strerror(ret));
    }

    snd_pcm_uframes_t frames = 2048;
    ret = snd_pcm_hw_params_get_period_size(pcm_params_, &frames, NULL);
    if (ret < 0)
    {
        LogError("unable get period size: %s !\n", snd_strerror(ret));
    }
    LogInfo("frames %d !\n", frames);
    
    //ret = snd_pcm_sw_params_set_start_threshold(pcm_handle_, pcm_params_sw_, frames*2);
    
    // 这个值对于不同的samplerate影响非常大?
    // 16384 适合4800
    // 16384/3 

    /*
    01-03 17:25:29 797 alsa_pcm.cpp::setAudioParams() - sample rate: 16000 , channels: 2
     
    01-03 17:25:29 799 alsa_pcm.cpp::setAudioParams() - frames 80 !
     
    01-03 17:25:29 799 alsa_pcm.cpp::PlayFrame() -  Entry...  
    01-03 17:25:29 801 alsa_pcm.cpp::setAudioParams() - sample rate: 8000 , channels: 2
     
    01-03 17:25:29 804 alsa_pcm.cpp::setAudioParams() - frames 40 !

    01-03 17:26:04 790 alsa_pcm.cpp::setAudioParams() - sample rate: 48000 , channels: 2
 
01-03 17:26:04 791 alsa_pcm.cpp::PlayFrame() -  Entry...  
01-03 17:26:04 796 alsa_pcm.cpp::setAudioParams() - frames 242 !

    */
    // 对于不同的samplerate必须设置不同的参数
    ret = snd_pcm_sw_params_set_start_threshold(pcm_handle_, pcm_params_sw_, 67*frames);        //the value(16384) is  suit for pc
    if (ret < 0)
    {
        LogError("unable set start threshold: %s !\n", snd_strerror(ret));
    }

    if (ret = snd_pcm_sw_params(pcm_handle_, pcm_params_sw_) < 0)
    {
        LogError("unable set sw params: %s !\n", snd_strerror(ret));
    }
   // */
}

int AlsaPcm::GetPcmWriteCount()
{
    return pcm_write_count_;
}

