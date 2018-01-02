#include "tb_buffer.hpp"
#include "tb_shared_buffer.hpp"


TBBuffer * TBBuffer::CreateInstance(size_t size ,const char * filter )
{
    TBBuffer * tb_buffer = 0;
    if(filter == 0 )
	{
        tb_buffer = new TBSharedBuffer(size);
    }
    return tb_buffer;
}

