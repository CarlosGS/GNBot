#ifndef BASECOM_H
#define BASECOM_H

#include "Serializer.h"

#define REQ_DATA 1
#define REQ_RESULT 2
#define REQ_NOMOREDATA 3

struct BaseCom
{
	unsigned char typeRequest;
	unsigned long dataLength;
	unsigned long id;
	unsigned packsize()
	{
		unsigned int s = 0;
		s += Serializer::packsize(typeRequest);
		s += Serializer::packsize(dataLength);
		s += Serializer::packsize(id);
		return s;
	}

	unsigned char * pack(unsigned char * buffer)
	{
		buffer = Serializer::pack(typeRequest,buffer);
		buffer = Serializer::pack(dataLength,buffer);
		buffer = Serializer::pack(id,buffer);
		return buffer;
	}

	const unsigned char * unpack(const unsigned char * buffer)
	{
		buffer = Serializer::unpack(&typeRequest,buffer);
		buffer = Serializer::unpack(&dataLength,buffer);
		buffer = Serializer::unpack(&id,buffer);
		return buffer;
	}

};
	
	
#endif // BASECOM_H
