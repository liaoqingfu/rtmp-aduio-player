#ifndef _TB_BUFFER_H_
#define _TB_BUFFER_H_
#include <cstdlib>
#include <stdint.h>

class TBBuffer
{
public:
    static TBBuffer * CreateInstance(int      size = 2048, const char * filter = nullptr);
    virtual ~TBBuffer(){}
    virtual uint8_t * Data() { return nullptr;}
    virtual int Size() {return 0;}
    virtual bool Add(uint8_t * data, int len){return false;}
    virtual void Clear() {}
    virtual TBBuffer * Clone() { return nullptr;}
};

#endif //__TB_BUFFER_H__

