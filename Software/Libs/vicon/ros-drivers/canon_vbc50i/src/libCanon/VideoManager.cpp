#include <math.h>
#include <assert.h>
#include <sys/time.h>
#include "canon_vbc50i/libCanon/VideoManager.h"
#include "canon_vbc50i/libCanon/JpegReader.h"
#ifdef USE_IPP
#include "canon_vbc50i/libCanon/JpegReader-ipp.h"
#endif

VideoManager::VideoManager()
{
#ifdef USE_IPP
	jpeg = new JpegReaderIPP;
#else
	jpeg = new JpegReader;
#endif

	observedFrameRate = 0;
	lastFrameTimeStamp = -1;
	interImageDelay = 0;
	paused = true;
	readyToRequest = false;
	verbose = 1;
	signal = NULL;
	signalarg = NULL;
	videosock = NULL;

	destination = NULL;
	frame = 0;
	terminate = false;
	savestream = false;

	rec_thr_id = 0;
	req_thr_id = 0;
	pthread_mutex_init(&sockMtx,NULL);
	pthread_mutex_init(&initMtx,NULL);
	pthread_mutex_init(&reqMtx,NULL);
	pthread_cond_init(&reqCond,NULL);

}

VideoManager::~VideoManager()
{
	//printf("~VideoManager\n");
	close();
	delete jpeg;
	jpeg = NULL;
}

void VideoManager::setVerboseLevel(unsigned int v) {
	verbose=v;
	if (videosock != NULL) {
		if (verbose) 
			videosock->showErrors(); 
		else 
			videosock->hideErrors();
	}
	if (verbose > 1)
		printf("VideoManager: Setting verbose level to %d\n",verbose);
}


double ts() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec + tv.tv_usec*1e-6;
}

bool VideoManager::receiveOneFrame()
{
	pthread_mutex_lock(&initMtx);
	pthread_mutex_unlock(&initMtx);
	if (videosock == NULL) goto clean_exit;
	// will be dealt with in request thread
	if (videosock->IsBroken()) goto clean_exit; 
	if (!videosock->IsOpen()) goto clean_exit;
	//printf("Waiting for one frame\n");	
	while (1) {
		if (!reception.receive(videosock,5000))
			goto clean_exit;
		//printf("Received videogram %02X\n",reception.getId());	
		switch (reception.getId()) {
			case 0x02:
				observedFrameRate = 1.0 / 
					(reception.getTimeStamp() - lastFrameTimeStamp);
				lastFrameTimeStamp = reception.getTimeStamp();
				readyToRequest = true;
				pthread_cond_broadcast(&reqCond);
				decodeImage(); 
				return true;
			default : 
				if (verbose >= 1) {
					printf("Discarding: ");reception.print();
				}
				break;
		}
	}
clean_exit:
	//printf("Videogram reception failed\n");	
	readyToRequest = true;
	pthread_cond_broadcast(&reqCond);
	return false;
}

bool VideoManager::decodeImage()
{
#ifdef DOTIMING
	static double stopclock = 0;
	static double lasttime = -1;
	struct timeval tv;
	double t0,t1;
	gettimeofday(&tv,NULL);t0=tv.tv_sec+tv.tv_usec*1e-6;
	if (lasttime < 0) lasttime = t0; 
#endif
	frame += 1;

	if (destination != NULL) {
		jpeg->setExternalOutput(destination);
	}
	bool res = jpeg->load(reception.getData(),reception.getLength());
	jpeg->setTimeStamp(reception.getTimeStamp());

#ifdef DOTIMING
	gettimeofday(&tv,NULL);t1=tv.tv_sec+tv.tv_usec*1e-6;
	stopclock += t1-t0;

	if (frame % 10 == 0) {
		printf("\nDecode: %f s\n",1e3*stopclock/10);
		printf("FPS: %f 10 in %f\n",10/(t1-lasttime),(t1-lasttime) );
		stopclock = 0;
		lasttime = -1;
	}
#endif

	if (!res) 
		return false;
	if (savestream) 
		ssaver.record((const char*)reception.getData(),reception.getLength());
	if (signal != NULL) {
		signal(signalarg,jpeg,frame);
	}
	return true;
}

void * video_reception_thread(void * thrarg)
{
	unsigned int nfailure = 0;
	VideoManager * vm = (VideoManager*)thrarg;
	while (!vm->terminate) {
		if (vm->isPaused()) {
			// wait 5 ms;
			usleep(5000);
		} else if (vm->receiveOneFrame()) {
			nfailure = 0;
		} else {
			switch (vm->verbose) {
				case 2:
					printf("Failed to receive videogram (%d)\n",nfailure);
					break;
				case 1:
					printf("?");fflush(stdout);
					break;
				default :
					break;
			}
			nfailure += 1;
		}
		if (vm->verbose>1) {
			printf("%c",vm->terminate?'!':'.');
			fflush(stdout);
		}
	}
	return NULL;
}

