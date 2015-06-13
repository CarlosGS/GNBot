#ifndef CANON_LOGIN_H
#define CANON_LOGIN_H

struct RawBinDatagram {
	unsigned int len;
	unsigned char data[32];
};

extern RawBinDatagram * raw_bin_login;

#endif
