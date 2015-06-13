#ifndef DGM_QUEUE_H
#define DGM_QUEUE_H

#include <map>
#include <pthread.h>
#include "Datagram.h"


class DgmQueue
{
    protected:
	typedef std::multimap< unsigned char,Datagram, 
		std::less<unsigned char> > Queue;

	Queue Q;
	pthread_mutex_t Qmtx;
	pthread_cond_t Qcnd;
	unsigned int verbose;

	bool interesting[256];

	void lock() {
	    //printf("T %08X locking queue\n",(int)pthread_self());
	    pthread_mutex_lock(&Qmtx);
	    //printf("T %08X queue locked\n",(int)pthread_self());
	}
	void unlock() {
	    //printf("T %08X unlocking queue\n",(int)pthread_self());
	    pthread_mutex_unlock(&Qmtx);
	    //printf("T %08X queue unlocked\n",(int)pthread_self());
	}

    public :
	DgmQueue();
	~DgmQueue();

	void addInteresting(unsigned int i);
	void remInteresting(unsigned int i);
	void resetInteresting();

	void storeDgm(const Datagram & dgm);

	bool waitDgm(unsigned int id, Datagram & dgm, double maxwait_s=-1);

	void print();

	void setVerboseLevel(unsigned int v) {verbose=v; }
};




#endif // DGM_QUEUE_H