void * video_request_thread(void * thrarg)
{
	VideoManager * vm = (VideoManager*)thrarg;
	while (!vm->terminate) {
		if (vm->checkSocketState()) {
			vm->requestNextImageIfNecessary();
		} else {
			sleep(1);
		}
	}
	return NULL;
}

bool VideoManager::checkSocketState()
{
	if (videosock->IsBroken()) {
		printf("VM: Socket is broken, trying to reconnect\n");
		return reconnect();
	}
	return true;
}

bool VideoManager::requestNextImageIfNecessary()
{
	if (paused) {
		usleep(5000);
		return false;
	}
	if (!readyToRequest) {
		pthread_cond_wait(&reqCond,&reqMtx);
	}
	
	double now = ts();
	if ((now - lastFrameTimeStamp) >= interImageDelay) {
		readyToRequest = false;
		return requestNextImage();
	} else {
		double sleeptime = interImageDelay - (now - lastFrameTimeStamp);
		usleep((unsigned int)(sleeptime*1e6));
	}

	return false;
}


bool VideoManager::open(const char *  hostname, unsigned int port)
{
	pthread_mutex_init(&initMtx,NULL);
	pthread_mutex_lock(&initMtx);
	if (videosock != NULL) close();
	frame = 0;

	//printf("Connecting to %s:%d\n",hostname,port);
	videosock = new Socket(hostname,port);
	if (verbose) 
		videosock->showErrors(); 
	else 
		videosock->hideErrors();
	if (!videosock->Open()) {
		delete videosock;
		videosock = NULL;
		return false;
	}
	if (verbose>=1)
		printf("Successfully connected to %s:%d\n",hostname,port);

	if (verbose==0)
		videosock->hideErrors();

	terminate = false; paused = false;
	pthread_create(&rec_thr_id,NULL,video_reception_thread,this);
	if (!sendInitSeq()) return false;

	readyToRequest = true;
	pthread_create(&req_thr_id,NULL,video_request_thread,this);
	pthread_mutex_unlock(&initMtx);
	return true;
}

bool VideoManager::reconnect()
{
	pthread_mutex_init(&initMtx,NULL);
	pthread_mutex_lock(&initMtx);
	if (verbose > 0)
		printf("VM: reconnecting\n");
	videosock->Close();
	sleep(1);
	if (!videosock->Open()) {
		if (verbose > 0)
			printf("VM: Failed to reopen connection\n");
		return false;
	}
	if (!sendInitSeq()) {
		if (verbose > 0)
			printf("VM: Failed to resend init seq\n");
		return false;
	}
	if (!setImageSize(width,height)) {
		if (verbose > 0)
			printf("VM: Failed to reset image size\n");
		return false;
	}
	pthread_mutex_unlock(&initMtx);
	return true;
}

bool VideoManager::close()
{
	terminate = true;
	pthread_mutex_unlock(&initMtx);
	if (rec_thr_id != 0) {
		pthread_join(rec_thr_id,NULL);
		rec_thr_id = 0;
	}
	if (req_thr_id != 0) {
		pthread_cancel(req_thr_id);
		req_thr_id = 0;
	}
	if (videosock != NULL) {
		videosock->Close();
		delete videosock;
		videosock = NULL;
	}
	paused = true;
	return true;
}

bool VideoManager::requestNextImage()
{
	if (videosock == NULL) return false;
	if (videosock->IsBroken()) return false;
	if (!videosock->IsOpen()) return false;

	VideoGram vg(0x85);
	pthread_mutex_lock(&sockMtx);
	bool res = vg.send(videosock);
	pthread_mutex_unlock(&sockMtx);
	//printf("Requested new image: %s\n",res?"Success":"Failure");
	return res;
}

bool VideoManager::getImageSize(unsigned int *width, unsigned int *height)
{
	*width = jpeg->width;
	*height = jpeg->height;
	return true;
}


bool VideoManager::setImageSize(unsigned int _width, unsigned int _height)
{
	unsigned char message[4] = {
		0x00, 0x00, 0x00, 0x00
	};
	bool known_size = false;
	// Pedantic function to check for input values
	if ((_width==768) && (_height==576)) known_size = true;
	if ((_width==384) && (_height==288)) known_size = true;
	if ((_width==192) && (_height==144)) known_size = true;
	if ((_width==96) && (_height==72)) known_size = true;
	if (!known_size) {
		fprintf(stderr,"Warning unknown image size %dx%d\n",
				_width,_height);
		assert(known_size);
	}
	width = _width;
	height = _height;
	message[0] = width >> 8;
	message[1] = width & 0xFF;
	message[2] = height >> 8;
	message[3] = height & 0xFF;
	VideoGram vg(0x88);
	vg.setData(message,4);
	pthread_mutex_lock(&sockMtx);
	bool res = vg.send(videosock);
	pthread_mutex_unlock(&sockMtx);
	if (verbose > 0) {
		printf("Sent: ");vg.print();
		printf("Changed image size: %s\n",res?"Success":"Failure");
	}
	return res;
}


