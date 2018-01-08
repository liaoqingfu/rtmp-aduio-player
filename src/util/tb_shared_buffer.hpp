#ifndef _TB_SHARED_BUFFER_H_
#define _TB_SHARED_BUFFER_H_

#include <tb_buffer.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class TBSharedBuffer:
        public TBBuffer
{
public:
    TBSharedBuffer(int size = 2048);
    ~TBSharedBuffer();

    virtual uint8_t * Data();
    virtual int Size();
    virtual bool Add(uint8_t * data, int len);
    virtual void Clear();
    virtual TBBuffer *Clone();
protected:
    uint8_t *buf_;
	int capacity_;
	int size_;
};
#endif

