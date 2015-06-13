
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "canon_vbc50i/libCanon/CanonDriver.h"

CanonDriver::CanonDriver(const char * initseq_filename,
		const char * hostname) : cm(initseq_filename)
{
	host = strdup(hostname);
	connected = false;
	verbose = 0;
}

CanonDriver::~CanonDriver() 
{
	disconnect();
	stopVideoReception();
	free(host); host = NULL;
}



bool CanonDriver::connect()
{
	cm.setVerboseLevel(verbose);
	if (!cm.open(host,CONTROL_PORT)) {
		printf("%s: Can't open CommManager '%s:%d'\n",__FUNCTION__,
				host,CONTROL_PORT);
		return false;
	}
	connected = true;
	return requestCurrentPos();	
}

bool CanonDriver::startVideoReception(VideoManager::SignalF f,
		void * farg)
{
	return startVideoReception(NULL,f,farg);
}

bool CanonDriver::startVideoReception(unsigned char * dst, 
		VideoManager::SignalF f, void * farg)
{
	vm.setDestination(dst);
	vm.setRecordingBasename(host);
	vm.setSignalFunction(f,farg);

	printf("Opening video reception\n");
	if (!vm.open(host,VIDEO_PORT)) {
		printf("%s: Can't open VideoManager '%s:%d'\n",__FUNCTION__,
				host,VIDEO_PORT);
		return false;
	}
	vm.setVerboseLevel(verbose);
	return true;
}

bool CanonDriver::stopVideoReception()
{
	return vm.close();
}

bool CanonDriver::setVideoReceptionPauseStatus(bool pause)
{
	if (vm.isPaused() == pause) 
		return true;
	if (vm.isPaused()) 
		return vm.resume();
	else
		return vm.pause();
}

bool CanonDriver::getVideoReceptionPauseStatus()
{
	return vm.isPaused();
}

bool CanonDriver::pauseVideoReception()
{
	return vm.pause();
}

bool CanonDriver::resumeVideoReception()
{
	return vm.resume();
}

bool CanonDriver::disconnect()
{
	//printf("Sending 0x21\n");
	if (connected) {
		if (!sendAndWait(0x21,0x0000)) {
			if (verbose) 
				fprintf(stderr,"Failed to send 0x21\n");
		}
	}
	//printf("Sent 0x21\n");
	cm.close();
	//printf("Close comm manager\n");
	connected = false;

	stopVideoReception();
	//printf("Close video manager\n");

	return true;
}


bool CanonDriver::moveTo(double pan, double tilt, double zoom)
{
	unsigned char data[6];
	signed short span,stilt,szoom;
	//printf("P %f T %f Z %f\n",pan,tilt,zoom);
	if (pan < -170) pan = -170;
	if (pan > +170) pan = +170;
	if (tilt < -90) tilt = -90;
	if (tilt > 10) tilt = 10;
	if (zoom < 1.97) zoom = 1.97;
	if (zoom > 41.26) zoom = 41.26;
	//printf("P %f T %f Z %f\n",pan,tilt,zoom);

	span = (signed short)round(pan*100.0);
	stilt = (signed short)round(tilt*100.0);
	szoom = (signed short)round(zoom*100.0);

	unsigned short upan,utilt,uzoom;
	upan = (unsigned short)span;
	utilt = (unsigned short)stilt;
	uzoom = (unsigned short)szoom;
	//printf("P %02X T %02X Z %02X\n",upan,utilt,uzoom);

	data[0] = upan >> 8;
	data[1] = (upan & 0xFF);
	data[2] = utilt >> 8;
	data[3] = (utilt & 0xFF);
	data[4] = uzoom >> 8;
	data[5] = (uzoom & 0xFF);

	bool res = sendAndWait(0x33,0x00e0,data,6);
	if (!res && (verbose > 0)) printf("Move To Failed\n");
	return res;
}

bool CanonDriver::panTo(double pan)
{
	requestCurrentPos();
	return moveTo(pan,ctilt,czoom);
}

bool CanonDriver::tiltTo(double tilt)
{
	requestCurrentPos();
	return moveTo(cpan,tilt,czoom);
}

bool CanonDriver::zoomTo(double zoom)
{
	requestCurrentPos();
	return moveTo(cpan,ctilt,zoom);
}

bool CanonDriver::center()
{
	requestCurrentPos();
	return moveTo(0,0,czoom);
}

bool CanonDriver::startMoving(Direction dir) 
{
	unsigned char data = dir;
	return sendAndWait(0x3C,0x0000,&data,1);
}


bool CanonDriver::stop() 
{
	return sendAndWait33(0x3C,0x0000);
}

