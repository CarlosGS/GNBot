#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <pthread.h>
#include <list>
#include "../libSock/Socket.h"
#include "Datagram.h"
#include "DgmQueue.h"

//#define CM_CARE_FOR_REPLY

class CommManager
{
	protected :
		bool terminate;
		Socket * sock;
		DgmQueue Q;
		pthread_t in_thread;
		pthread_mutex_t sockmtx;

		unsigned int verbose;
		friend void * commmanager_inthread(void *);
		void run();
		bool sendInitSeq();
		bool reconnect();

		std::vector<Datagram> init_seq;
#ifdef CM_CARE_FOR_REPLY
		std::vector<Datagram> init_reply;
#endif

	public :
		CommManager(const char * initseq_filename = NULL);
		~CommManager();

		bool open(const char * hostname, unsigned int port);
		bool close();

		bool send(Datagram & dgm);
		bool wait(unsigned char id, Datagram & dgm, double timeout=1.0);

		bool addToReceptionField(unsigned char id);
		bool removeFromReceptionField(unsigned char id);
		bool resetReceptionField();

		void setVerboseLevel(unsigned int v); 
};





#endif
