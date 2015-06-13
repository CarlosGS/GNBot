/**Signature>
* Author      : Cedric Pradalier 
* Universite  : INRIA - GRAVIR - INPG
* Email       : cedric.pradalier@inrialpes.fr
* Contexte    : These MESR 
* Date        : 2001 - 2004
* License     : Libre (???)
<Signature**/
// Definition of the Socket class

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>


const int MAXHOSTNAME = 1024;
const int MAXCONNECTIONS = 5;

class Socket
{
	public :
		void showErrors() {outputErrors = true;}
		void hideErrors() {outputErrors = false;}
	protected :
		bool outputErrors;
	public :
		// Server initialization
		bool Create();
		bool Bind ( const int port );
		bool Listen() ;
		bool Accept ( Socket & s ) const {return Accept(&s);}
		bool Accept ( Socket * s ) const;

		// Client initialization
		bool Connect ( const char * host, const int port );
		void SetNonBlocking ( const bool );


	public:
		Socket();
		Socket(const char * _host, int _port);
		Socket(int port);
		~Socket();

		// Data Transimission
		size_t Send ( const unsigned char * s, size_t size ) ;
		size_t Receive (  unsigned char * s, size_t max_size ) ;
		bool WaitData(size_t millisec);

		bool WaitBuffer(unsigned char * s, size_t size,
				size_t millisec) ;

		bool SendAll(const unsigned char * s, size_t size,
				size_t millisec) ;

		bool Close();


		bool IsOpen() const { return m_sock != -1; }
		bool IsBroken() const {return broken_pipe;}

		bool PrepareServer();
		bool Open(Socket * newsocket);
		bool Open();
	protected: 
		bool accepting;
		bool broken_pipe;

		int port;
		char * host;
		bool server;
	private:

		fd_set rfs;
		int m_sock;
		sockaddr_in m_addr;

};


#endif
