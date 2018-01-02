#include <iostream>
#include <cstring>
     
#include "rtmp_parse.hpp"
#include <librtmp/rtmp.h>
#include <librtmp/log.h>


#define TAG_TYPE_AUDIO 8
#define TAG_TYPE_VIDEO 9
#define TAG_TYPE_MEDATA  12


bool RtmpParser::init()
{
    return true;
}
bool RtmpParser::inputData(const char *pcData, int iLen)    // 只管数据进来,由内部线程来parse数据
{
    return true;
}
// 设置环形buffer
void RtmpParser::onGetAAC(const char *pcData, int iLen, uint32_t ui32TimeStamp)
{

}
int RtmpParser::onLoop()
{
    double duration=-1;
    int nRead;
    //is live stream ?
    bool bLiveStream=true;              
    char rtmpUrl[256];
    
    int bufsize=1024*1024*10;           
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
    if(!RTMP_SetupURL(rtmp, rtmpUrl))       // 设置输入的RTMP连接的URL。
    {
        RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
        RTMP_Free(rtmp);                    // 释放结构体“RTMP”
        return -1;
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
        return -1;
    }

    if(!RTMP_ConnectStream(rtmp,0)){
        RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        return -1;
    }
    nRead=RTMP_Read(rtmp,buf,9);
    RTMP_LogPrintf("%c%c%c\n", buf[0], buf[1],buf[2]);
    RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB, timestamp = %d, dataType = %d, buflen = %d\n",nRead,countbufsize*1.0/1024, 
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     rtmp->m_read.buflen);

    unsigned int tagDataLen  = 0;
    while(nRead=RTMP_Read(rtmp,buf,bufsize))
    {
        tempBuf = (unsigned char *)buf;
        countbufsize+=nRead;
        // 有时候会拿到两帧数据
		tagDataLen = 0;
		tagDataLen = tempBuf[1];
		tagDataLen <<= 8;
		tagDataLen |= tempBuf[2];
		tagDataLen <<= 8;
		tagDataLen |= tempBuf[3];
		RTMP_LogPrintf("Receive: %5dByte, buf[0] = 0x%02x,%02x %02x %02x timestamp = %d, dataType = %d, tagDataLen = %d\n",nRead, 
		    tempBuf[0], tempBuf[1], tempBuf[2], tempBuf[3],
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     tagDataLen);
        // 解析flag
        if(TAG_TYPE_AUDIO == tempBuf[0])                // 音频帧
        {
            // 获取帧长度
            // 识别该帧是帧头还是帧数据
            if(0xaf == tempBuf[11] && 0x00 == tempBuf[12])
            {
                // 
                makeAdtsHeader(&tempBuf[13], m_adts);
            }
            else if(0xaf == tempBuf[11] && 0x01 == tempBuf[12])
            {
                //添加adts头
            	memcpy(m_adts.data + 7, tempBuf + 13, tagDataLen - 2);
            	m_adts.aac_frame_length = 7 + tagDataLen - 2;
                m_adts.timeStamp = rtmp->m_read.timestamp;
                writeAdtsHeader(m_adts, m_adts.data);
            	{
            		
            		//if (onAudio) {
            			//onAudio(m_adts);
            		//}
            	}
            	m_adts.aac_frame_length = 7;
            }
        }
        else if(TAG_TYPE_VIDEO == tempBuf[0])           // 视频帧
        {

        }
        else
        {

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
    return 0;

}

