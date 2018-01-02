#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>

class Thread
{
public:
	typedef pthread_t Id;
  	Thread();
  	virtual ~Thread();
	virtual void run();
	void join();
	virtual void shutdown();
	bool isShutdown() const;
  	virtual void onLoop() = 0;
protected:
  	Id mId;
  	volatile bool mShutdown;

private:
  	// Suppress copying
  	Thread(const Thread &);
	const Thread & operator=(const Thread &);
};

#endif

