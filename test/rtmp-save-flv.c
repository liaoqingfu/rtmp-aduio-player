
/**
 * Simplest Librtmp Receive
 *
 * 雷霄骅，张晖
 * leixiaohua1020@126.com
 * zhanghuicuc@gmail.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序用于接收RTMP流媒体并在本地保存成FLV格式的文件。
 * This program can receive rtmp live stream and save it as local flv file.
 */
#include <iostream>
#include <cstring>
     

#include <librtmp/rtmp.h>
#include <librtmp/log.h>
using namespace std;

int InitSockets()
{
	
	return 1;
}

void CleanupSockets()
{
	
}

int main(int argc, char* argv[])
{
	InitSockets();
	
	double duration=-1;
	int nRead;
	//is live stream ?
	bool bLiveStream=true;				
	char rtmpUrl[256];
	
	int bufsize=1024*1024*10;			
	   char *buf =  new   char[bufsize];
	memset(buf,0,bufsize);
	long countbufsize=0;
	
	FILE *fp=fopen("receive.flv","wb");
	if (!fp){
		RTMP_LogPrintf("Open File Error.\n");
		CleanupSockets();
		return -1;
	}

	if(argc < 2)
	{
        RTMP_LogPrintf("please use rtmp-save-flv  rtmp_url\n");
        strcpy(rtmpUrl, "rtmp://live.hkstv.hk.lxdns.com/live/hks");
        RTMP_LogPrintf("used default url = %s\n", rtmpUrl);
	}
	else
	{
        strcpy(rtmpUrl,argv[1]);
        RTMP_LogPrintf("user url = %s\n", rtmpUrl);
	}
	
	
	/* set log level */
	//RTMP_LogLevel loglvl=RTMP_LOGDEBUG;
	//RTMP_LogSetLevel(loglvl);

	RTMP *rtmp=RTMP_Alloc();                // 为结构体“RTMP”分配内存。
	RTMP_Init(rtmp);                        // 初始化结构体“RTMP”中的成员变量。
	//set connection timeout,default 30s
	rtmp->Link.timeout=10;	                // 设置连接超时
	// HKS's live URL
	if(!RTMP_SetupURL(rtmp, rtmpUrl))       // 设置输入的RTMP连接的URL。
	{
		RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
		RTMP_Free(rtmp);                    // 释放结构体“RTMP”
		CleanupSockets();
		return -1;
	}
	if (bLiveStream)
	{
		rtmp->Link.lFlags|=RTMP_LF_LIVE;
	}
	
	//1hour
	RTMP_SetBufferMS(rtmp, 3600*1000);		
	
	if(!RTMP_Connect(rtmp,NULL)){
		RTMP_Log(RTMP_LOGERROR,"Connect Err\n");
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}

	if(!RTMP_ConnectStream(rtmp,0)){
		RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}
	unsigned tagDataLen = 0;
    nRead=RTMP_Read(rtmp,buf,13);
    RTMP_LogPrintf("%c%c%c\n", buf[0], buf[1],buf[2]);
    RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB, timestamp = %d, dataType = %d, buflen = %d\n",nRead,countbufsize*1.0/1024, 
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     rtmp->m_read.buflen);
    fwrite(buf,1,nRead,fp);
	while(nRead=RTMP_Read(rtmp,buf,bufsize)){
		fwrite(buf,1,nRead,fp);
        
		countbufsize+=nRead;
		// 有时候会拿到两帧数据
		tagDataLen = 0;
		tagDataLen = (unsigned char)buf[1];
		tagDataLen <<= 8;
		tagDataLen |= (unsigned char)buf[2];
		tagDataLen <<= 8;
		tagDataLen |= (unsigned char)buf[3];
		RTMP_LogPrintf("Receive: %5dByte, buf[0] = 0x%02x,%02x %02x %02x timestamp = %d, dataType = %d, tagDataLen = %d\n",nRead, 
		    buf[0], buf[1], buf[2], (unsigned char)buf[3],
		    rtmp->m_read.timestamp,
		     rtmp->m_read.dataType,
		     tagDataLen);
		if (!bLiveStream&&duration<0)                   // 点播的时候使用
		{
			 duration = RTMP_GetDuration(rtmp);
			 RTMP_LogPrintf("duration:%f\n",duration);
		}
	}

	if(fp)
		fclose(fp);

	if(buf){
		delete [] buf;
	}

	if(rtmp){
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		rtmp=NULL;
	}	
	return 0;
}

