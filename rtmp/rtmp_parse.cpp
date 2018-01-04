#include <iostream>
#include <cstring>
     
#include "rtmp_parse.hpp"
#include <librtmp/rtmp.h>
#include <librtmp/log.h>
#include "media_player.hpp"
#include "log_util.hpp"
#include <netinet/in.h>

#define TAG_TYPE_AUDIO 0x8
#define TAG_TYPE_VIDEO 0x9
#define TAG_TYPE_MEDATA  0x12

RtmpParser::RtmpParser(const char *url)
{
	strcpy(rtmpUrl_, url);
	mediaPlayer_ =  new MediaPlayer(this);
	init();
}

bool RtmpParser::init()
{
	run();
    return true;
}
bool RtmpParser::inputData(const char *pcData, int iLen)    // 只管数据进来,由内部线程来parse数据
{
    return true;
}
// 设置环形buffer
void RtmpParser::onGetAAC(const uint8_t *pcData, int iLen, uint32_t ui32TimeStamp)
{
    uint8_t *pcm;
	//添加adts头
	memcpy(m_adts.data + 7, pcData, iLen);
	m_adts.aac_frame_length = 7 + iLen;
    m_adts.timeStamp = ui32TimeStamp;
    writeAdtsHeader(m_adts, m_adts.data);
    //audioDec_->InputData(m_adts.data, m_adts.aac_frame_length, &pcm);
	// 解aac
	//LogDebug("m_adts.aac_frame_length = %d", m_adts.aac_frame_length);
	mediaPlayer_->onAAC(m_adts);
	m_adts.aac_frame_length = 7;
}
void RtmpParser::onLoop()
{
    double duration=-1;
    int nRead;
    //is live stream ?
    bool bLiveStream=true;              
    
    
    int bufsize=1024*1024*2;           
    char *buf =  new char[bufsize];
    unsigned char *tempBuf;
    memset(buf,0,bufsize);
    long countbufsize=0;
    
    
    /* set log level */
    //RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
    //RTMP_LogSetLevel(loglvl);

    RTMP *rtmp=RTMP_Alloc();                // 为结构体“RTMP”分配内存。
    RTMP_Init(rtmp);                        // 初始化结构体“RTMP”中的成员变量。
    //set connection timeout,default 30s
    rtmp->Link.timeout=10;                  // 设置连接超时
    // HKS's live URL
    if(!RTMP_SetupURL(rtmp, rtmpUrl_))       // 设置输入的RTMP连接的URL。
    {
        RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
        RTMP_Free(rtmp);                    // 释放结构体“RTMP”
        return;
    }
    if (bLiveStream)
    {
        rtmp->Link.lFlags|=RTMP_LF_LIVE;
    }
    
    //1hour
    RTMP_SetBufferMS(rtmp, 3600*1000);          // 函数设置一下缓冲大小
    
    if(!RTMP_Connect(rtmp,NULL)){
        RTMP_Log(RTMP_LOGERROR,"Connect Err\n");
        RTMP_Free(rtmp);
        return;
    }

    if(!RTMP_ConnectStream(rtmp,0)){
        RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        return;
    }
    nRead=RTMP_Read(rtmp,buf,13);
    RTMP_LogPrintf("%c%c%c\n", buf[0], buf[1],buf[2]);
    RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB, timestamp = %d, dataType = %d, buflen = %d\n",nRead,countbufsize*1.0/1024, 
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     rtmp->m_read.buflen);

    unsigned int tagDataLen  = 0;
	unsigned int timeStamp  = 0;
    countbufsize = 0;
    nRead = 0;
    while(true)
    {
        if(0 == nRead)
        {
            nRead = RTMP_Read(rtmp,buf,bufsize);
            if(0 == nRead)
            {
                RTMP_LogPrintf("RTMP_Read finish");
                break;
            }
            tempBuf = (unsigned char *)buf;
            countbufsize += nRead;
        }
        
        
        // 有时候会拿到两帧数据
		tagDataLen = 0;
		tagDataLen = tempBuf[1];
		tagDataLen <<= 8;
		tagDataLen |= tempBuf[2];
		tagDataLen <<= 8;
		tagDataLen |= tempBuf[3];
		
        timeStamp = 0;
		timeStamp = tempBuf[7];
		timeStamp <<= 8;
		timeStamp = tempBuf[4];
		timeStamp <<= 8;
		timeStamp |= tempBuf[5];
		timeStamp <<= 8;
		timeStamp |= tempBuf[6];
		/*
		RTMP_LogPrintf("Receive: %5dByte, buf[0] = 0x%02x,%02x %02x %02x, %02x %02x timestamp = %d, dataType = %d, tagDataLen = %d\n",nRead, 
		    tempBuf[0], tempBuf[1], tempBuf[2], tempBuf[3], tempBuf[11], tempBuf[12],
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     tagDataLen);
		 */  
        // 解析flag
        if(TAG_TYPE_AUDIO == tempBuf[0])                // 音频帧
        {
            // 获取帧长度
            // 识别该帧是帧头还是帧数据
            if(0xaf == tempBuf[11] && 0x00 == tempBuf[12])
            {
                RTMP_LogPrintf("audio head: %5dByte, buf[0] = 0x%02x,%02x %02x %02x timestamp = %d, dataType = %d, tagDataLen = %d\n",nRead, 
			    tempBuf[0], tempBuf[1], tempBuf[2], tempBuf[3],
			    rtmp->m_read.timestamp,
			     rtmp->m_read.dataType,
			     tagDataLen);
			    RTMP_LogPrintf("audio strAudioCfg 0x%02x, 0x%02x\n", tempBuf[13], tempBuf[14]);
                makeAdtsHeader(&tempBuf[13], m_adts);
				getAACInfo(m_adts, m_iSampleRate, m_iChannel);
				RTMP_LogPrintf("audio m_iSampleRate:%d, m_iChannel:%d\n", m_iSampleRate, m_iChannel);
            }
            else if(0xaf == tempBuf[11] && 0x01 == tempBuf[12])
            {
               /*
            	RTMP_LogPrintf("audio data: %5dByte, buf[0] = 0x%02x, timestamp = %d, timeStamp = %d, tagDataLen = %d\n",nRead, 
			    tempBuf[0], 
			    rtmp->m_read.timestamp,
			     timeStamp,
			     tagDataLen);
			    */
            	onGetAAC(tempBuf + 13, tagDataLen - 2, timeStamp);
            }
            
        }
        else if(TAG_TYPE_VIDEO == tempBuf[0])           // 视频帧
        {
            /*
			RTMP_LogPrintf("video data: %5dByte, buf[0] = 0x%02x,%02x %02x %02x timestamp = %d, dataType = %d, tagDataLen = %d\n",nRead, 
		    tempBuf[0], tempBuf[1], tempBuf[2], tempBuf[3],
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     tagDataLen);
		     */
		    
            //LogDebug("tempBuf[11][12] = 0x%x 0x%x, tagDataLen = %d", tempBuf[11], tempBuf[12], tagDataLen);
            if(tempBuf[12] == 0x0)              //0 = AVC sequence header
            {
                uint8_t *spsData = nullptr;
    		    uint8_t *ppsData = nullptr;
                uint32_t spsSize = 0;
                uint32_t ppsSize = 0;

                LogDebug("tempBuf[22] %2x %2x ",tempBuf[22], tempBuf[23]);
                for(int i = 0; i < tagDataLen; i++)
                {
                   // printf("[%d]0x%02x\n ", i+11, tempBuf[i+11]);     // 调试信息
                }
                printf("\n");
                // 获取sps
                spsSize = tempBuf[22];
                spsSize <<= 8;
                spsSize |= tempBuf[23];
                spsData = &tempBuf[24];
                m_strSPS.clear();
                m_strSPS.assign("\x00\x00\x00\x01", 4);
                m_strSPS.append((char *)spsData, spsSize);
                LogDebug("spsSize = %d, m_strSPS.size = %d", spsSize, m_strSPS.size());
           
                // 获取pps
                ppsSize = tempBuf[24 + spsSize + 1];
                ppsSize <<= 8;
                ppsSize |= tempBuf[24 + spsSize + 2];
                ppsData = &tempBuf[24 + spsSize + 3];
                m_strPPS.clear();
                m_strPPS.assign("\x00\x00\x00\x01", 4);
                m_strPPS.append((char *)ppsData, ppsSize);
                LogDebug("ppsSize = %d, ppsSize.size = %d",ppsSize, m_strPPS.size());
            }
            else if(tempBuf[12] == 0x1)         // 1 = AVC NALU
            {
        		uint32_t iTotalLen = tagDataLen;
        		uint8_t *videoTagData = nullptr;
        		videoTagData = &tempBuf[11];            //数据起始
        		// 0 Stream header
        		// 1 AVCPacketType
        		// 2-4 CompositionTime
        		// 
        		uint32_t iOffset = 5;           // 直接跳过
        		while(iOffset + 4 < iTotalLen)
        		{         // 可能存在多个nal
                    uint32_t iFrameLen;
                    // size:
                    memcpy(&iFrameLen, videoTagData + iOffset, 4);    // 获取帧长
                    iFrameLen = ntohl(iFrameLen);
        			iOffset += 4;                   // 跳过size占用的4个字节，然后才是真正的数据
        			if(iFrameLen + iOffset > iTotalLen){
        				break;
        			}
        			_onGetH264((char *)(videoTagData + iOffset), iFrameLen, timeStamp);
        			iOffset += iFrameLen;
        		}
        	}
        }
        else if(TAG_TYPE_MEDATA == tempBuf[0])
        {
			RTMP_LogPrintf("medata: %5dByte, buf[0] = 0x%02x,%02x %02x %02x timestamp = %d, dataType = %d, tagDataLen = %d\n",nRead, 
			    tempBuf[0], tempBuf[1], tempBuf[2], tempBuf[3],
			    rtmp->m_read.timestamp,
			     rtmp->m_read.dataType,
			     tagDataLen);
		         
        }
        else
        {
           
        }
        if(nRead >= tagDataLen + 15)     // 15 = tag head size + pre tag size
        {
            nRead -= (tagDataLen + 15);
            tempBuf += (tagDataLen + 15);
        }
        else
        {
            nRead = 0;
        }
   
    }

    if(buf){
        delete [] buf;
    }

    if(rtmp){
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp=NULL;
    }   

    delete mediaPlayer_;
}


