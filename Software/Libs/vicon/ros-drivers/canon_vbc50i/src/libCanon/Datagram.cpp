#include <assert.h>

#include <stdlib.h>
#include <string.h>

#include "canon_vbc50i/libCanon/Datagram.h"

RawDatagram::RawDatagram()
{
	_blength = 16;
	_data = (unsigned char *)malloc(16);
	assert(_data != NULL);
	reset();
}

RawDatagram::RawDatagram(const RawDatagram & dgm)
{
	_blength = dgm._blength;
	_data = (unsigned char *)malloc(_blength);
	assert(_data != NULL);
	memcpy(_data,dgm._data,_blength);
}

const RawDatagram & RawDatagram::operator=(const RawDatagram & dgm)
{
	_blength = dgm._blength;
	_data = (unsigned char *)realloc(_data,_blength);
	assert(_data != NULL);
	memcpy(_data,dgm._data,_blength);
	return *this;
}


RawDatagram::~RawDatagram()
{
	free(_data);
}

void RawDatagram::reset()
{
	memset(_data,0,_blength);
}

void RawDatagram::resize(unsigned int len)
{
	if (len < 16) len = 16;
	if ((len % 4) != 0) len += 4 - (len % 4);
	_blength = len;
	//printf("Resized to %d\n",len);
	_data = (unsigned char *)realloc(_data,_blength);
	assert(_data != NULL);
}

bool RawDatagram::read(FILE * fp)
{
	unsigned int i,len;
	unsigned int x0,x1;
	if (fp == NULL) return false;
	if (fscanf(fp," %X %X ",&x0,&x1) != 2) 
		return false;
	_data[0] = x0; _data[1] = x1;
	len = _data[0];
	len = (len << 8) | _data[1];
	resize(16 + 4*len);
	for (i=2;i<16+4*len;i++) {
		if (fscanf(fp," %X ",&x0) != 1)
			return false;
		_data[i] = x0;
	}
	return true;
}

bool RawDatagram::read(const std::vector<unsigned char> & s)
{
	std::vector<unsigned char> c = s;
	unsigned int i,len;
	unsigned int x0,x1;
	if (c.size() < 16) {
		while (c.size() < 16) c.push_back(0);
	}
	x0 = c[0]; x1 = c[1];
	_data[0] = x0; _data[1] = x1;
	len = _data[0];
	len = (len << 8) | _data[1];
	resize(16 + 4*len);
	for (i=2;i<16+4*len;i++) {
		if (i >= c.size()) break;
		_data[i] = c[i];
	}
	return true;
}

bool RawDatagram::read(const unsigned char * s, unsigned int slen)
{
	unsigned int i,len;
	unsigned int x0,x1;
	if (slen < 16) return false;

	x0 = s[0]; x1 = s[1];
	_data[0] = x0; _data[1] = x1;
	len = _data[0];
	len = (len << 8) | _data[1];
	resize(16 + 4*len);
	for (i=2;i<16+4*len;i++) {
		if (i >= slen) break;
		_data[i] = s[i];
	}
	return true;
}

bool RawDatagram::write(FILE * fp) const
{
	unsigned int i,len;
	if (fp == NULL) return false;
	len = _data[0];
	len = (len << 8) | _data[1];
	for (i=0;i<16+4*len;i++) {
		fprintf(fp,"%02X ",_data[i]);
	}
	return true;
}

bool RawDatagram::receive(Socket * sock, unsigned int timeout, bool verbose)
{
	unsigned int len;
	if (sock == NULL) return false;
	if (!sock->WaitBuffer(_data,2,timeout)) {
		if (verbose) printf("Can't receive length\n");
		return false;
	}
	len = _data[0];
	len = (len << 8) | _data[1];
	if (len > 1000) {
		if (verbose) printf("Inconsistent message body length\n");
		return false;
	}
	resize(16 + 4*len);
	if (!sock->WaitBuffer(_data+2,14+4*len,100)) {
		if (verbose) printf("Can't receive message body\n");
		return false;
	}
	return true;
}

bool RawDatagram::send(Socket * sock)
{
	unsigned int len;
	if (sock == NULL) return false;
	len = _data[0];
	len = (len << 8) | _data[1];
	return sock->SendAll(_data,16+4*len,100);
}

void Datagram::setId(unsigned char id)
{
	_data[5] = id;
}

void Datagram::setStatus(unsigned char s1, unsigned char s2)
{
	_data[6] = s1;
	_data[7] = s2;
}

void Datagram::setStatus(unsigned short s)
{
	_data[6] = s >> 8;
	_data[7] = s & 0xFF;
}

unsigned char Datagram::getId() const 
{
	return _data[5];
}

unsigned short Datagram::getStatus() const
{
	return (((unsigned short)_data[6])<<8) | _data[7];
}

void Datagram::setData(unsigned char c, unsigned int pos)
{
	if ((pos + 8) >= _blength) {
		resize(pos+8);
	}
	_data[pos+8] = c;
}

void Datagram::setData(unsigned char * bd, unsigned int nb)
{
	resize(nb+8);
	memcpy(_data+8,bd,nb);
	unsigned int qlen = _blength/4;
	unsigned int qextralen = qlen - 4;
	//printf("blength = %d -> qel = %d\n",_blength,qextralen);
	_data[0] = qextralen >> 8;
	_data[1] = qextralen & 0xFF;
}

unsigned int Datagram::getBLength() const
{
	return _blength - 8;
}

const unsigned char * Datagram::bdata() const 
{
	return _data + 8;
}

void Datagram::print(FILE * fp) const
{
	unsigned int i;
	printf("Id %02X Status %04X Data: ",getId(),getStatus());
	for (i=0;i<getBLength();i++) {
		printf("%02X ",bdata()[i]);
	}
	//printf("\nRaw: ");
	//write(fp);
	printf("\n");
}