bool CanonDriver::requestCurrentPos()
{
	Datagram dgm;
	dgm.reset();
	dgm.setId(0x33);
	dgm.setStatus(0x8000);

	cm.addToReceptionField(0x33);
	if (!cm.send(dgm)) {
		if (verbose) {
			printf("%s: Can't send datagram: ",__FUNCTION__);
			dgm.print(stdout);
			printf("\n");
		}
		goto cleanup;
	}

	if (!cm.wait(0x33,dgm)) goto cleanup;
	cm.removeFromReceptionField(0x33);
	return interpreteDatagram33(dgm);

cleanup:
	if (verbose)
		printf("requestCurrentPos failed\n");
	cm.removeFromReceptionField(0x33);
	return false;
}

void CanonDriver::getCurrentPos(double * pan, double * tilt, double * zoom)
{
	requestCurrentPos();
	if (pan != NULL) *pan = cpan ;
	if (tilt != NULL) *tilt = ctilt ;
	if (zoom != NULL) *zoom = czoom ;
	//printf("At position: %f %f %f\n", cpan,ctilt,czoom);
}

bool CanonDriver::startZooming() 
{
	unsigned char data = 0x01;
	return sendAndWait(0x3D,0x0000,&data,1);
}

bool CanonDriver::stopZooming() 
{
	return sendAndWait33(0x3D,0x0000);
}

bool CanonDriver::startDeZooming() 
{
	unsigned char data = 0x02;
	return sendAndWait(0x3D,0x0000,&data,1);
}


bool CanonDriver::setPanSpeed(unsigned short speed) 
{
	unsigned char data[8] = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x80, 0x00
	};
	data[0] = speed >> 8;
	data[1] = speed & 0xFF;
	return sendAndWait(0x3B,0x0000,data,8);
}

bool CanonDriver::setTiltSpeed(unsigned short speed) 
{
	unsigned char data[8] = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x40, 0x00
	};
	data[2] = speed >> 8;
	data[3] = speed & 0xFF;
	return sendAndWait(0x3B,0x0000,data,8);
}

bool CanonDriver::setZoomSpeed(unsigned short speed) 
{
	unsigned char data[8] = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x20, 0x00
	};
	data[4] = speed >> 8;
	data[5] = speed & 0xFF;
	return sendAndWait(0x3B,0x0000,data,8);
}

bool CanonDriver::setMaxSpeed()
{
	unsigned char data[8] = {
		0x08, 0x84, 0x06, 0x63,
		0x00, 0x07, 0xE0, 0x00
	};
	return sendAndWait(0x3B,0x0000,data,8);
}

bool CanonDriver::setMinSpeed()
{
	unsigned char data[8] = {
		0x02, 0x21, 0x01, 0x98,
		0x00, 0x00, 0xE0, 0x00
	};
	return sendAndWait(0x3B,0x0000,data,8);
}

bool CanonDriver::getSpeeds(unsigned short * pan, 
		unsigned short * tilt, unsigned short * zoom)
{
	Datagram dgm;
	if (!sendRequestAndWait(0x3B,0x8000,dgm))
		return false;
	const unsigned char * data = dgm.bdata();
	*pan = data[0];
	*pan = (*pan << 8) | data[1];
	*tilt = data[2];
	*tilt = (*tilt << 8) | data[3];
	*zoom = data[4];
	*zoom = (*zoom << 8) | data[5];
	return true;
}

bool CanonDriver::setFocusMode(FocusMode fm, ManualFocusMode mfm)
{
	unsigned char data = fm;
	if (!sendAndWait(0x40,0x0000,&data,1)) return false;

	if ((fm == FMManual) && (mfm != FMUndef)) {
		// This sequence used in the win app
		data = 0;
		if (!sendAndWait(0x43,0x0000,&data,1)) return false;
		data = mfm;
		if (!sendAndWait(0x43,0x0000,&data,1)) return false;
		data = 0;
		if (!sendAndWait(0x43,0x0000,&data,1)) return false;
	}

	return true;

}

bool CanonDriver::getFocusMode(FocusMode * fm, ManualFocusMode * mfm)
{
	Datagram dgm;
	if (!sendRequestAndWait(0x40,0x8000,dgm))
		return false;
	*fm = (FocusMode)(dgm.bdata()[0]);
	if (*fm == FMManual) {
		if (!sendRequestAndWait(0x43,0x8000,dgm))
			return false;
		*mfm = (ManualFocusMode)(dgm.bdata()[0]);
	}
	return true;
}

