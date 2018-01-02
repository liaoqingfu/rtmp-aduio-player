#ifndef _TB_SHARED_BUFFER_H_
#define _TB_SHARED_BUFFER_H_

#include <tb_buffer.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

class TBSharedBuffer:
        public TBBuffer
{
public:
    TBSharedBuffer(size_t size = 1024 );
    ~TBSharedBuffer();

    virtual char * Data();
    virtual size_t Size();
    virtual void Add(void * data, size_t len);
    virtual void Clear();
    virtual TBBuffer *Clone();
protected:
    typedef boost::shared_ptr<std::vector<char> > BufPtr;
    BufPtr buf_;
};
#endif

