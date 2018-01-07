#include "tb_shared_buffer.hpp"
#include <string.h>

using namespace std;


TBSharedBuffer::TBSharedBuffer(int size)
{
    buf_ = new uint8_t[size];
    capacity_ = size;
	size_ = 0;
}
TBSharedBuffer::~TBSharedBuffer()
{
	if(buf_)
	{
		//printf("~TBSharedBuffer size_ = %d, %d\n", size_, capacity_);
		delete [] buf_;
	}
}

uint8_t * TBSharedBuffer::Data()
{
    return buf_;
}
int TBSharedBuffer::Size()
{
    return size_;
}

bool TBSharedBuffer::Add(uint8_t *data, int len)
{
	if((!buf_) || (!data) || (len < 0))
	{
		printf("add failed, buf_ = 0x%x, data = 0x%x, len = %d\n", buf_, data, len);
		return false;
	}

    if(len + size_ > capacity_)	//ÖØÐÂÉêÇëÄÚ´æ
   	{
   		printf("new buffer again, len = %d, size_ = %d, capacity_ = %d\n", len, size_, capacity_);
		capacity_ = ((len + size_)>>11 + 1) * 2048;
		uint8_t *tempBuf  = new uint8_t[capacity_];
		if(!tempBuf)
		{
			printf("new tempBuf faile\n");
			return false;
		}
		memcpy(tempBuf, buf_, size_);
		delete [] buf_;
		buf_ = tempBuf;
	}
	memcpy(buf_ + size_, data, len);
	size_ += len;

	return true;
}

void TBSharedBuffer::Clear()
{
    size_ = 0;
}

TBBuffer *TBSharedBuffer::Clone()
{
	TBBuffer *tbBfuffer = new TBSharedBuffer(capacity_);

	if(!tbBfuffer)
		return nullptr;
	
	if(tbBfuffer->Add(buf_, size_))
    {
    	return tbBfuffer;
	}
	else
	{
		delete tbBfuffer;
		return nullptr;
	}
}

