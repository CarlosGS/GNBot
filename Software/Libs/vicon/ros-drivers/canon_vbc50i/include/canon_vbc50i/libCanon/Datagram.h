#ifndef DATA_GRAM_H
#define DATA_GRAM_H

#include <stdio.h>
#include "../libSock/Socket.h"
#include <vector>

class RawDatagram
{
	protected :
		unsigned int _blength;
		unsigned char * _data;

	public :
		RawDatagram();
		RawDatagram(const RawDatagram & dgm);
		const RawDatagram & operator=(const RawDatagram & dgm);
		~RawDatagram();

		void reset();
		void resize(unsigned int len);
		bool read(FILE * fp);
		bool read(const std::vector<unsigned char> & s);
		bool read(const unsigned char * s, unsigned int len);
		bool write(FILE * fp) const;
		bool receive(Socket * sock,unsigned int timeout=1000, 
				bool verbose=false);
		bool send(Socket * sock);

		unsigned int blength() {return _blength;}
		const unsigned char * data() const {return _data;}
		unsigned char * data() {return _data;}
};
		
#if 1	

class Datagram : public RawDatagram
{
	public :
		void setId(unsigned char i);
		unsigned char getId() const;
		unsigned short getStatus() const;
		void setStatus(unsigned char s1,unsigned char s2);
		void setStatus(unsigned short s1);

		void setData(unsigned char c, unsigned int pos);
		void setData(unsigned char * bd, unsigned int nb);
		
		unsigned int getBLength() const;
		const unsigned char * bdata() const ;

		void print(FILE * fp = stdout) const;
	
};

#endif

#endif // DATA_GRAM_H
