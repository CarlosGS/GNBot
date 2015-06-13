#ifndef COMM_MANAGER_H
#define COMM_MANAGER_H

#include <pthread.h>
#include <list>
#include <Socket.h>
#include "Datagram.h"

class CommManager
{
	protected :
		std::list<Datagram> inbox;
		Socket * sock;
		struct CallBack {
			void (*callback)(void*);
			void * arg;
		};
		CallBack cb[256];

		pthread_t in_thread;
		pthread_mutex_t mtx;
		friend void * commmanager_inthread(void *);
		void store(const Datagram & dgm);
	public :
		CommManager();
		~CommManager();
		
		bool open(const char * hostname, unsigned int port);
		bool close();

		bool send(const Datagram & dgm);
		Datagram wait(unsigned char id);
};
		




#endif
