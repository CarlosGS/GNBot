#ifndef VIDEO_GRAM_H
#define VIDEO_GRAM_H

#include <stdio.h>
#include "../libSock/Socket.h"

class VideoGram
{
	protected :
		double timestamp;
		unsigned int size;
		unsigned char * data;
	public :
		VideoGram(unsigned int id=0);
		~VideoGram();

		void reset();
		void setId(unsigned int id);
		void setData(unsigned char * s, unsigned int l);

		double getTimeStamp();
		unsigned int getId();
		unsigned int getSize();
		unsigned int getLength();
		unsigned char * getData();
		const unsigned char * getData() const;

		bool send(Socket * sock) const;
		bool receive(Socket * sock, unsigned int timeout_milli);
		void print(FILE * fp=stdout);
};




#endif // VIDEO_GRAM_H

