/**Signature>
* Author      : Cedric Pradalier 
* Universite  : INRIA - GRAVIR - INPG
* Email       : cedric.pradalier@inrialpes.fr
* Contexte    : These MESR 
* Date        : 2001 - 2004
* License     : Libre (???)
<Signature**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "canon_vbc50i/libSock/SocketCom.h"

//#define TRACE

SocketCom::SocketCom(const char * _host, int _port) :
	Socket()
{
	server = false;
	host = strdup(_host);
	port = _port;
}
	
SocketCom::SocketCom(int _port) :
	Socket()
{
	server = true;
	host = NULL;
	port = _port;
}
	
SocketCom::~SocketCom()
{
	if (!server) free(host);
#ifdef TRACE
	printf("SocketCom : Destruction\n");
#endif
}

bool SocketCom::PrepareServer()
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
bool SocketCom::Open(Socket * newsocket)
{
	if ((!server)||(!accepting)) return false;
	if (!Accept(newsocket)) return false;
#ifdef TRACE
	printf("Accept OK\n");
#endif
	return true;
}

bool SocketCom::Open()
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

