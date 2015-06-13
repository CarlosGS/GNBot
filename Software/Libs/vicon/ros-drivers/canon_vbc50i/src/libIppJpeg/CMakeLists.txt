# add definitions - turn on debugging checks and output
ADD_DEFINITIONS( -ggdb -march=pentium4 -O3 -pipe -Wall)
#ADD_DEFINITIONS( -ggdb -Wall)

INCLUDE_DIRECTORIES( /data/aa/pub/intel/ipp41/ia32_itanium/include )
LINK_DIRECTORIES( /data/aa/pub/intel/ipp41/ia32_itanium/sharedlib )
ADD_LIBRARY(
	IppJpeg
	STATIC
	colorcomp.cpp
	dechtbl.cpp
	decoder.cpp
	decqtbl.cpp
	enchtbl.cpp
	encoder.cpp
	encqtbl.cpp
	jpegbase.cpp
)


TARGET_LINK_LIBRARIES(IppJpeg ippj ippi ipps ippcore )

#INSTALL_TARGETS(/lib/i486-linux IppJpeg)
	
	
