
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "canon_vbc50i/libCanon/CanonDriver.h"

CanonDriver::CanonDriver(const char * initseq_filename,
		const char * hostname)
{
	Datagram dgm;
	ctrlsock = NULL;
	videosock = NULL;
	FILE * fp = fopen(initseq_filename,"r");
	assert(fp != NULL);
	host = strdup(hostname);
	while (1) {
		if (!dgm.read(fp)) break;
		init_seq.push_back(dgm);
	}
	init_reply.resize(init_seq.size());
	keepalive_seq.resize(4);
	keepalive_reply.resize(keepalive_seq.size());
	keepalive_seq[0].reset();
	keepalive_seq[0].setId(0x88);
	keepalive_seq[0].setStatus(0x80,0x00);
	keepalive_seq[1].reset();
	keepalive_seq[1].setId(0x82);
	keepalive_seq[1].setStatus(0x80,0x00);
	keepalive_seq[2].reset();
	keepalive_seq[2].setId(0x86);
	keepalive_seq[2].setStatus(0x80,0x00);
	keepalive_seq[3].reset();
	keepalive_seq[3].setId(0x85);
	keepalive_seq[3].setStatus(0x80,0x00);
}

CanonDriver::~CanonDriver() 
{
	disconnect();
}

bool CanonDriver::connect()
{
	if (ctrlsock != NULL) delete ctrlsock;
	ctrlsock = new Socket(host,CONTROL_PORT);
	if (!ctrlsock->Open()) {
		printf("%s: Can't open Socket '%s:%d'\n",__FUNCTION__,
				host,CONTROL_PORT);
		return false;
	}
	if (videosock != NULL) delete videosock;
	videosock = new Socket(host,VIDEO_PORT);
	if (!videosock->Open()) {
		printf("%s: Can't open videosocket '%s:%d'\n",__FUNCTION__,
				host,VIDEO_PORT);
		return false;
	}
	unsigned int i;
	for (i=0;i<init_seq.size();i++) {
		if (!init_seq[i].send(ctrlsock)) {
			printf("%s: Can't send datagram %d:",__FUNCTION__,i);
			init_seq[i].write(stdout);
			printf("\n");
			return false;
		}
		if (!init_reply[i].receive(ctrlsock)) {
			printf("%s: Can't receive datagram %d\n",__FUNCTION__,i);
			return false;
		}
	}
	printf("Initialization completed\n");
	return requestCurrentPos();	
}

bool CanonDriver::disconnect()
{
	if (ctrlsock != NULL) {
		//if (!ctrlsock->IsOpen()) return true;
		Datagram dgm;
		dgm.setId(0x21);
		dgm.setStatus(0x0000);
		if (!dgm.send(ctrlsock)) {
			printf("%s: Can't send datagram :",__FUNCTION__);
			dgm.print(stdout);
			printf("\n");
			return false;
		}
		dgm.reset();
		if (!dgm.receive(ctrlsock)) {
			printf("%s: Can't receive datagram\n",__FUNCTION__);
			return false;
		}
		if (dgm.getId() != 0x21) {
			printf("%s: received incorrect datagram\n",__FUNCTION__);
			return false;
		}
		ctrlsock->Close();
		delete ctrlsock;
		ctrlsock = NULL;

	}

	if (videosock != NULL) {
		videosock->Close();
		delete videosock;
		videosock = NULL;
	}

	return true;
}

bool CanonDriver::getframe()
{
	unsigned int i;
	unsigned char b[1024];
	if (!keepalive()) return false;
	if (videosock == NULL) return false;
	if (videosock->WaitBuffer(b,1024,1000)) {
		for (i=0;i<80;i++) {
			printf("%02X ",b[i]);
		}
	} else {
		printf("%s: Cannot receive frame\n",__FUNCTION__);
		return false;
	}
	return true;
}

bool CanonDriver::keepalive()
{
	if (ctrlsock == NULL) return false;
	//if (!ctrlsock->IsOpen()) return false;
	unsigned int i;
	for (i=0;i<keepalive_seq.size();i++) {
		if (!keepalive_seq[i].send(ctrlsock)) {
			printf("%s: Can't send datagram %d:",__FUNCTION__,i);
			keepalive_seq[i].write(stdout);
			printf("\n");
			return false;
		}
		if (!keepalive_reply[i].receive(ctrlsock)) {
			printf("%s: Can't receive datagram %d\n",__FUNCTION__,i);
			return false;
		}
	}
	return true;
}

