#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#include "../libSock/Socket.h"
#include "JpegReader.h"
#include "JpegStreamSaver.h"
#include "Videogram.h"

class VideoManager
{
	public :
		typedef void (*SignalF)(void*,const JpegReader *,unsigned int);
	protected :
		VideoGram reception;
		double lastFrameTimeStamp;
		double interImageDelay;
		double observedFrameRate;
		bool paused;
		bool readyToRequest;
		unsigned char * destination;

		unsigned int verbose;
		bool terminate;

		unsigned int frame;
		SignalF signal;
		void * signalarg;
		pthread_t rec_thr_id;
		pthread_t req_thr_id;
		pthread_mutex_t sockMtx;
		pthread_mutex_t initMtx;
		pthread_mutex_t reqMtx;
		pthread_cond_t reqCond;

		unsigned int width, height;
		JpegReader *jpeg;
		JpegStreamSaver ssaver;
		bool savestream; 
			
		bool decodeImage();
		bool receiveOneFrame();
		bool requestNextImage();
		bool requestNextImageIfNecessary();
		bool checkSocketState();
		bool sendInitSeq();

		Socket * videosock;

		friend void * video_reception_thread(void * thrarg);
		friend void * video_request_thread(void * thrarg);
		bool reconnect();

	public :
		VideoManager();
		~VideoManager();

		bool open(const char * hostname, unsigned int port);
		bool close();

		bool isPaused() const {return paused;}
		bool pause();
		bool resume();

		bool isRecording() const {return savestream;}
		void startRecording() {savestream = true;}
		void stopRecording() {savestream = false;}
		void setRecordingDestination(const char * dirname);
		void setRecordingBasename(const char * bname);

		/** 768x576, 384x288, 192x144, 96x72 **/
		// TODO : test other modes
		bool setImageSize(unsigned int width, unsigned int height);
		bool getImageSize(unsigned int *width, unsigned int *height);


		void setDestination(unsigned char * dst) {destination = dst;}
		void setSignalFunction(SignalF f, void * arg) {
			signal = f; signalarg = arg;
		}

		void setVerboseLevel(unsigned int v); 

		void setMaxFrameRate();
		void setFrameRate(double fps);
		double getRequiredFrameRate();
		double getObservedFrameRate();
		void setVideoOutputColorSpace(JpegReader::ColorSpace cspace);
};




#endif // VIDEO_MANAGER_H
