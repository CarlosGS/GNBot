/**Signature>
* Author      : Cedric Pradalier 
* Universite  : INRIA - GRAVIR - INPG
* Email       : cedric.pradalier@inrialpes.fr
* Contexte    : These MESR 
* Date        : 2001 - 2004
* License     : Libre (???)
<Signature**/
// Implementation of the Socket class.

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

#include "canon_vbc50i/libSock/Socket.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifdef MACOSX
#define socklen_t int
#define MSG_NOSIGNAL 0
#endif


Socket::Socket() : m_sock ( -1 )
{

	outputErrors = true;
	broken_pipe = false;
	accepting = false;
	memset ( &m_addr, 0, sizeof ( m_addr ) );

	server = false;
	host = NULL;
	port = 0;

}


Socket::Socket(const char * _host, int _port) : m_sock(-1)
{
	outputErrors = true;
	broken_pipe = false;
	accepting = false;
	memset ( &m_addr, 0, sizeof ( m_addr ) );

	server = false;
	host = strdup(_host);
	port = _port;
}
	
Socket::Socket(int _port) : m_sock(-1)
{
	outputErrors = true;
	broken_pipe = false;
	accepting = false;
	memset ( &m_addr, 0, sizeof ( m_addr ) );

	server = true;
	host = NULL;
	port = _port;
}
	

Socket::~Socket()
{
	if ( IsOpen() ) 
	{
		Close();
	}

	if (host != NULL) {
		free(host);
		host = NULL;
	}
}

bool Socket::Close()
{
	shutdown(m_sock,2);
	close(m_sock);
	broken_pipe = false;
	return true;
}