bool CanonDriver::getAutoExposure(bool * autoexp, AutoExposureMode * aem)
{
	Datagram dgm;
	if (!sendRequestAndWait(0x82,0x8000,dgm))
		return false;
	//dgm.print();
	*autoexp = (dgm.bdata()[0] == 0x00);
	if (!*autoexp) {
		*aem = AXPundef;
		return true;
	}
	if (!sendRequestAndWait(0x87,0x8000,dgm))
		return false;
	unsigned short us = dgm.bdata()[0];
	us = (us << 8) | dgm.bdata()[1];
	*aem = (AutoExposureMode)(us);
	//dgm.print();
	return true;
}


bool CanonDriver::setAutoExposure(AutoExposureMode aem)
{
	if (!sendAndWait(0x82,0x0000)) return false;
	if (aem != AXPundef) {
		unsigned char data[2];
		data[0] = ((unsigned int)aem) >> 8;
		data[1] = ((unsigned int)aem) & 0xFF;
		if (!sendAndWait(0x87,0x0000,data,2)) return false;
	}
	return true;
}

bool CanonDriver::setManualExposure()
{
	return setExposureParameters(0x07,0x64,0x80);
}

bool CanonDriver::setExposureParameters(unsigned int aperture, 
		unsigned int inv_shutter, unsigned int gain)
{
	unsigned char data[16] = {
		0xE0, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};
	if ((aperture<1) || (aperture >15)) return false;
	if ((gain<1) || (gain >256)) return false;
	if ((inv_shutter<1) || (inv_shutter >250)) return false;
	data[7] = aperture;
	data[11] = inv_shutter;
	data[14] = gain >> 8;
	data[15] = gain & 0xFF;
	bool r = sendAndWait(0x82,0x0000,data,16);
	return r;
}

bool CanonDriver::getExposureParameters(unsigned int * aperture, 
		unsigned int * inv_shutter, unsigned int * gain)
{
	unsigned char c = 0xE0;
	Datagram dgm;
	if (!sendRequestAndWait(0x82,0x8000, dgm,&c,1)) return false;
	const unsigned char * data = dgm.bdata();
	//dgm.print();
	*aperture = data[7];
	*inv_shutter = data[11];
	*gain = data[14];
	*gain = (*gain << 8) | data[15];
	//printf("a %d is %d g %d\n",*aperture,*inv_shutter,*gain);
	return true;
}

bool CanonDriver::setDigitalZoom(unsigned char zoom)
{
	return sendAndWait(0x88,0x0000,&zoom,1);
}

bool CanonDriver::setNightMode(bool activated)
{
	unsigned char data = (activated)?0x01:0x00;
	return sendAndWait(0x85,0x0000,&data,1);
}

bool CanonDriver::setInfraRed(bool activated)
{
	unsigned char data = (activated)?0x01:0x00;
	return sendAndWait(0x86,0x0000,&data,1);
}


unsigned char CanonDriver::getDigitalZoom()
{
	Datagram dgm;
	if (!sendRequestAndWait(0x88,0x8000,dgm)) return 0;
	//dgm.print();
	return dgm.bdata()[1];
}

bool CanonDriver::getNightMode()
{
	Datagram dgm;
	if (!sendRequestAndWait(0x85,0x8000,dgm)) return false;
	//dgm.print(stdout);
	return dgm.bdata()[0] == 0x01;
}

bool CanonDriver::getInfraRed()
{
	Datagram dgm;
	if (!sendRequestAndWait(0x86,0x8000,dgm)) return false;
	return dgm.bdata()[0] == 0x01;
}

bool CanonDriver::setImageSize(unsigned int width, unsigned int height)
{
	return vm.setImageSize(width,height);
}

bool CanonDriver::getImageSize(unsigned int *width, unsigned int *height)
{
	return vm.getImageSize(width,height);
}

bool CanonDriver::sendRequestAndWait(unsigned char id, 
		unsigned short status, Datagram & dgm,
		unsigned char * data, unsigned int datasize)
{
	Datagram dgmreq;
	dgmreq.reset();
	dgmreq.setId(id);
	dgmreq.setStatus(status);
	if (data != NULL) {
		dgmreq.setData(data,datasize);
	}

	//printf("Sending: ");dgmreq.print(stdout);
	cm.addToReceptionField(id);
	if (!cm.send(dgmreq)) {
		if (verbose) {
			printf("%s: Can't send datagram: ",__FUNCTION__);
			dgm.print(stdout);
			printf("\n");
		}
		goto cleanup;
	}
	dgm.reset();
	if (!cm.wait(id,dgm,1.0)) {
		if (verbose) printf("Failed to receive reply dgm %02X\n",id);
		goto cleanup;
	}
	cm.removeFromReceptionField(id);
	return true;
cleanup:
	cm.removeFromReceptionField(id);
	return false;
}

