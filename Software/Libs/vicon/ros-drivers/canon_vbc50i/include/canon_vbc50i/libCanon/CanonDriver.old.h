#ifndef CANON_DRIVER_H
#define CANON_DRIVER_H

#include <vector>
#include <Socket.h>
#include "Datagram.h"

class CanonDriver
{
	public :
		const static unsigned int CONTROL_PORT = 65311;
		const static unsigned int VIDEO_PORT = 65310;
	protected :
		Socket * ctrlsock;
		Socket * videosock;
		char * host;

		std::vector<Datagram> init_seq;
		std::vector<Datagram> init_reply;
		std::vector<Datagram> keepalive_seq;
		std::vector<Datagram> keepalive_reply;

		double cpan,ctilt,czoom;

	public :
		CanonDriver(const char * initseq_filename,
				const char * hostname);
		~CanonDriver();

		bool connect();
		bool disconnect();
		bool keepalive();
		bool requestCurrentPos();
		void getCurrentPos(double * pan, double * tilt, double * zoom);

		bool waitcompletion();
		bool moveto(double pan, double tilt, double zoom);
		bool panto(double pan);
		bool tiltto(double to);
		bool zoomto(double zoom);

		bool center();
		typedef enum {
			Right=1, 
			Left=2, 
			Up=4, 
			UpRight=5, 
			UpLeft=6, 
			Down=8,
			DownRight=9,
			DownLeft=10
		} Direction;
		bool movetoward(Direction dir);
		bool stop();

		bool getframe();
};

#endif // CANON_DRIVER_H
