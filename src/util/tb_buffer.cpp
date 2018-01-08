#include "tb_buffer.hpp"
#include "tb_shared_buffer.hpp"


TBBuffer * TBBuffer::CreateInstance(int size , const char * filter)
{
    TBBuffer * tb_buffer = nullptr;
    if(filter == nullptr )
	{
        tb_buffer = new TBSharedBuffer(size);
    }
    return tb_buffer;
}