bool Socket::Create()
{
	broken_pipe = false;
	m_sock = socket ( AF_INET, SOCK_STREAM, 0 );

	if ( ! IsOpen() )
		return false;


	// TIME_WAIT - argh
	int on = 1;
	if ( setsockopt ( m_sock, SOL_SOCKET, 
				SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
	{
		if (outputErrors) perror("Create");
		return false;
	}
	if ( setsockopt ( m_sock, SOL_SOCKET, 
				SO_KEEPALIVE, ( const char* ) &on, sizeof ( on ) ) == -1 )
	{
		if (outputErrors) perror("Create");
		return false;
	}

	return true;

}



bool Socket::Bind ( const int port )
{
	broken_pipe = false;

	if ( ! IsOpen() )
	{
		return false;
	}



	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = INADDR_ANY;
	m_addr.sin_port = htons ( port );

	int bind_return = bind ( m_sock,
			( struct sockaddr * ) &m_addr,
			sizeof ( m_addr ) );
	if (bind_return == -1)
	{
		if (outputErrors) perror("Bind");
		return false;
	}
	return true;
}


bool Socket::Listen() 
{
	broken_pipe = false;
	accepting = false;
	if ( ! IsOpen() )
	{
		return false;
	}

	int listen_return = listen ( m_sock, MAXCONNECTIONS );


	accepting = ( listen_return != -1 );
	return accepting;
}


bool Socket::Accept ( Socket* new_socket ) const
{
	int addr_length = sizeof ( m_addr );
	new_socket->m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

	return ( new_socket->m_sock > 0 );
}


size_t Socket::Send ( const unsigned char * s, size_t size) 
{
	int status = send ( m_sock, s, size, MSG_NOSIGNAL );
	if ( status < 0 )
	{
		switch (errno) {
			case ECONNRESET:
			case ENOTCONN:
			case EPIPE:
				broken_pipe = true;
				break;
			default :
				break;
		}
		if (outputErrors) {
			char tmp[128];
			sprintf(tmp,"Send %s:%d",host,port);
			perror(tmp);
		}
		return 0;
	}
	else
	{
		return (size_t)status;
	}
}


size_t Socket::Receive ( unsigned char * s, size_t max_size ) 
{
	int status = recv ( m_sock, s, max_size , 0 );

	if ( status < 0 )
	{
		//printf("status == -1   errno == %d in Socket::recv\n",errno);
		switch (errno) {
			case ECONNRESET:
			case ENOTCONN:
			case EPIPE:
				broken_pipe = true;
				break;
			default :
				break;
		}
		if (outputErrors)  {
			char tmp[128];
			sprintf(tmp,"Receive %s:%d",host,port);
			perror(tmp);
		}
		return 0;
	}
	else 
	{
		return (size_t)status;
	}
}


bool Socket::Connect ( const char * host, const int port )
{
	broken_pipe = false;
	if ( ! IsOpen() ) return false;

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons ( port );

	int status = inet_pton ( AF_INET, host, &m_addr.sin_addr );
	if ( errno == EAFNOSUPPORT ) return false;
	if (status <= 0)
	{
		struct hostent *_host_;
		_host_ =  gethostbyname(host);
		if (_host_ == NULL) return false;
		memcpy(&m_addr.sin_addr.s_addr, _host_->h_addr, _host_->h_length);
	}


	status = connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );
	if (status < 0)
	{
		if (outputErrors) perror("Connect");
		return false;
	}

	int opts = fcntl ( m_sock, F_GETFL );
	if (opts >= 0)
		fcntl(m_sock,F_SETFL,opts | O_SYNC);
	return ( status == 0 );
}

void Socket::SetNonBlocking ( const bool b )
{

	int opts;

	opts = fcntl ( m_sock, F_GETFL );

	if ( opts < 0 )
	{
		return;
	}

	if ( b )
		opts = ( opts | O_NONBLOCK );
	else
		opts = ( opts & ~O_NONBLOCK );

	fcntl ( m_sock, F_SETFL,opts );

}


bool Socket::WaitData(size_t millisec)
{
	struct timeval to = {0,0};
	to.tv_sec = (millisec/1000);
	to.tv_usec = (millisec%1000) * 1000;
	FD_ZERO(&rfs);FD_SET(m_sock,&rfs);
	int r = select(m_sock+1,&rfs,NULL,NULL,&to);
	if (r < 0) {
		switch (errno) {
			case EBADF:
				broken_pipe = true;
				break;
			default :
				break;
		}
		if (outputErrors) {
			char tmp[128];
			sprintf(tmp,"Select %s:%d",host,port);
			perror(tmp);
		}
	}
	return (r >= 1);
}
		
bool Socket::WaitBuffer(unsigned char * s, size_t size,
		size_t millisec) 
{
	unsigned int readbytes;
	double t1ms,t2ms,dtms,maxms;
	struct timeval tv1,tv2;
	struct timeval to = {0,0};
	maxms = millisec*1e-3;
	to.tv_sec = (millisec/1000);
	to.tv_usec = (millisec%1000) * 1000;
	gettimeofday(&tv1,NULL);
	readbytes = 0;
	t1ms = tv1.tv_sec+tv1.tv_usec*1e-6;
	while ((readbytes < size) && !broken_pipe)
	{
		FD_ZERO(&rfs);FD_SET(m_sock,&rfs);
		int r = select(m_sock+1,&rfs,NULL,NULL,&to);
		if (r < 1) {
			switch (errno) {
				case EBADF:
					broken_pipe = true;
					break;
				default :
					break;
			}
			if (outputErrors) {
				char tmp[128];
				sprintf(tmp,"Select %s:%d",host,port);
				perror(tmp);
			}
			return false;
		}
		//r = recv ( m_sock, s+readbytes, size-readbytes , 0 );
		r = Receive ( s+readbytes, size-readbytes );
		if (r>0) readbytes += r;
		//printf("Rec %d\n",readbytes);

		gettimeofday(&tv2,NULL);
		t2ms = tv2.tv_sec+tv2.tv_usec*1e-6;
		dtms = t2ms - t1ms;
		if (dtms >= maxms) break;
		dtms = maxms - dtms;
		to.tv_sec = (unsigned int)(dtms);
		to.tv_usec = (unsigned int)((dtms - to.tv_sec)*1e6);
	}
	return (readbytes == size);
}

bool Socket::SendAll(const unsigned char * s, size_t size,
		size_t millisec) 
{
	unsigned int sentbytes;
	double t1ms,t2ms,dtms,maxms;
	struct timeval tv1,tv2;
	gettimeofday(&tv1,NULL);
	maxms = millisec * 1e-3;
	t1ms = tv1.tv_sec+(tv1.tv_usec*1e-6);
	sentbytes = 0;
	while ((sentbytes < size) && !broken_pipe)
	{
		int r = Send (s+sentbytes, size-sentbytes);
		if (r > 0) sentbytes += r;
		//printf("Sent %d ",sentbytes);
		gettimeofday(&tv2,NULL);
		t2ms = tv2.tv_sec+(tv2.tv_usec*1e-6);
		dtms = t2ms - t1ms;
		//printf("%f %f dtms : %f/%f\n",t1ms,t2ms,dtms,maxms);
		
		if (dtms >= maxms) break;
	}
	//printf(" Finally sent %d bytes\n",sentbytes);
	return (sentbytes == size);
}


bool Socket::PrepareServer()
{
	if (!server) return false;
	if (accepting) return true;
#ifdef TRACE
	printf("Opening server\n");
#endif
	if (!Create()) return false;
#ifdef TRACE
	printf("Create OK\n");
#endif
	if (!Bind(port)) return false;
#ifdef TRACE
	printf("Binding OK\n");
#endif
	if (!Listen()) return false;
#ifdef TRACE
	printf("Listening OK\n");
#endif
	return true;
}
bool Socket::Open(Socket * newsocket)
{
	if ((!server)||(!accepting)) return false;
	if (!Accept(newsocket)) return false;
#ifdef TRACE
	printf("Accept OK\n");
#endif
	return true;
}

//#define TRACE

bool Socket::Open()
{
	if (server)
	{
#ifdef TRACE
		printf("Opening server\n");
#endif
		Socket listener;
		if (!listener.Create()) return false;
#ifdef TRACE
		printf("Create OK\n");
#endif
		if (!listener.Bind(port)) return false;
#ifdef TRACE
		printf("Binding OK\n");
#endif
		if (!listener.Listen()) return false;
#ifdef TRACE
		printf("Listening OK\n");
#endif
		if (!listener.Accept(*this)) return false;
#ifdef TRACE
		printf("Accept OK\n");
#endif
	}
	else
	{
#ifdef TRACE
		printf("Opening client %s:%d\n",host,port);
#endif
		if (!Create()) return false;
#ifdef TRACE
	printf("Create OK\n");
#endif
		if (!Connect(host,port)) return false;
#ifdef TRACE
		printf("Connect OK\n");
#endif
		SetNonBlocking(true);
	}
	return true;
}


