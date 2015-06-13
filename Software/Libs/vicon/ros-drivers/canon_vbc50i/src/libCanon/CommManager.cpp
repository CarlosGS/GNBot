#include <assert.h>
#include <string.h>
#include <errno.h>

#include "canon_vbc50i/libCanon/XString.h"
#include "canon_vbc50i/libCanon/CommManager.h"
#include "canon_vbc50i/libCanon/login.h"


CommManager::CommManager(const char * initseq_filename)
{
	sock = NULL;
	verbose = 1;
	in_thread = 0;
	terminate = false;
	pthread_mutex_init(&sockmtx,NULL);
	Q.setVerboseLevel(1);

	Datagram dgm;
	bool loginFromFile = false;
	FILE * fp = NULL;
	if (initseq_filename) {
		fp = fopen(initseq_filename,"r");
		if (fp == NULL) {
			fprintf(stderr,"CommManager: Could not open '%s'\n",initseq_filename);
		} else {
			loginFromFile = true;
		}
	}
	if (loginFromFile) {
		while (1) {
			if (!dgm.read(fp)) break;
			//printf("F Telegram %d: id %d\n",init_seq.size(),dgm.getId());
			init_seq.push_back(dgm);
		}
		fclose(fp);
	} else {
		printf("Using builtin login sequence\n");
		RawBinDatagram * bdgm = raw_bin_login;
		while (bdgm->len != 0) {
			if (!dgm.read(bdgm->data,bdgm->len)) break;
			//printf("M Telegram %d: id %d\n",init_seq.size(),dgm.getId());
			init_seq.push_back(dgm);
			bdgm ++;
		}
	}
#ifdef CM_CARE_FOR_REPLY
	init_reply.resize(init_seq.size());
#endif
}	

CommManager::~CommManager()
{
	close();
}

void * commmanager_inthread(void * arg) 
{
	CommManager * cm = (CommManager*)arg;
	cm->run();
	return NULL;
}

bool CommManager::sendInitSeq()
{
	unsigned int i;
	Datagram dgm;
	for (i=0;i<init_seq.size();i++) {
		if (!send(init_seq[i])) {
			printf("%s: Can't send datagram %d:",__FUNCTION__,i);
			init_seq[i].write(stdout);
			printf("\n");
			goto cleanup;
		}
		if (!sock->WaitData((i==0)?1000:100)) {
			printf("No answer for datagram %d\n",i);
			goto cleanup;
		}
		if (!dgm.receive(sock)) {
			printf("Error receiving answer for datagram %d\n",i);
			goto cleanup;
		}
		if (verbose > 1) {
			printf("Received: ");dgm.print(stdout);
		}
#ifdef CM_CARE_FOR_REPLY
		init_reply[i] = dgm;
#endif
	}

	printf("CM: Initialization completed\n");
	return true;
cleanup:
	printf("CM: Initialization failed\n");
	return false;
}


bool CommManager::open(const char * hostname, unsigned int port)
{
	int r;
	if (sock != NULL) delete sock;
	sock = new Socket(hostname,port);
	if (verbose) 
		sock->showErrors(); 
	else 
		sock->hideErrors();
	if (!sock->Open()) goto clean_exit;
	if (!sendInitSeq()) goto clean_exit;
	
	terminate = false;
	r = pthread_create(&in_thread,NULL,commmanager_inthread,this);
	if (r != 0) throw XString("Failed to start CommManager thread: ") + strerror(errno);

	return true;


clean_exit:
	delete sock;
	sock = NULL;
	return false;
} 

void CommManager::setVerboseLevel(unsigned int v) {
	verbose=v;
	if (sock != NULL) {
		if (verbose) 
			sock->showErrors(); 
		else 
			sock->hideErrors();
	}
	Q.setVerboseLevel(verbose);
	if (verbose > 1)
		printf("CommManager: Setting verbose level to %d\n",verbose);
}

bool CommManager::reconnect()
{
	if (sock == NULL) {
		return false;
	}
	// setVerboseLevel(2);
	if (verbose > 0)
		printf("CM: reconnecting\n");
	sock->Close();
	bool ret = sock->Open();;
	
	if (ret) ret = sendInitSeq();
	return ret;
}

bool CommManager::close()
{
	terminate = true;
	if (in_thread != 0) {
		pthread_join(in_thread,NULL);
		in_thread = 0;
	}
	if (sock != NULL) {
		sock->Close();
		delete sock;
	}
	sock = NULL;
	return true;
}

void CommManager::run()
{
	if (verbose>1) 
		printf("T %08X CommManager::run()\n",(int)pthread_self());
	Datagram dgm;
	if (sock == NULL) return;
	if (!sock->IsOpen()) return;
	while (!terminate) {
		if (!sock->IsOpen()) {
			usleep(10000);
			continue;
		}
		if (sock->IsBroken()) {
			reconnect();
		}
#if 0
		printf("T %08X CommManager::run()\n",(int)pthread_self());
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		printf("Current time is %d.%d\n",(int)ts.tv_sec,(int)ts.tv_nsec);
#endif
		if (!sock->WaitData(100)) {
			usleep(10000);
			continue;
		}
		if (!dgm.receive(sock,100,(verbose>1))) {
			usleep(10000);
			continue;
		}
		if (verbose > 1) {
			printf("Received: ");dgm.print(stdout);
		}
		Q.storeDgm(dgm);
	}
	if (verbose>1) 
		printf("CommManager::run exiting\n");
}

bool CommManager::send(Datagram & dgm)
{
	if (sock == NULL) return false;
	if (!sock->IsOpen()) return false;
	if (sock->IsBroken()) { /* run will take care of that */return false; }
	//printf("Locking sock mtx\n");
	pthread_mutex_lock(&sockmtx);
	//printf("-- Locked sock mtx\n");
	bool res = dgm.send(sock);
	if (verbose > 1) {
		printf("Sent    : ");dgm.print(stdout);
	}
	pthread_mutex_unlock(&sockmtx);
	return res;
}

bool CommManager::addToReceptionField(unsigned char id)
{
	Q.addInteresting(id);
	return true;
}

bool CommManager::removeFromReceptionField(unsigned char id)
{
	Q.remInteresting(id);
	return true;
}

bool CommManager::resetReceptionField()
{
	Q.resetInteresting();
	return true;
}

bool CommManager::wait(unsigned char id, Datagram & dgm, double timeout)
{
	return Q.waitDgm(id,dgm,timeout);
	//return Q.waitDgm(id,dgm,-1);
}





