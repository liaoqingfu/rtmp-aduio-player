#include "thread.hpp"
#include <cassert>
#include <iostream>

using namespace std;


extern "C"
{
static void*threadWrapper(void* parm)
{
    assert(parm);
    Thread* t = static_cast<Thread*> (parm);
    assert(t);
    t->onLoop();

    return 0;
}
}

Thread::Thread() :
mId(0), mShutdown(false)
{
}

Thread::~Thread()
{
    shutdown();
    join();
}

void Thread::run()
{
    assert(mId == 0);

    // spawn the thread
    if (int retval = pthread_create(&mId, 0, threadWrapper, this))
    {
        std::cerr << "Failed to spawn thread: " << retval << std::endl;
        assert(0);
    }
}

void Thread::join()
{
    if (mId == 0)
    {
        return;
    }


    void* stat;
    if (mId != pthread_self())
    {
        int r = pthread_join(mId, &stat);
        if (r != 0)
        {
            cerr << "Internal error: pthread_join() returned " << r << endl;
            //        assert(0);
        }
    }

    mId = 0;
}

void Thread::shutdown()
{
    mShutdown = true;
}

bool Thread::isShutdown() const
{
    return mShutdown;
}


