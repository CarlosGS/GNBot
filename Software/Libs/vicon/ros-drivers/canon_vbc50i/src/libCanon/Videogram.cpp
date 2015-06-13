#include <sys/time.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "canon_vbc50i/libCanon/Videogram.h"


VideoGram::VideoGram(unsigned int id)
{
	timestamp = 0;
	size = 8;
	data = (unsigned char*)malloc(size);
	assert(data != NULL);
	data[0] = id;
	bzero(data+1,size-1);
}

VideoGram::~VideoGram()
{
	free(data);
}

void VideoGram::reset()
{
	assert(data != NULL);
	timestamp = 0;
	bzero(data,size);
}

void VideoGram::setId(unsigned int id)
{
	data[0] = id;
}

unsigned int VideoGram::getId()
{
	return data[0];
}

void VideoGram::setData(unsigned char * s, unsigned int l)
{
	size = 8 + l; 
	data = (unsigned char *)realloc(data,size);
	assert(data != NULL);
	memcpy(data+8,s,l);
	data[4] = (l >> 24) & 0xFF;
	data[5] = (l >> 16) & 0xFF;
	data[6] = (l >>  8) & 0xFF;
	data[7] = (l >>  0) & 0xFF;
}

unsigned int VideoGram::getSize()
{
	return size;
}

unsigned int VideoGram::getLength()
{
	return size - 8;
}
	
unsigned char * VideoGram::getData()
{
	return data + 8;
}

const unsigned char * VideoGram::getData() const
{
	return data + 8;
}


bool VideoGram::send(Socket * sock) const
{
	return sock->SendAll(data,size,100);
}


bool VideoGram::receive(Socket * sock, unsigned int timeout_milli)
{
	reset();
	if (!sock->WaitBuffer(data,8,timeout_milli)) {
		return false;
	}
	struct timeval tv;
	gettimeofday(&tv,NULL);
	timestamp = tv.tv_sec + tv.tv_usec*1e-6;

	unsigned int l = 0;
	l = (l<<8) + data[4];
	l = (l<<8) + data[5];
	l = (l<<8) + data[6];
	l = (l<<8) + data[7];
	if (data[0] == 0x02) {
		if (l > 2e6) return false;
	} else {
		if (l > 1e3) return false;
	}
	size = l + 8;
	data = (unsigned char *)realloc(data,size);
	assert(data != NULL);
	if (!sock->WaitBuffer(data+8,l,timeout_milli)) {
		return false;
	}
	return true;
}

void VideoGram::print(FILE * fp)
{
	unsigned int i;
	fprintf(fp,"Id %02X [%02X %02X %02X] L %X : [ ",data[0],data[1],data[2],data[3],size-8);
	for (i=8;i<size;i++) {
		fprintf(fp,"%02X ",data[i]);
	}
	fprintf(fp,"]\n");
}

double VideoGram::getTimeStamp()
{
	return timestamp;
}


