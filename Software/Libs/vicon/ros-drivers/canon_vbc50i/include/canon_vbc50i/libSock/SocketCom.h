/**Signature>
* Author      : Cedric Pradalier 
* Universite  : INRIA - GRAVIR - INPG
* Email       : cedric.pradalier@inrialpes.fr
* Contexte    : These MESR 
* Date        : 2001 - 2004
* License     : Libre (???)
<Signature**/
#ifndef SOCKET_COM_H
#define SOCKET_COM_H

#include "Socket.h"


class SocketCom : public Socket
{
	protected :
		int port;
		char * host;
		bool server;
	public :
		SocketCom(const char * _host, int _port);
		SocketCom(int port);
		~SocketCom();
		
		bool PrepareServer();
		bool Open(Socket * newsocket);
		bool Open();
};
	

#endif // SOCKET_COM_H