bool CanonDriver::sendAndWait(unsigned char id, unsigned short status, 
		unsigned char * data, unsigned int datasize)
{
	Datagram dgm;
	dgm.reset();
	dgm.setId(id);
	dgm.setStatus(status);
	if (data != NULL) {
		dgm.setData(data,datasize);
	}


	cm.addToReceptionField(id);
	if (!cm.send(dgm)) {
		if (verbose) {
			printf("%s: Can't send datagram: ",__FUNCTION__);
			dgm.print(stdout);
			printf("\n");
		}
		goto cleanup;
	}
	if (!cm.wait(id,dgm,1.0)) {
		if (verbose > 1)
			printf("SaW: Failed to receive ack dgm %02X\n",id);
		goto cleanup;
	}
	if (verbose) {printf("Received dgm %02X\n",id); dgm.print();}
	cm.removeFromReceptionField(id);
	return true;
cleanup:
	cm.removeFromReceptionField(id);
	return false;
}

bool CanonDriver::sendAndWait33(unsigned char id, unsigned short status, 
		unsigned char * data, unsigned int datasize)
{
	Datagram dgm;
	dgm.reset();
	dgm.setId(id);
	dgm.setStatus(status);
	if (data != NULL) {
		dgm.setData(data,datasize);
	}


	cm.addToReceptionField(id);
	cm.addToReceptionField(0x33);
	if (!cm.send(dgm)) {
		if (verbose) {
			printf("%s: Can't send datagram: ",__FUNCTION__);
			dgm.print(stdout);
			printf("\n");
		}
		goto cleanup;
	}
	if (!cm.wait(id,dgm,1.0)) {
		if (verbose > 1)
			printf("SaW33-1: Failed to receive ack dgm %02X\n",id);
		goto cleanup;
	}
	if (!cm.wait(0x33,dgm,1.0)) {
		if (verbose > 1)
			printf("SaW33-2: Failed to receive ack dgm %02X\n",0x33);
		goto cleanup;
	}
	cm.removeFromReceptionField(id);
	cm.removeFromReceptionField(0x33);

	return interpreteDatagram33(dgm);
cleanup:
	cm.removeFromReceptionField(id);
	cm.removeFromReceptionField(0x33);
	return false;
}

bool CanonDriver::interpreteDatagram33(const Datagram & dgm)
{
	if (dgm.getId() != 0x33) return false;
	const unsigned char * rdata = dgm.bdata();
	cpan = (signed short)(((unsigned short)(rdata[0]) << 8) | rdata[1]);
	cpan /= 100.0;
	ctilt = (signed short)(((unsigned short)(rdata[2]) << 8) | rdata[3]);
	ctilt /= 100.0;
	czoom = (signed short)(((unsigned short)(rdata[4]) << 8) | rdata[5]);
	czoom /= 100.0;
	return true;
}

bool CanonDriver::sendDatagram(const std::vector<unsigned char> & dgm)
{
	Datagram dgram;
	dgram.read(dgm);
	dgram.print();
	unsigned int id = dgram.getId();
	cm.setVerboseLevel(15);
	cm.addToReceptionField(id);
	if (!cm.send(dgram)) {
		if (verbose) {
			printf("%s: Can't send datagram: ",__FUNCTION__);
			dgram.print(stdout);
			printf("\n");
		}
		goto cleanup;
	}
	if (!cm.wait(id,dgram,1.0)) {
		if (verbose > 1)
			printf("SDgm: Failed to receive ack dgm %02X\n",id);
		goto cleanup;
	}
	cm.removeFromReceptionField(id);
	cm.setVerboseLevel(verbose);
	return true;
cleanup:
	cm.removeFromReceptionField(id);
	cm.setVerboseLevel(verbose);
	return false;
}

void CanonDriver::setMaxFrameRate()
{
	vm.setMaxFrameRate();
}

void CanonDriver::setFrameRate(double fps)
{
	vm.setFrameRate(fps);
}

double CanonDriver::getRequiredFrameRate()
{
	return vm.getRequiredFrameRate();
}

double CanonDriver::getObservedFrameRate()
{
	return vm.getObservedFrameRate();
}

void CanonDriver::setVideoOutputColorSpace(JpegReader::ColorSpace cspace)
{
	vm.setVideoOutputColorSpace(cspace);
}

void CanonDriver::startVideoRecording()
{
	vm.startRecording();
}

void CanonDriver::stopVideoRecording()
{
	vm.stopRecording();
}

bool CanonDriver::getVideoRecordingStatus() const
{
	return vm.isRecording();
}

void CanonDriver::setVideoRecordingMode(bool mode)
{
	if (mode)
		vm.startRecording();
	else 
		vm.stopRecording();
}

void CanonDriver::setRecordingDestination(const char * dirname)
{
	vm.setRecordingDestination(dirname);
}

