void RtmpParser::_onGetH264(const char *pcData, int iLen, uint32_t ui32TimeStamp)
{
    
    switch (pcData[0] & 0x1F) 
    {
	    case 5: 
    	{
    	    LogDebug("frame_type = 0x%x, len = %d, t = %d", pcData[0]&0x1f, iLen, ui32TimeStamp);
    		onGetH264(m_strSPS.data() + 4, m_strSPS.length() - 4, ui32TimeStamp);
    		onGetH264(m_strPPS.data() + 4, m_strPPS.length() - 4, ui32TimeStamp);
    	}
    	case 1: 
    	{
    	    LogDebug("frame_type = 0x%x, len = %d, t = %d", pcData[0]&0x1f, iLen, ui32TimeStamp);
    		onGetH264(pcData, iLen, ui32TimeStamp);
    	}
		break;
	    default:
		//WarnL <<(int)(pcData[0] & 0x1F);
		break;
	}
}
void RtmpParser::onGetH264(const char *pcData, int iLen, uint32_t ui32TimeStamp)
{
    LogDebug("frame_type = 0x%x, len = %d, t = %d", pcData[0]&0x1f, iLen, ui32TimeStamp);
    m_h264frame.type = pcData[0] & 0x1F;
	m_h264frame.timeStamp = ui32TimeStamp;
	m_h264frame.data.assign("\x0\x0\x0\x1", 4);  //添加264头
	m_h264frame.data.append(pcData, iLen);
	LogDebug("");
	{
		// 送给解码器进行解码
		mediaPlayer_->onH264(m_h264frame);
	}
	m_h264frame.data.clear();
	FunExit();
}