static void waitZeroAndPrint(Socket * sock,unsigned int verbose=0)
{
	bool printed = false;
	while (sock->WaitData(100)) {
		unsigned char c;
		sock->Receive(&c,1);
		if (verbose >= 1) {
			printed = true;
			if (isprint(c)) 
				fprintf(stdout,"%c",c);
			else 
				fprintf(stdout,".");
		}
		//fprintf(stdout," 0r%02X ",c); 
		if (c == 0) break;
	}
	if (printed) fprintf(stdout,"\n");
}


static bool send0x0A(Socket * sock, unsigned char * s)
{
	unsigned int l = strlen((char*)s);
	s[l] = 0x0a;
	s[l+1] = 0x00;
	bool res = sock->SendAll(s,l+1,100);	
	//printf("Sent: '%s': %d\n",s,res);
			
	return res;
}

bool VideoManager::sendInitSeq()
{
	if (videosock == NULL) {
		printf("sendInitSeq: videosock == NULL\n");
		return false;
	}
	if (!videosock->IsOpen()) {
		printf("sendInitSeq: videosock not open\n");
		return false;
	}
	unsigned char tmp[1024];
	strcpy((char*)tmp,"VERSION"); 
	if (!send0x0A(videosock,tmp)) return false;
	waitZeroAndPrint(videosock,verbose);
	//printf("\n---\n\n");

	unsigned char login[20] = {
		'S', 'U', 'P', 'E', 'R', ' ', 'r', 0x10,
		0x10, 0x0b, 0x45, 0x29, 0x3d, 0x52, 0x3c, 0x4a,
		0x4f, 0x16, 0x00
	};
	if (!send0x0A(videosock,login)) return false;
	unsigned char admin[] = "SET Mode Administrator";
	if (!send0x0A(videosock,admin)) return false;
	unsigned char muxconf[] = "GET MUXCONF";
	if (!send0x0A(videosock,muxconf)) return false;
	waitZeroAndPrint(videosock,verbose);
	//printf("\n---\n\n");
	unsigned char start[] = "START";
	if (!send0x0A(videosock,start)) return false;

	VideoGram vg;
	reception.receive(videosock,100);
	if (verbose >= 2) {
		printf("Sent    : START\n");
		printf("Received: ");reception.print();
	}

	vg.reset(); vg.setId(0x85); 
	if (!vg.send(videosock)) return false;
	reception.receive(videosock,100);
	if (verbose >= 2) {
		printf("Sent    : ");vg.print();
		printf("Received: ");reception.print();
	}

	unsigned char data8c[4] = {0x03, 0x0a, 0x0b, 0x0e};
	vg.reset(); vg.setId(0x8c);  vg.setData(data8c,4);
	if (!vg.send(videosock)) return false;
	reception.receive(videosock,100);
	if (verbose >= 2) {
		printf("Sent    : ");vg.print();
		printf("Received: ");reception.print();
	}

	vg.reset(); vg.setId(0x8d); 
	if (!vg.send(videosock)) return false;
	reception.receive(videosock,100);
	if (verbose >= 2) {
		printf("Sent    : ");vg.print();
		printf("Received: ");reception.print();
	}

	vg.reset(); vg.setId(0x87); 
	if (!vg.send(videosock)) return false;
	reception.receive(videosock,100);
	if (verbose >= 2) {
		printf("Sent    : ");vg.print();
		printf("Received: ");reception.print();
	}

	unsigned char data91[4] = {0x00, 0x00, 0x00, 0x00};
	vg.reset(); vg.setId(0x91); vg.setData(data91,4); 
	if (!vg.send(videosock)) return false;
	reception.receive(videosock,100);
	if (verbose >= 2) {
		printf("Sent    : ");vg.print();
		printf("Received: ");reception.print();
	}

	if (verbose >= 1)
		printf("Video initialization completed\n");
	return true;
}

bool VideoManager::pause()
{
	paused = true;
	return true;
}

bool VideoManager::resume()
{
	paused = false;
	requestNextImage();
	return true;
}

void VideoManager::setMaxFrameRate()
{
	interImageDelay = 0;
}

void VideoManager::setFrameRate(double fps)
{
	if (fps < 0) return;
	if (isinf(fps))  {
		interImageDelay = 0;
	} else {
		interImageDelay = 1.0/fps;
		if (interImageDelay > 0.005) {
			// 5 millisecond for the communication
			interImageDelay -= 0.005;
		}
	}
}

double VideoManager::getRequiredFrameRate()
{
	if (interImageDelay == 0) 
		return INFINITY;
	return 1.0/(interImageDelay+0.01);
}

double VideoManager::getObservedFrameRate()
{
	return observedFrameRate;
}


void VideoManager::setVideoOutputColorSpace(JpegReader::ColorSpace cspace)
{
	jpeg->setOutputColorSpace(cspace);
}

void VideoManager::setRecordingDestination(const char * dirname)
{
	ssaver.setDirectory(dirname);
}

void VideoManager::setRecordingBasename(const char * bname)
{
	ssaver.setBase(bname);
}

