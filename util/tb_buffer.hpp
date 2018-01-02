#ifndef _TB_BUFFER_H_
#define _TB_BUFFER_H_
#include <cstdlib>

class TBBuffer
{
public:
    static TBBuffer * CreateInstance(size_t size = 2048,const char * filter = 0);
    virtual ~TBBuffer(){}
    virtual char * Data() { return 0;}
    virtual size_t Size() {return 0;}
    virtual void Add(void * data, size_t len){}
    virtual void Clear() {}
    virtual TBBuffer * Clone() { return 0;}
};

#endif //__TB_BUFFER_H__

