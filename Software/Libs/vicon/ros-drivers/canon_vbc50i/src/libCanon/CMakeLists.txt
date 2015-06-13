
INCLUDE_DIRECTORIES(..)

ADD_LIBRARY(
	Canon
	STATIC
	JpegReader.cpp
	${IPP_SRC}
	DgmQueue.cpp
	CommManager.cpp
	Videogram.cpp
	VideoManager.cpp
	JpegStreamSaver.cpp
	Datagram.cpp
	CanonDriver.cpp
	login.cpp
)


