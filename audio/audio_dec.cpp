/*
 * MIT License
 *
 * Copyright (c) 2017 xiongziliang <771730766@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "audio_dec.hpp"
#include "neaacdec.h"
#include "log_util.hpp"

AudioDec::AudioDec(void) {
	// TODO Auto-generated constructor stub
	FunEntry();
	_handle=nullptr;
	samplebit_=16;
}

AudioDec::~AudioDec(void) {
	// TODO Auto-generated destructor stub
	FunEntry();
	if(_handle != nullptr)
	{
		NeAACDecClose((NeAACDecHandle)_handle);
		_handle =nullptr;
	}
}

bool AudioDec::Init(const void *adtshed,int hedlen) 
{
    FunEntry();
	_handle = NeAACDecOpen();
	if(_handle == nullptr)
	{
	    LogError("NeAACDecOpen failed");
		return false;
	}

#if 1       // 必须设置该段代码，否则解码16k、8k的采样率时不对
    NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(_handle);
    conf->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(_handle, conf);
#endif

	char err = NeAACDecInit((NeAACDecHandle)_handle, ( unsigned char *)adtshed, hedlen, &samplerate_, &channels_);
	if (err != 0)
	{
	    LogError("NeAACDecInit failed");
		return false;
	}
	return true;

}

int AudioDec::InputData(const void *data, int len, unsigned char** pOutBuffer) {
    uint8_t *pcmData = (uint8_t *)data;
	NeAACDecFrameInfo hInfo;
	NeAACDecHandle handle = (NeAACDecHandle)_handle;
	//FunEntry();
	//LogInfo("%02x %02x %02x %02x %02x %02x, %02x, len = %d", pcmData[0], pcmData[1], pcmData[2], pcmData[3],
	//     pcmData[4], pcmData[5], pcmData[6], len);
	*pOutBuffer=(unsigned char*)NeAACDecDecode(handle, &hInfo, (unsigned char*)data,len);
	if (!((hInfo.error == 0) && (hInfo.samples > 0)))
	{
	    LogError("NeAACDecDecode failed");
		return 0;
	}
	//LogInfo("samples = %d, channels = %d, samplerate = %d", hInfo.samples, hInfo.channels, 
	//    hInfo.samplerate);
	return hInfo.samples*hInfo.channels;
}

