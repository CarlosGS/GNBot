#include <math.h>
#include <errno.h>
#include <time.h>

#include "canon_vbc50i/libCanon/DgmQueue.h"


DgmQueue::DgmQueue()
{
	unsigned int i;
	pthread_mutex_init(&Qmtx,NULL);
	pthread_cond_init(&Qcnd,NULL);
	for (i=0;i<256;i++) interesting[i] = false;
	verbose = 0;
}

DgmQueue::~DgmQueue()
{
}

void DgmQueue::addInteresting(unsigned int i)
{
	lock();
	interesting[i] = true;
	unlock();
}

void DgmQueue::remInteresting(unsigned int i)
{
	lock();
	interesting[i] = false;
	Q.erase(i);
	unlock();
}

void DgmQueue::resetInteresting()
{
	unsigned int i;
	lock();
	for (i=0;i<256;i++) interesting[i] = false;
	Q.clear();
	unlock();
}


void DgmQueue::storeDgm(const Datagram & dgm)
{
	lock();
	if (interesting[dgm.getId()]) {
		//printf("T %08X Storing %d\n",(int)pthread_self(),dgm.getId());
		Q.insert(Queue::value_type(dgm.getId(),dgm));
		//printf("T %08X Signaling\n",(int)pthread_self());
		pthread_cond_signal(&Qcnd);
	} else {
		if (verbose > 0)
			printf("T %08X Discarding %d\n",(int)pthread_self(),dgm.getId());
		if (verbose > 1) 
			dgm.print();
	}
	unlock();
}

bool DgmQueue::waitDgm(unsigned int id, Datagram & dgm, double maxwait_s)
{
	//printf("T %08X Waiting %d\n",(int)pthread_self(),id);
	lock();
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	//printf("Current time is %d.%d",(int)ts.tv_sec,(int)ts.tv_nsec);
	double tmax = ts.tv_sec + ts.tv_nsec*1e-9 + maxwait_s;
	ts.tv_sec = (long int)floor(tmax);
	ts.tv_nsec = (long int)floor((tmax-ts.tv_sec)*1e9);
	//printf(", waiting till %d.%d\n",(int)ts.tv_sec,(int)ts.tv_nsec);
	int r;
	Queue::iterator it = Q.find(id);
	if (it == Q.end()) {
		while (it == Q.end()) {
			if (maxwait_s > 0) {

				//printf("T %08X TWaiting and unlocking\n",(int)pthread_self());
				r = pthread_cond_timedwait(&Qcnd,&Qmtx,&ts);
				//printf("T %08X cond wait exits with %d\n",(int)pthread_self(),r);
				//print();
				if (r != 0) {
					//printf("T %08X Timeout (%d)\n",(int)pthread_self(),r);
					unlock();
					return false;
				}
			} else {
				//printf("T %08X Waiting and unlocking\n",(int)pthread_self());
				r = pthread_cond_wait(&Qcnd,&Qmtx);
				//printf("T %08X cond wait exits with %d\n",(int)pthread_self(),r);
			}
			//printf("Finding\n");
			it = Q.find(id);
			//printf("%s\n",(it==Q.end())?"Nothing":"Something");
		}
	} 
	//printf("T %08X Received %d\n",(int)pthread_self(),id);
	dgm = it->second;
	Q.erase(it);
	unlock();
	return true;
}

void DgmQueue::print()
{
	Queue::iterator it;
	printf("### QUEUE ######\n");
	for (it=Q.begin();it != Q.end();it++)
	{
		printf("[ %d D %d ]\n",it->first,it->second.getId());
	}
	printf("################\n");
}