bool CanonDriver::waitcompletion()
{
	unsigned char data[8] = {0x0e,0x6b,0x0a,0xd0, 0x00,0x00,0xc0,0x00};
	Datagram dgm;
	dgm.setId(0x3b);
	dgm.setStatus(0x0000);
	dgm.setData(data,8);
	
	if (!dgm.send(ctrlsock)) {
		printf("%s: Can't send datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	dgm.reset();
	if (!dgm.receive(ctrlsock,2000)) {
		printf("%s: Can't receive datagram\n",__FUNCTION__);
		return false;
	}
	if (dgm.getId() != 0x3b) {
		printf("%s: Received invalid datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	return true;
}

bool CanonDriver::moveto(double pan, double tilt, double zoom)
{
	if (ctrlsock == NULL) return false;
	//if (!ctrlsock->IsOpen()) return false;
	Datagram dgm;
	unsigned char data[6];
	signed short span,stilt,szoom;
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
	dgm.reset();
	dgm.setId(0x33);
	dgm.setStatus(0x00,0xe0);
	dgm.setData(data,6);

	if (!dgm.send(ctrlsock)) {
		printf("%s: Can't send datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	//printf("Sent: ");dgm.print(stdout);
	dgm.reset();
	if (!dgm.receive(ctrlsock)) {
		printf("%s: Can't receive datagram\n",__FUNCTION__);
		return false;
	}
	if (dgm.getId() != 0x33) {
		printf("%s: Received invalid datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	//printf("Rec : ");dgm.print(stdout);
	return waitcompletion();
}

bool CanonDriver::panto(double pan)
{
	requestCurrentPos();
	return moveto(pan,ctilt,czoom);
}

bool CanonDriver::tiltto(double tilt)
{
	requestCurrentPos();
	return moveto(cpan,tilt,czoom);
}

bool CanonDriver::zoomto(double zoom)
{
	requestCurrentPos();
	return moveto(cpan,ctilt,zoom);
}

bool CanonDriver::center()
{
	requestCurrentPos();
	return moveto(0,0,czoom);
}

bool CanonDriver::movetoward(Direction dir) 
{
	Datagram dgm;
	dgm.reset();
	dgm.setId(0x3C);
	dgm.setStatus(0x00,0x00);
	dgm.setData((unsigned char)dir,0);

	if (!dgm.send(ctrlsock)) {
		printf("%s: Can't send datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	dgm.reset();
	if (!dgm.receive(ctrlsock)) {
		printf("%s: Can't receive datagram\n",__FUNCTION__);
		return false;
	}
	if (dgm.getId() != 0x3C) {
		printf("%s: Received invalid datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	return true;
}
	
bool CanonDriver::stop() 
{
	Datagram dgm;
	dgm.reset();
	dgm.setId(0x3C);
	dgm.setStatus(0x00,0x00);

	if (!dgm.send(ctrlsock)) {
		printf("%s: Can't send datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	dgm.reset();
	if (!dgm.receive(ctrlsock)) {
		printf("%s: Can't receive datagram\n",__FUNCTION__);
		return false;
	}
	if (dgm.getId() != 0x3C) {
		printf("%s: Received invalid datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	if (!dgm.receive(ctrlsock,2000)) {
		printf("%s: Can't receive stop datagram\n",__FUNCTION__);
		return false;
	}
	if ((dgm.getId() != 0x33) || (dgm.getStatus() != 0x06E0)) {
		printf("%s: Received invalid datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	const unsigned char * rdata = dgm.bdata();
	cpan = (signed short)(((unsigned short)(rdata[0]) << 8) | rdata[1]);
	cpan /= 100.0;
	ctilt = (signed short)(((unsigned short)(rdata[2]) << 8) | rdata[3]);
	ctilt /= 100.0;
	czoom = (signed short)(((unsigned short)(rdata[4]) << 8) | rdata[5]);
	czoom /= 100.0;
	return true;
}

bool CanonDriver::requestCurrentPos()
{
	Datagram dgm;
	dgm.reset();
	dgm.setId(0x33);
	dgm.setStatus(0x8000);

	if (!dgm.send(ctrlsock)) {
		printf("%s: Can't send datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	dgm.reset();
	if (!dgm.receive(ctrlsock)) {
		printf("%s: Can't receive datagram\n",__FUNCTION__);
		return false;
	}
	if ((dgm.getId() != 0x33) || 
			((dgm.getStatus() & 0x00FF) != 0x00E0)) {
		printf("%s: Received invalid datagram: ",__FUNCTION__);
		dgm.print(stdout);
		printf("\n");
		return false;
	}
	const unsigned char * rdata = dgm.bdata();
	cpan = (signed short)(((unsigned short)(rdata[0]) << 8) | rdata[1]);
	cpan /= 100.0;
	ctilt = (signed short)(((unsigned short)(rdata[2]) << 8) | rdata[3]);
	ctilt /= 100.0;
	czoom = (signed short)(((unsigned short)(rdata[4]) << 8) | rdata[5]);
	czoom /= 100.0;
	return true;
}

void CanonDriver::getCurrentPos(double * pan, double * tilt, double * zoom)
{
	requestCurrentPos();
	if (pan != NULL) *pan = cpan ;
	if (tilt != NULL) *tilt = ctilt ;
	if (zoom != NULL) *zoom = czoom ;
	printf("At position: %f %f %f\n",
			cpan,ctilt,czoom);
}



