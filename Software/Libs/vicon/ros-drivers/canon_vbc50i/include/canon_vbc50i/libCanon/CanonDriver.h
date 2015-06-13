#ifndef CANON_DRIVER_H
#define CANON_DRIVER_H

#include <vector>
#include "../libSock/Socket.h"
#include "Datagram.h"

#include "CommManager.h"
#include "VideoManager.h"

class CanonDriver
{
    public :
	const static unsigned int CONTROL_PORT = 65311;
	const static unsigned int VIDEO_PORT = 65310;
    protected :

	bool connected;
	unsigned int verbose;
	CommManager cm;
	VideoManager vm;
	char * host;

	bool sendRequestAndWait(unsigned char id, unsigned short status, 
		Datagram & dgm,unsigned char * data=NULL, 
		unsigned int datasize=0);
	bool sendAndWait(unsigned char id, unsigned short status, 
		unsigned char * data=NULL, unsigned int datasize=0);
	bool sendAndWait33(unsigned char id, unsigned short status, 
		unsigned char * data=NULL, unsigned int datasize=0);
	bool interpreteDatagram33(const Datagram & dgm);
	double cpan,ctilt,czoom;

    public :
	CanonDriver(const char * initseq_filename,
		const char * hostname);
	~CanonDriver();

	bool connect();
	bool disconnect();
	bool requestCurrentPos();
	void getCurrentPos(double * pan, double * tilt, double * zoom);

	bool moveTo(double pan, double tilt, double zoom);
	bool panTo(double pan);
	bool tiltTo(double to);
	bool zoomTo(double zoom);

	bool center();
	typedef enum {
	    None=0, Right=1, Left=2, Up=4, UpRight=5, 
	    UpLeft=6, Down=8, DownRight=9, DownLeft=10
	} Direction;
	bool startMoving(Direction dir);
	bool stop();

	bool startDeZooming();
	bool startZooming();
	bool stopZooming();

	// speed in 0x221 - 0x884 (degree/sec)
	bool setPanSpeed(unsigned short speed);
	// speed in 0x198 - 0x663 (degree/sec)
	bool setTiltSpeed(unsigned short speed);
	// speed in 1 - 7 (used in the win app)
	bool setZoomSpeed(unsigned short speed);
	bool getSpeeds(unsigned short * pan, unsigned short * tilt,
		unsigned short * zoom);

	bool setMaxSpeed();
	bool setMinSpeed();

	typedef enum {FMAuto=0, FMAutoDomes=3, FMInfinity=2, FMManual=1} FocusMode;
	typedef enum {FMUndef=0, FMManualFar=1, FMManualNear=2} ManualFocusMode;
	bool setFocusMode(FocusMode fm, ManualFocusMode mfm=FMUndef);
	bool getFocusMode(FocusMode * fm, ManualFocusMode * mfm);

	/** TODO : find the meaning of these modes **/
	typedef enum {AXPundef=0,AXPmode1=0x120,AXPmode2=0x1b0,AXPmode3=0x240,AXPmode4=0x2d0} AutoExposureMode;
	bool getAutoExposure(bool * autoexp, AutoExposureMode * aem);
	bool setAutoExposure(AutoExposureMode aem = AXPundef);
	bool setManualExposure();
	/***
	 * aperture in 0x00 - 0x0F
	 * inv_shutter in 0x01 - 0xfa (1/1 sec to 1/250 sec)
	 * gain in 0x01 - 0x100
	 * **/
	bool setExposureParameters(unsigned int aperture, 
		unsigned int inv_shutter, unsigned int gain);
	bool getExposureParameters(unsigned int * aperture, 
		unsigned int * inv_shutter, unsigned int * gain);

	bool setDigitalZoom(unsigned char zoom);
	unsigned char getDigitalZoom();

	bool setNightMode(bool activated=true);
	bool getNightMode();
	bool setInfraRed(bool activated=true);
	bool getInfraRed();

	/** 768x576, 384x288, 192x144, 96x72 **/
	// TODO : test other modes
	bool setImageSize(unsigned int width, unsigned int height);
	bool getImageSize(unsigned int  *width, unsigned int *height);

	bool startVideoReception(unsigned char * dst, 
		VideoManager::SignalF f, void * farg=NULL);
	bool startVideoReception(VideoManager::SignalF f,
		void * farg=NULL);
	void setVideoOutputColorSpace(JpegReader::ColorSpace cspace);
	bool pauseVideoReception();
	bool resumeVideoReception();
	bool setVideoReceptionPauseStatus(bool pause);
	bool getVideoReceptionPauseStatus();
	bool getVideoRecordingStatus() const;
	void setVideoRecordingMode(bool mode);
	bool stopVideoReception();		
	void setRecordingDestination(const char * dirname);
	void startVideoRecording();		
	void stopVideoRecording();		
	void setMaxFrameRate();
	void setFrameRate(double fps);
	double getRequiredFrameRate();
	double getObservedFrameRate();

	void setVerboseLevel(unsigned int v) {
	    verbose = v;
	    printf("Verbose level set to %d\n",verbose);
	}

	bool sendDatagram(const std::vector<unsigned char> & dgm);

};

#endif // CANON_DRIVER_H
