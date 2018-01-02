#include "tb_shared_buffer.hpp"
#include <string.h>

using namespace std;


TBSharedBuffer::TBSharedBuffer(size_t size)
{
    buf_ = BufPtr(new vector<char>(size));
    buf_->resize(0);
}
TBSharedBuffer::~TBSharedBuffer()
{

}

char * TBSharedBuffer::Data()
{
    return &buf_->operator[](0);
}
size_t TBSharedBuffer::Size()
{
    return buf_->size();
}

void TBSharedBuffer::Add(void * data, size_t len)
{
    for(int i = 0; i < len; ++i){
        buf_->push_back(*(char*)((char*)data + i ) );
         //       memcpy(buf_->data(), data, len);
    }
}

void TBSharedBuffer::Clear()
{
    buf_->clear();
}

TBBuffer *TBSharedBuffer::Clone()
{
    return new TBSharedBuffer(*this);
}

